/*****************************************************************************

  Keypad values test

  BattMon 2.0 project repository:
  https://github.com/lpollier/battmon
  MIT licence
  Copyright (c) 2020-2022 Loïc POLLIER

*****************************************************************************/

#include <LiquidCrystal.h>
// Define LCD pin setting
#define LCD_RS      8
#define LCD_EN      9
#define LCD_D4      4
#define LCD_D5      5
#define LCD_D6      6
#define LCD_D7      7

// Set the pins used on the LCD panel
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("2x16 LCD keypad ");
  lcd.setCursor(0, 1);
  lcd.print("Press Key:");
}

void loop() {
  int x;
  x = analogRead(0);
  lcd.setCursor(11, 1);
  lcd.print(x);
  lcd.print("     ");
  delay(200);
}
