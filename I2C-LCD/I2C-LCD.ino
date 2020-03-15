/*
www.E2CRE8.be - Brainbox Arduino  - by Bart Huyskens
13/01/2016

This program drives an I2C LCD of the type: 
 16 character 2 line I2C Display
 Backpack Interface labelled "YwRobot Arduino LCM1602 IIC V1"  (2€ @ aliexpress)

Connect this LCD as follows:
LCD             Brainbox Arduino
GND             GND
VCC             +5V
SDA             SDA/2
SCL             SCL/3
!! Pull up resistors are required - place 4K7 between SDA and 5V and 4K7 between SCL and 5V

To communicate correctly with this I2C LCD you need to install the <LiquidCrystal_I2C.h> library in the arduino IDE
  1- download the "LiquidCrystal_I2C" library as a zip file from https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
  2- do not unzip the file
  3- in ARduino IDE: Sketch >> include library >> add .ZIP library - select the downloaded zip file
  4- this library is installed under 'mydocs'->Arduino : remove it by deleting it there
*/

#include <Wire.h>  // Comes with Arduino IDE

// Get the LCD I2C Library here: 
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
// Move any other LCD libraries to another folder or delete them
// See Library "Docs" folder for possible commands etc.
#include <LiquidCrystal_I2C.h>

const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
int sensorValue = 0;  
int outputValue = 0;  

// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address


void setup()
{
 
  
  lcd.begin(20,4);// initialize library
  lcd.backlight();          //backlight on
  delay(250);
  lcd.noBacklight();        //backlight off
  delay(250);
  
  
  lcd.backlight();              //backlight on
  lcd.setCursor(0,0);           // set cursor to positon x=0, y=0
  lcd.print("Brainbox");        // print text on the LCD
  delay(500);
  lcd.setCursor(2,1);
  lcd.print("Arduino");
  delay(1000);
}
 
 
void loop()
{
sensorValue = analogRead(analogInPin);              // read the analog value measured at analog pin AN0 (pin18) - We used a potmeter
outputValue = map(sensorValue, 0, 1023, 0, 100);    // rescale the sensorValue (0-1024) to (0-100)%

lcd.clear();                                        // clear the LCD
lcd.setCursor(0,0);                                 // set cursor to positon x,y
lcd.print("AN0 Value:");                            // print text on the LCD
lcd.print(sensorValue);                             // print the value of the variable on the LCD

lcd.setCursor(0,1);
lcd.print("AN0 :");
lcd.print(outputValue); 
lcd.print("%");                                     // add a % character

delay(200);                                         // delay of 100msec to avoid flikkering of the LCD
 
}
