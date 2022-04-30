/*****************************************************************************

  Format functions

  BattMon 2.0 project repository:
  https://github.com/lpollier/battmon
  MIT licence
  Copyright (c) 2020-2022 Loïc POLLIER

*****************************************************************************/

void fmtBinary(uint16_t val, byte digitLen, char *buf, unsigned bufLen=0xFF);
unsigned fmtUnsigned(unsigned long val, char *buf, unsigned bufLen=0xFF, byte digitLen=0);
void fmtDouble(double val, byte precision, char *buf, unsigned bufLen=0xFF);

// Format a binary value with number of the digits length.
// The 'digitLen' parameter is a number from 0 to 16 indicating the desired digits length.
// The 'buf' parameter points to a buffer to receive the formatted string. This must be
// sufficiently large to contain the resulting string. The buffer's length may be
// optionally specified. If it is given, the maximum length of the generated string
// will be one less than the specified value.
//
// Example: fmtBinary(11, 8, buf); // Produces 00001011 (8 digits length)
//
void fmtBinary(uint16_t val, byte digitLen, char *buf, unsigned bufLen) {
  if (!buf || !bufLen) return;
  // Limit the digits length to the LCD width
  const byte lcdWidth = 16;
  if (digitLen > lcdWidth)
    digitLen = lcdWidth;

  if (--bufLen > 0)
  {
    // Reset the digit buffer
    memset(buf, 0, lcdWidth);

    // Produce the digit string (backwards in the digit buffer)
    uint16_t mask = 1;
    for (byte i=0; i<digitLen; i++) {
      if ((val & mask) >= 1)
        buf[digitLen-1-i] = '1';
      else
        buf[digitLen-1-i] = '0';
      mask <<= 1;
    }
  }

  // Add the null termination
  *buf += '\0';
}

// Produce a formatted string in a buffer corresponding to the value provided.
// If the 'digitLen' parameter is non-zero, the value will be padded with leading
// zeroes to achieve the specified length. The number of characters added to
// the buffer (not including the null termination) is returned.
//
unsigned fmtUnsigned(unsigned long val, char *buf, unsigned bufLen, byte digitLen) {
  if (!buf || !bufLen) return(-1);

  // Produce the digit string (backwards in the digit buffer)
  char dbuf[10];
  unsigned idx = 0;
  while (idx < sizeof(dbuf)) {
    dbuf[idx++] = (val % 10) + '0';
    if ((val /= 10) == 0) break;
  }

  // Copy the optional leading zeroes and digits to the digit buffer
  unsigned len = 0;
  byte padding = (digitLen > idx) ? digitLen - idx : 0;
  char c = '0';
  while ((--bufLen > 0) && (idx || padding)) {
    if (padding) padding--;
    else c = dbuf[--idx];
    *buf++ = c;
    len++;
  }

  // Add the null termination
  *buf = '\0';
  return(len);
}

// Format a floating point value with number of the decimal places.
// The 'precision' parameter is a number from 0 to 6 indicating the desired decimal places.
// The 'buf' parameter points to a buffer to receive the formatted string. This must be
// sufficiently large to contain the resulting string. The buffer's length may be
// optionally specified. If it is given, the maximum length of the generated string
// will be one less than the specified value.
//
// Example: fmtDouble(3.1415, 2, buf); // Produces 3.14 (two decimal places)
//
void fmtDouble(double val, byte precision, char *buf, unsigned bufLen) {
  if (!buf || !bufLen) return;
  // Limit the precision to the maximum allowed value
  const byte maxPrecision = 6;
  if (precision > maxPrecision)
    precision = maxPrecision;

  if (--bufLen > 0)
  {
    // Check for a negative value
    if (val < 0.0) {
      val = -val;
      *buf = '-';
      bufLen--;
    }

    // Compute the rounding factor and fractional multiplier
    double roundingFactor = 0.5;
    unsigned long mult = 1;
    for (byte i=0; i<precision; i++) {
      roundingFactor /= 10.0;
      mult *= 10;
    }
    if (bufLen > 0) {
      // Apply the rounding factor
      val += roundingFactor;

      // Add the integral portion to the digit buffer
      unsigned len = fmtUnsigned((unsigned long)val, buf, bufLen);
      buf += len;
      bufLen -= len;
    }

    // Handle the fractional portion
    if ((precision > 0) && (bufLen > 0)) {
      *buf++ = '.';
      if (--bufLen > 0) buf += fmtUnsigned((unsigned long)((val - (unsigned long)val) * mult), buf, bufLen, precision);
    }
  }

  // Add the null termination
  *buf += '\0';
}
