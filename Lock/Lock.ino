#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include <EEPROM.h>

const bool EnablePasscodeChange = true;
const bool EnableServo = false;

const unsigned long MaxNoActivityMin = 2;
const unsigned long MaxNoActivityMillis = MaxNoActivityMin * 60000;
unsigned char PasscodeLength = 4;
unsigned char Eeprom_PasscodeLength = 0;
const unsigned char MaxPasscodeLength = 19;
char Data[MaxPasscodeLength];
char Master[MaxPasscodeLength] = "1234";

unsigned char data_count = 0;
char Key;
bool LockState = false;

unsigned long TimeLastActivity = 0;
unsigned long TimeNoActivity = 0;
bool LCDBacklightState = true;

char code_buff1[MaxPasscodeLength];
unsigned char dataCountBuff1 = 0;
char code_buff2[MaxPasscodeLength];
unsigned char dataCountBuff2 = 0;
unsigned char NewPasscodeLength = 0;

const unsigned char ROWS = 4;
const unsigned char COLS = 3;

char KeyMap[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {23, 22, 21, 20};
byte colPins[COLS] = {19, 18, 8};

// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

Keypad customKeypad = Keypad(makeKeymap(KeyMap), rowPins, colPins, ROWS, COLS);

Servo Lock;

void InitializationLCD() {
  lcd.begin(20,4);// initialize library
  BackLightTest();
  lcd.setCursor(0,0);           // set cursor to positon x=0, y=0
  lcd.print("Brainbox");        // print text on the LCD
  delay(500);
  lcd.setCursor(2,1);
  lcd.print("Arduino");
  delay(500);
  lcd.setCursor(4,2);
  lcd.print("Lock");
  delay(1000);
  lcd.clear();
}

void BackLightTest() {
  lcd.backlight();          //backlight on
  delay(250);
  lcd.noBacklight();        //backlight off
  delay(250);
  lcd.backlight();          //backlight on
  }

void ActivityCheck() {
  if (millis() > TimeLastActivity) {
    TimeNoActivity = millis() - TimeLastActivity;
  }

  else {
    TimeNoActivity = millis() + TimeLastActivity;
  }

  if ((TimeNoActivity > MaxNoActivityMillis) && LCDBacklightState)  {
    LCDBacklightState = false;
    Serial.print("LCD BackLight: ");
    Serial.println(LCDBacklightState);
  }
}

void ActivityReset() {
  TimeLastActivity = millis();
  LCDBacklightState = true;
  Serial.print("LCD BackLight: ");
  Serial.println(LCDBacklightState);
}

void Backlight(bool state) {
  if (state) {
    lcd.backlight();
  }
  else {
    lcd.noBacklight();
  }
}

void clearData(){
  while(data_count > 0){
    Data[data_count--] = 0; 
  }
  return;
}

char WaitForKey() {
  char key;
  key = customKeypad.getKey();
  while (key == NO_KEY) {
     key = customKeypad.getKey();
     ActivityCheck();
     Backlight(LCDBacklightState);
    }
  ActivityReset();
  LCDBacklightState = true;
  Backlight(LCDBacklightState);
  return key;
}

void GetCode() {
   if (data_count < MaxPasscodeLength) {
    Data[data_count] = Key; 
    lcd.setCursor(data_count,1); 
    lcd.print("*");
    Serial.println();
    Serial.print("Data_count: ");
    Serial.println(data_count);
    Serial.print("Data: ");
    Serial.println(Data[data_count]);
    data_count++;
   }
   else {
      lcd.setCursor(data_count,1);
      lcd.print("*");
      lcd.setCursor(0, 3);
      lcd.print("Passcode Overflow");
      Serial.println();
      Serial.println("Passcode Overflow Error");
      _delay_ms(1000);
      lcd.clear();
      clearData();
   }
}

void CodeCorrect() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Correct");
  LockState = true;
  _delay_ms(1000);
  Serial.println();
  Serial.println("Opened");
}

void CodeIncorrect() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Incorrect");
  Serial.println();
  Serial.println("Incorrect PSWD");
  LockState = false;
  _delay_ms(1000);
}

void CheckCode() {
  if(data_count == PasscodeLength){
      Data[PasscodeLength] = 0;
      _delay_ms(500);
      lcd.clear();
      Serial.println();
      Serial.print("Data:   ");
      Serial.println(Data);
      Serial.print("Master: ");
      Serial.println(Master);

      if(!strcmp(Data, Master)){
          CodeCorrect();
        }

      else{
          Serial.println();
          Serial.print("PasscodeLength: ");
          Serial.println(PasscodeLength);
          Serial.print("DataCount: ");
          Serial.println(data_count);
          CodeIncorrect();
        }
  }
  else {
      Serial.println();
      Serial.print("Data:   ");
      Serial.println(Data);
      Serial.print("Master: ");
      Serial.println(Master);
      Serial.println();
      Serial.print("PasscodeLength: ");
      Serial.println(PasscodeLength);
      Serial.print("DataCount: ");
      Serial.println(data_count);
      CodeIncorrect();
    }
  lcd.clear();
  clearData();
}

