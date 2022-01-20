#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <stdio.h>

#define EnablePasscodeChange true
#define EnableServo false
#define EnableDebug true

#ifdef EnableDebug
#define printDebug(fp, fmt, ...) fprintf(fp, fmt, ##__VA_ARGS__)
#else
#define printDebug(fp, fmt, ...)
#endif
#define MaxNoActivitySec 15
#define MaxNoActivityMillis MaxNoActivitySec * 1000

#define MaxPasscodeLength 20
char Data[MaxPasscodeLength];
char Master[MaxPasscodeLength];

bool LockState = false;

unsigned long TimeLastActivity = 0;
unsigned long TimeNoActivity = 0;

#define ROWS 4
#define COLS 3

char KeyMap[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}};

unsigned char rowPins[ROWS] = {23, 22, 21, 20};
unsigned char colPins[COLS] = {19, 18, 8};

Keypad customKeypad = Keypad(makeKeymap(KeyMap), rowPins, colPins, ROWS, COLS);

// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

int uart_putchar(char c, FILE* stream);
int uart_getchar(FILE* stream);
void append(char *s, char c);
void BackLightTest();
void InitializationLCD();
void Backlight(bool state);
void ActivityCheck();
void ActivityReset();
void GetCode(char *code, char PressedKey);
void CodeCorrect();
void CodeIncorrect();
bool CheckCode(char *Data1, char *Data2);
void OpenLock();
void CloseLock();
void GetEepromCode();
void WriteEepromCode(char *newcode);
void ChangeCode();

static FILE USBserial;

void setup()
{
  // put your setup code here, to run once:
  USBserial.get = uart_getchar;
  USBserial.put = uart_putchar;
  USBserial.flags = _FDEV_SETUP_RW;

  GetEepromCode();
  InitializationLCD();
  TimeLastActivity = millis();
  if (EnableDebug)
  {
    Serial.begin(9600);
    printDebug(&USBserial, "LCD Backlight: 1");
  }
}

void loop()
{
  ActivityCheck();
  char Key = customKeypad.getKey();
  if (LockState && Key == '*')
  {
    ActivityReset();
    CloseLock();
    return;
  }

  else if (LockState && Key == '#' && EnablePasscodeChange)
  {
    ActivityReset();
    ChangeCode();
    return;
  }
  else if (LockState)
  {
    OpenLock();
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print("Enter Passcode:");
  lcd.setCursor(0, 2);
  lcd.print("Press * to validate");
  if (Key == '*')
  {
    if (CheckCode(Data, Master))
    {
      CodeCorrect();
      memset(Data, 0, sizeof(Data));
      return;
    }
    CodeIncorrect();
    memset(Data, 0, sizeof(Data));
    return;
  }
  else
  {
    GetCode(Data, Key);
  }
}

int uart_putchar(char c, FILE* stream)
{
	return Serial.write(c);
}

int uart_getchar(FILE* stream)
{
	return Serial.read();
}

void append(char *s, char c)
{
  int len = strlen(s);
  s[len] = c;
  s[len + 1] = '\0';
}

void BackLightTest()
{
  lcd.backlight(); //backlight on
  delay(250);
  lcd.noBacklight(); //backlight off
  delay(250);
  lcd.backlight(); //backlight on
}

void InitializationLCD()
{
  lcd.begin(20, 4); // initialize library
  BackLightTest();
  lcd.setCursor(0, 0);   // set cursor to positon x=0, y=0
  lcd.print("Brainbox"); // print text on the LCD
  delay(500);
  lcd.setCursor(2, 1);
  lcd.print("Arduino");
  delay(500);
  lcd.setCursor(4, 2);
  lcd.print("Lock");
  delay(500);
  lcd.setCursor(0, 3);
  lcd.print("by Sander Speetjens");
  delay(1000);
  lcd.clear();
}

void Backlight(bool state)
{
  if (state)
  {
    lcd.backlight();
    return;
  }
  lcd.noBacklight();
}

void ActivityCheck()
{
  if (millis() > TimeLastActivity)
  {
    TimeNoActivity = millis() - TimeLastActivity;
  }

  else
  {
    TimeNoActivity = millis() + TimeLastActivity;
  }

  if ((TimeNoActivity > MaxNoActivityMillis))
  {
    Backlight(false);
    printDebug(&USBserial, "LCD BackLight: 0");
  }
}

void ActivityReset()
{
  TimeLastActivity = millis();
  Backlight(true);
  printDebug(&USBserial, "LCD BackLight: 1");
}

void GetCode(char *code, char PressedKey)
{
  if (PressedKey == NO_KEY)
    return;
  ActivityReset();
  lcd.setCursor(strlen(code), 1);
  lcd.print("*");
  append(code, PressedKey);
  printDebug(&USBserial, "\nData: %c", code[strlen(code) - 1]);

  if (!(strlen(code) < MaxPasscodeLength))
  {
    lcd.setCursor(0, 3);
    lcd.print("Passcode Overflow");
    memset(code, 0, MaxPasscodeLength);
    printDebug(&USBserial, "\nPasscode Overflow Error\n");
    _delay_ms(1000);
    lcd.clear();
  }
}

void CodeCorrect()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Correct");
  LockState = true;
  _delay_ms(1000);
  printDebug(&USBserial, "\nOpened\n");
}

