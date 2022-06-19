//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C *lcd;

void setup()
{

  TwoWire wire(0);
  wire.setPins( 23, 19 );

  lcd = new LiquidCrystal_I2C(0x27,20,2,&wire);  // set the LCD address to 0x27 for a 16 chars and 2 line display

  lcd->init();                      // initialize the lcd 
  lcd->init();
  // Print a message to the LCD.
  lcd->backlight();
  lcd->setCursor(0,0);
  lcd->print("Hello, world!");
  lcd->setCursor(0,1);
  lcd->print("NA5Y");
}


void loop()
{
}