void OpenLock() {
  if (EnableServo) {
    Lock.write(90);
  }
  lcd.setCursor(0,0);
  lcd.print("Lock Opened");
  lcd.setCursor(0,2);
  lcd.print("Press -* to lock");
  if (EnablePasscodeChange) {
    lcd.setCursor(3,3);
    lcd.print("-# to change code");
  }
  TimeLastActivity = millis();
}

void CloseLock() {
  if (EnableServo) {
    Lock.write(180);
  }
  LockState = false;
  lcd.clear();
  Serial.println();
  Serial.println("Closing");
  TimeLastActivity = millis();
}

void GetEepromCode() {
   EEPROM.get(0, Eeprom_PasscodeLength);
   if (Eeprom_PasscodeLength) {
      NewPasscodeLength = Eeprom_PasscodeLength;
      for (unsigned char i = 0; i < PasscodeLength; i++) {
        EEPROM.get(i + 1, Master[i]);
      }
   }
   else {
    NewPasscodeLength = PasscodeLength;
    return;
   }
   PasscodeLength = NewPasscodeLength;
}

void WriteEepromCode(){
  EEPROM.put(0, PasscodeLength);
  for (unsigned char i = 0; i < PasscodeLength; i++) {
    EEPROM.put(i + 1, Master[i]);
  }
}

void CheckNewCode() {
  Serial.print("dataCountBuff1: ");
  Serial.println(dataCountBuff1);
  Serial.print("dataCountBuff2: ");
  Serial.println(dataCountBuff2);
  if (dataCountBuff1 == dataCountBuff2) {
      if(!strcmp(code_buff1, code_buff2)){
          NewPasscodeLength = dataCountBuff2;
          for (unsigned char i = 0; i <= MaxPasscodeLength; i++) {
            Master[i] = 0;
          }
          
          for (unsigned char i = 0; i <= PasscodeLength; i++) {
            Master[i] = code_buff1[i];
          }
          Serial.println();
          Serial.print("Master: ");
          Serial.println(Master);
          lcd.clear();
          lcd.print("Passcodes Match");
          _delay_ms(1000);
          lcd.clear();
        }
     else {
        NewPasscodeLength = PasscodeLength;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Codes do not match");
        Serial.println();
        Serial.println("Passcodes do not match");
        _delay_ms(1000);
        lcd.clear();
     }
  }
  else {
    NewPasscodeLength = PasscodeLength;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Codes do not match");
    Serial.println();
    Serial.println("Passcodes are not the same length");
    _delay_ms(1000);
    lcd.clear();
  }
  clearData();
}

void ChangeCode() {
    dataCountBuff1 = 0;
    dataCountBuff2 = 0;
    for (unsigned char i = 0; i <= MaxPasscodeLength; i++) {
      code_buff1[i] = 0;
      code_buff2[i] = 0;
    }
    Serial.println();
    Serial.println("Changing Master");
    lcd.clear();
    lcd.print("Enter new code:");
    lcd.setCursor(0,2);
    lcd.print("press * to confirm");
    Key = WaitForKey();
    while(Key != '*') {
      GetCode();
      Key = WaitForKey();
    }
    Data[data_count] = 0;
    dataCountBuff1 = data_count;
    for (unsigned char i =0; i <= data_count; i++) {
      code_buff1[i] = Data[i];
    }
    clearData();
    lcd.clear();
    _delay_ms(500);
    lcd.print("Confirm new code:");
    lcd.setCursor(0,2);
    lcd.print("press * to confirm");
    Key = WaitForKey();
    while (Key != '*') {
      GetCode();
      Key = WaitForKey();
    }
    Data[data_count] = 0;
    dataCountBuff2 = data_count;
    for (unsigned char i =0; i <= data_count; i++) {
      code_buff2[i] = Data[i];
    }
    clearData();
    CheckNewCode();
    PasscodeLength = NewPasscodeLength;
    Serial.print("PasscodeLength: ");
    Serial.println(PasscodeLength);
    WriteEepromCode();
}

void setup() {
  // put your setup code here, to run once:
  GetEepromCode();
  if (EnableServo) {
    Lock.attach(11);
    Lock.write(180);
  }
  InitializationLCD();
  Serial.begin(9600);
  TimeLastActivity = millis();
  Serial.print("LCD Backlight: ");
  Serial.println(LCDBacklightState);
}

void loop() {
  // put your main code here, to run repeatedly:
  Key = customKeypad.getKey();
  if (LockState && Key == '*') {
    CloseLock();
  }

  else if (LockState && Key == '#' && EnablePasscodeChange) {
    ChangeCode();
  }
  
  else if (LockState) {
    OpenLock();
  }

  else {
    lcd.setCursor(0,0);
    lcd.print("Enter Passcode:");
    lcd.setCursor(0,2);
    lcd.print("Press * to validate");
    if (Key = WaitForKey(); Key == '*') {
      CheckCode();
    }
    else {
      GetCode();
    }
  }
}
