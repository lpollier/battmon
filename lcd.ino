/*****************************************************************************

  LCD functions

  BattMon 2.0 project repository:
  https://github.com/lpollier/battmon
  MIT licence
  Copyright (c) 2020-2022 Loïc POLLIER

*****************************************************************************/

void lcdReinitPhi() {
  init_phi_prompt(&lcd, keypads, function_keys, lcd_columns, lcd_rows, '~'); // Supply the liquid crystal object and the phi_buttons objects. Also supply the column and row of the lcd, and indicator as '>'. You can also use '\x7e', which is a right arrow.
}

void lcdCharShiftLeft(byte startIdx, byte endIdx) {
  byte x, y;
  for (x=startIdx; x<=endIdx; x++) {
    for (y=0; y<=7; y++) {
      if (x > startIdx) {
        if (bitRead(lcdCustomCharBuffer[x][y], 4)) bitSet(lcdCustomCharBuffer[x-1][y], 0);
        lcd.createChar(x - 1, lcdCustomCharBuffer[x-1]); // Now we can write it!
      }
      lcdCustomCharBuffer[x][y] <<= 1;
      lcdCustomCharBuffer[x][y] &= B11111;
    }
  }
  lcd.createChar(x, lcdCustomCharBuffer[x]); // Had to save this one for last
}

void lcdCharShiftRight(byte startIdx, byte endIdx) {
  byte x, y;
  for (x=startIdx; x<=endIdx; x++) {
    for (y=0; y<=7; y++) {
      if (x < endIdx) {
        if (bitRead(lcdCustomCharBuffer[x][y], 0)) bitSet(lcdCustomCharBuffer[x+1][y], 5);
        else bitClear(lcdCustomCharBuffer[x+1][y], 5);
      }
      lcdCustomCharBuffer[x][y] >>= 1;
    }
  lcd.createChar(x, lcdCustomCharBuffer[x]);
  }
}

void lcdClearSpace(byte col, byte row, byte spaces) {
  byte x = 0;
  lcd.setCursor(col, row);
  for (x; x<spaces; x++) lcd.write(' ');
  lcd.setCursor(col, row);
}

void lcdPadBinary(uint8_t value, uint8_t bits) {
  if (bits > 8) return; // lcd.write('E');
  byte x = 7;
  do {
    if (x > bits - 1) continue;
    lcd.write('0' + bitRead(value, x));
  } while (x--);
}
