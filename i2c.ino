/*****************************************************************************

  I2C functions

  BattMon 2.0 project repository:
  https://github.com/lpollier/battmon
  MIT licence
  Copyright (c) 2020-2022 Loïc POLLIER

*****************************************************************************/

uint8_t i2c_detect_device(uint8_t addr) {
  addr <<= 1; // Shift to make room for read/write bit
  if (i2c_start(addr + I2C_WRITE)) {
    // Device inaccessible
    i2c_stop();
    delay(100);
    return false;
  }
  else {
    // Device accessible
    i2c_write(0x00);
    i2c_rep_start(addr + I2C_READ);
    i2c_readAck();
    i2c_readNak();
    i2c_stop();
    delay(100);
    return true;
  }
}

void i2c_smbus_write_word(uint8_t command, unsigned int data) {
  i2c_start((deviceAddress << 1) + I2C_WRITE);
  i2c_write(command);
  i2c_write((uint8_t)data);
  i2c_write((uint8_t)(data >> 8));
  i2c_stop();
  return;
}

unsigned int i2c_smbus_read_word(uint8_t command) {
  unsigned int buffer = 0;
  i2c_start((deviceAddress << 1) + I2C_WRITE);
  i2c_write(command);
  i2c_rep_start((deviceAddress << 1) + I2C_READ);
  buffer = i2c_readAck();
  buffer += i2c_readNak() << 8;
  i2c_stop();
  return buffer;
}

uint8_t i2c_smbus_read_block(uint8_t command, char* blockBuffer, uint8_t blockBufferLen) {
  uint8_t x, num_bytes;
  i2c_start((deviceAddress << 1) + I2C_WRITE);
  i2c_write(command);
  i2c_rep_start((deviceAddress << 1) + I2C_READ);
  num_bytes = i2c_readAck(); // Num of bytes; 1 byte will be index 0
  num_bytes = constrain(num_bytes, 0, blockBufferLen - 2); // Room for null at the end
  for (x=0; x<num_bytes-1; x++) { // -1 because x=num_bytes-1 if x<y; last byte needs to be "nack"'d, x<y-1
    blockBuffer[x] = i2c_readAck();
  }
  blockBuffer[x++] = i2c_readNak(); // This will nack the last byte and store it in x's num_bytes-1 address
  blockBuffer[x] = 0; // And null it at last_byte+1 for LCD printing
  i2c_stop();
  return num_bytes;
}
