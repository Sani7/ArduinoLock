#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

#define Password_Length 7 

char Data[Password_Length]; 
char Master[Password_Length] = "495876"; 
byte data_count = 0;
char Key;
bool LockState = false;

const byte ROWS = 4;
const byte COLS = 3;

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

void clearData(){
  while(data_count !=0){
    Data[data_count--] = 0; 
  }
  return;
}

void setup() {
  // put your setup code here, to run once:
  Lock.attach(11);
  Lock.write(180);
  InitializationLCD();
}

void loop() {
  Key = customKeypad.getKey();
  // put your main code here, to run repeatedly:
  if ((LockState) && (Key == '*')) {
    Lock.write(180);
    LockState = false;
    lcd.clear();
  }

  else if (LockState) {
    lcd.setCursor(0,0);
    lcd.print("Lock Opened");
    lcd.setCursor(0,2);
    lcd.print("Press * to lock");
    Lock.write(90);
    }
  
  else {
    lcd.setCursor(0,0);
    lcd.print("Enter Password:");

    if (Key){
      Data[data_count] = Key; 
      lcd.setCursor(data_count,1); 
      lcd.print("*"); 
      data_count++; 
      }

    if(data_count == Password_Length-1){
      delay(500);
      lcd.clear();

      if(!strcmp(Data, Master)){
        lcd.print("Correct");
        LockState = true;
        delay(1000);
        }
      else{
        lcd.print("Incorrect");
        LockState = false;
        delay(1000);
        }
    
      lcd.clear();
      clearData();  
    }
  }
}