void CodeIncorrect()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Incorrect");
  printDebug(&USBserial, "Incorrect PSWD\n");
  LockState = false;
  _delay_ms(1000);
}
/*
 * This function checks if Data 1 char is the same as Data 2 char
*/
bool CheckCode(char *Data1, char *Data2)
{
  printDebug(&USBserial, "Data:   %s\nData len: %d\nMaster: %s\nMaster len: %d\n", Data1, strlen(Data1), Data2, strlen(Data2));

  if (strlen(Data1) == strlen(Data2))
  {
    if (!strcmp(Data1, Data2))
    {
      return 1;
    }
  }
  return 0;
}

void OpenLock()
{
  lcd.setCursor(0, 0);
  lcd.print("Lock Opened");
  lcd.setCursor(0, 2);
  lcd.print("Press * to lock");
  if (EnablePasscodeChange)
  {
    lcd.setCursor(3, 3);
    lcd.print("-# to change code");
  }
}

void CloseLock()
{
  LockState = false;
  lcd.clear();
  printDebug(&USBserial, "Closing\n");
}

void GetEepromCode()
{
  unsigned char Eeprom_PasscodeLength = 0;
  EEPROM.get(0, Eeprom_PasscodeLength);
  if (Eeprom_PasscodeLength)
  {
    for (unsigned char i = 0; i < Eeprom_PasscodeLength; i++)
    {
      EEPROM.get(i + 1, Master[i]);
    }
  }
}

void WriteEepromCode(char *newcode)
{
  EEPROM.put(0, strlen(newcode));
  for (unsigned char i = 0; i < strlen(newcode) + 1; i++)
  {
    EEPROM.put(i + 1, newcode[i]);
  }
}

void ChangeCode()
{
  char code_buff1[MaxPasscodeLength];
  char code_buff2[MaxPasscodeLength];
  char Key;
  memset(code_buff1, 0, sizeof(code_buff1));
  memset(code_buff2, 0, sizeof(code_buff2));
  printDebug(&USBserial, "Changing master\n");
  lcd.clear();
  lcd.print("Enter new code:");
  lcd.setCursor(0, 2);
  lcd.print("press * to confirm");
  Key = customKeypad.getKey();
  while (Key != '*')
  {
    GetCode(code_buff1, Key);
    Key = customKeypad.getKey();
  }

  lcd.clear();
  _delay_ms(500);
  lcd.print("Confirm new code:");
  lcd.setCursor(0, 2);
  lcd.print("press * to confirm");
  Key = customKeypad.getKey();
  while (Key != '*')
  {
    GetCode(code_buff2, Key);
    Key = customKeypad.getKey();
  }
  if (CheckCode(code_buff1, code_buff2))
  {
    strncpy(Master, code_buff1, sizeof(Master));
    WriteEepromCode(Master);
    printDebug(&USBserial, "New master: %s\n", Master);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Codes match");
  _delay_ms(1000);
  lcd.clear();
  return;
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Codes do not match");
  _delay_ms(1000);
  lcd.clear();
}