/*****************************************************************************

  BattMon 2.0: Made with Arduino Uno + 2x16 LCD keypad shield

  I2C/SMBus interface to read Smart Battery Data with an Arduino Uno and a 2x16 LCD keypad shield
  Monitors data from bq2040 gas gauge equipped in Lithium-ion battery pack such as for the Sony Aibo ERS-2xx (ERA-201B1), ERS-3xx (ERA-301B1) and ERS-7 (ERA-7B1/ERA-7B2) robot series
  Last update: 2022-09-27 released version 2.0.10

  Arduino analog input 4 - I2C SDA
  Arduino analog input 5 - I2C SCL

  Arduino Uno R3 pinout:
  https://content.arduino.cc/assets/Pinout-UNOrev3_latest.png

  LCD KeyPad Shield specification:
  https://wiki.dfrobot.com/LCD_KeyPad_Shield_For_Arduino_SKU__DFR0009

  bq2040 gas gauge datasheet:
  https://www.ti.com/lit/ds/symlink/bq2040.pdf

  Depends on this internal library:
  https://www.arduino.cc/en/Reference/LiquidCrystal
  Depends on these external libraries:
  https://github.com/liudr/phi_prompt
  https://github.com/liudr/phi_interfaces
  https://liudr.wordpress.com/libraries/phi_buttons
  http://www.peterfleury.epizy.com/i2cmaster.zip

  Based on "SMBusBattery_Phi/BattMon 1.0" code initiated by FalconFour:
  https://forum.arduino.cc/t/smbus-laptop-battery-hacker-with-phi_prompt-lcd-user-interface/62728

  BattMon 2.0 project repository:
  https://github.com/lpollier/battmon
  MIT licence
  Copyright (c) 2020-2022 Loïc POLLIER

*****************************************************************************/

// Import all libraries
#include <phi_prompt.h>
#include <phi_interfaces.h>
#include <phi_buttons.h>
#include <LiquidCrystal.h>
#include <i2cmaster.h>

// Define LCD digit sizing
#define lcd_rows    2
#define lcd_columns 16

// Define LCD pin setting
#define LCD_RS      8
#define LCD_EN      9
#define LCD_D4      4
#define LCD_D5      5
#define LCD_D6      6
#define LCD_D7      7

byte deviceAddress = B0001011; // bq2040 i2c device address, most battery controllers seem to use this
byte cmdSet = 0; // Default command set (bq2040)

// Set the pins used on the LCD panel
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7); // Create the lcd object

// The following sets up function keys for phi_prompt library
char right_keys[] = {"R"}; // All keys that act as the right key are listed here
char up_keys[] = {"U"}; // All keys that act as the up key are listed here
char down_keys[] = {"D"}; // All keys that act as the down key are listed here
char left_keys[] = {"L"}; // All keys that act as the left key are listed here
char select_keys[] = {"S"}; // All keys that act as the enter key are listed here
char * function_keys[] = {up_keys, down_keys, left_keys, right_keys, select_keys}; // All function key names are gathered here for phi_prompt

// The following lines instantiates a button group to control 5 buttons
char mapping[] = {'R', 'U', 'D', 'L', 'S'}; // This is a list of names for each button
byte pins[] = {0}; // Corresponds to A0 analog to digital input
//int values[] = {0, 130, 300, 480, 720}; // Keypad values for "DFROBOT" LCD keypad shield
int values[] = {0, 100, 250, 400, 650}; // Keypad values for unbranded LCD keypad shield copy
phi_analog_keypads my_btns(mapping, pins, values, 1, 5); // buttons_per_row is 1, buttons_per_column is 5

// This serial keypad is for debugging
phi_serial_keypads debug_keypad(&Serial, 9600);

// The following adds all available keypads as inputs for phi_prompt library
multiple_button_input * keypads[] = {&my_btns, &debug_keypad, 0};

// Define global variables
#define bufferLen 32
char i2cBuffer[bufferLen];
char noDevicesFound[] = {"No devices found"};
uint8_t serialCommand;
unsigned int serialData;
int global_style = 125; // This is the style of the phi_prompt menus

// Define battery data type values
#define BATT_BITFIELD 0
#define BATT_DEC      1
#define BATT_HEX      2
#define BATT_MAH      3
#define BATT_MA       4
#define BATT_MV       5
#define BATT_MINUTES  6
#define BATT_PERCENT  7
#define BATT_TENTH_K  8
#define BATT_DATE     9
#define BATT_STRING   16

// Define battery EEPROM command values
#define Cmd_ManufacturerAccess     0x00
#define Cmd_RemainingCapacityAlarm 0x01
#define Cmd_RemainingTimeAlarm     0x02
#define Cmd_BatteryMode            0x03
#define Cmd_AtRate                 0x04
#define Cmd_AtRateTimeToFull       0x05
#define Cmd_AtRateTimeToEmpty      0x06
#define Cmd_AtRateOK               0x07
#define Cmd_Temperature            0x08
#define Cmd_Voltage                0x09
#define Cmd_Current                0x0A
#define Cmd_AverageCurrent         0x0B
#define Cmd_MaxError               0x0C
#define Cmd_RelativeStateOfCharge  0x0D
#define Cmd_AbsoluteStateOfCharge  0x0E
#define Cmd_RemainingCapacity      0x0F
#define Cmd_FullChargeCapacity     0x10
#define Cmd_RunTimeToEmpty         0x11
#define Cmd_AverageTimeToEmpty     0x12
#define Cmd_AverageTimeToFull      0x13
#define Cmd_ChargingCurrent        0x14
#define Cmd_ChargingVoltage        0x15
#define Cmd_BatteryStatus          0x16
#define Cmd_CycleCount             0x17
#define Cmd_DesignCapacity         0x18
#define Cmd_DesignVoltage          0x19
#define Cmd_SpecificationInfo      0x1A
#define Cmd_ManufactureDate        0x1B
#define Cmd_SerialNumber           0x1C
// Reserved                        Register function codes 0x1D - 0x1F
#define Cmd_ManufacturerName       0x1D // Register function code 0x20
#define Cmd_DeviceName             0x1E // Register function code 0x21
#define Cmd_DeviceChemistry        0x1F // Register function code 0x22
#define Cmd_ManufacturerData       0x20 // Register function code 0x23
#define Cmd_Flags                  0x21 // Register function code 0x2F
#define Cmd_EDV1                   0x22 // Register function code 0x3E
#define Cmd_EDVF                   0x23 // Register function code 0x3F

// Table of ALL global command labels used in the command sets
const char cmdLabel_ManufacturerAccess[] PROGMEM = "ManufacturerAccess";
const char cmdLabel_RemainingCapacityAlarm[] PROGMEM = "RemainingCapacityAlarm";
const char cmdLabel_RemainingTimeAlarm[] PROGMEM = "RemainingTimeAlarm";
const char cmdLabel_BatteryMode[] PROGMEM = "BatteryMode";
const char cmdLabel_AtRate[] PROGMEM = "AtRate";
const char cmdLabel_AtRateTimeToFull[] PROGMEM = "AtRateTimeToFull";
const char cmdLabel_AtRateTimeToEmpty[] PROGMEM = "AtRateTimeToEmpty";
const char cmdLabel_AtRateOK[] PROGMEM = "AtRateOK";
const char cmdLabel_Temperature[] PROGMEM = "Temperature";
const char cmdLabel_Voltage[] PROGMEM = "Voltage";
const char cmdLabel_Current[] PROGMEM = "Current";
const char cmdLabel_AverageCurrent[] PROGMEM = "AverageCurrent";
const char cmdLabel_MaxError[] PROGMEM = "MaxError";
const char cmdLabel_RelativeStateOfCharge[] PROGMEM = "RelativeStateOfCharge";
const char cmdLabel_AbsoluteStateOfCharge[] PROGMEM = "AbsoluteStateOfCharge";
const char cmdLabel_RemainingCapacity[] PROGMEM = "RemainingCapacity";
const char cmdLabel_FullChargeCapacity[] PROGMEM = "FullChargeCapacity";
const char cmdLabel_RunTimeToEmpty[] PROGMEM = "RunTimeToEmpty";
const char cmdLabel_AverageTimeToEmpty[] PROGMEM = "AverageTimeToEmpty";
const char cmdLabel_AverageTimeToFull[] PROGMEM = "AverageTimeToFull";
const char cmdLabel_ChargingCurrent[] PROGMEM = "ChargingCurrent";
const char cmdLabel_ChargingVoltage[] PROGMEM = "ChargingVoltage";
const char cmdLabel_BatteryStatus[] PROGMEM = "BatteryStatus";
const char cmdLabel_CycleCount[] PROGMEM = "CycleCount";
const char cmdLabel_DesignCapacity[] PROGMEM = "DesignCapacity";
const char cmdLabel_DesignVoltage[] PROGMEM = "DesignVoltage";
const char cmdLabel_SpecificationInfo[] PROGMEM = "SpecificationInfo";
const char cmdLabel_ManufactureDate[] PROGMEM = "ManufactureDate";
const char cmdLabel_SerialNumber[] PROGMEM = "SerialNumber";
const char cmdLabel_ManufacturerName[] PROGMEM = "ManufacturerName";
const char cmdLabel_DeviceName[] PROGMEM = "DeviceName";
const char cmdLabel_DeviceChemistry[] PROGMEM = "DeviceChemistry";
const char cmdLabel_ManufacturerData[] PROGMEM = "ManufacturerData";
const char cmdLabel_Flags[] PROGMEM = "Flags";
const char cmdLabel_EDV1[] PROGMEM = "EDV1";
const char cmdLabel_EDVF[] PROGMEM = "EDVF";

// This is set up in the same fashion as phi_prompt menus, because it's also used as one in the selector. String the names of your commands together IN ORDER with the commands definition below this
const char * const bq2040Labels[] PROGMEM = {cmdLabel_ManufacturerAccess, cmdLabel_RemainingCapacityAlarm, cmdLabel_RemainingTimeAlarm, cmdLabel_BatteryMode, cmdLabel_AtRate, cmdLabel_AtRateTimeToFull, cmdLabel_AtRateTimeToEmpty, cmdLabel_AtRateOK, cmdLabel_Temperature, cmdLabel_Voltage, cmdLabel_Current, cmdLabel_AverageCurrent, cmdLabel_MaxError, cmdLabel_RelativeStateOfCharge, cmdLabel_AbsoluteStateOfCharge, cmdLabel_RemainingCapacity, cmdLabel_FullChargeCapacity, cmdLabel_RunTimeToEmpty, cmdLabel_AverageTimeToEmpty, cmdLabel_AverageTimeToFull, cmdLabel_ChargingCurrent, cmdLabel_ChargingVoltage, cmdLabel_BatteryStatus, cmdLabel_CycleCount, cmdLabel_DesignCapacity, cmdLabel_DesignVoltage, cmdLabel_SpecificationInfo, cmdLabel_ManufactureDate, cmdLabel_SerialNumber, cmdLabel_ManufacturerName, cmdLabel_DeviceName, cmdLabel_DeviceChemistry, cmdLabel_ManufacturerData, cmdLabel_Flags, cmdLabel_EDV1, cmdLabel_EDVF};

// Here, a two-dimension array. First byte is the command code itself (hex). Second byte is the type of result it's expected to return (all except the strings come through as words)
const uint8_t bq2040Commands[][2] PROGMEM = {{0x00, BATT_HEX}, {0x01, BATT_MAH}, {0x02, BATT_MINUTES}, {0x03, BATT_BITFIELD}, {0x04, BATT_MA}, {0x05, BATT_MINUTES}, {0x06, BATT_MINUTES}, {0x07, BATT_HEX}, {0x08, BATT_TENTH_K}, {0x09, BATT_MV}, {0x0A, BATT_MA}, {0x0B, BATT_MA}, {0x0C, BATT_PERCENT}, {0x0D, BATT_PERCENT}, {0x0E, BATT_PERCENT}, {0x0F, BATT_MAH}, {0x10, BATT_MAH}, {0x11, BATT_MINUTES}, {0x12, BATT_MINUTES}, {0x13, BATT_MINUTES}, {0x14, BATT_MA}, {0x15, BATT_MV}, {0x16, BATT_BITFIELD}, {0x17, BATT_DEC}, {0x18, BATT_MAH}, {0x19, BATT_MV}, {0x1A, BATT_HEX}, {0x1B, BATT_DATE}, {0x1C, BATT_DEC}, {0x20, BATT_STRING}, {0x21, BATT_STRING}, {0x22, BATT_STRING}, {0x23, BATT_STRING}, {0x2F, BATT_BITFIELD}, {0x3E, BATT_MV}, {0x3F, BATT_MV}};

// This is what selects the command set itself and the associated device address. Just add a new entry (char cmdset_item01[] PROGMEM and uint8_t cmdset_addr01 PROGMEM), then add that entry to the cmdset_items[] and cmdset_addrs[] lists below
const char cmdset_item00[] PROGMEM = "bq2040";
const char * const cmdset_items[] PROGMEM = {cmdset_item00};
const uint8_t cmdset_addr00 PROGMEM = B0001011;
const uint8_t cmdset_addrs[] PROGMEM = {cmdset_addr00};

// Update each of these commands with your custom command set, just add a new "case" correlating to the cmdset_items[] position above
uint8_t cmd_getCode(uint8_t command) {
  switch (cmdSet) {
    case 0:
      // bq2040 commands
      return pgm_read_byte(&bq2040Commands[command][0]); // command is first parameter
      break;
  }
}
uint8_t cmd_getType(uint8_t command) {
  switch (cmdSet) {
    case 0:
      // bq2040 commands
      return pgm_read_byte(&bq2040Commands[command][1]); // type is second parameter
      break;
  }
}
void cmd_getLabel(uint8_t command, char* destBuffer) {
  switch (cmdSet) {
    case 0:
      // bq2040 commands
      strlcpy_P(destBuffer, (char*)pgm_read_word(&bq2040Labels[command]), bufferLen-1);
      break;
  }
}
char** cmd_getPtr() {
  switch (cmdSet) {
    case 0:
      return (char**)&bq2040Labels;
      break;
  }
}
uint8_t cmd_getLength() {
  switch (cmdSet) {
    case 0:
      return (sizeof(bq2040Labels) / sizeof(&bq2040Labels));
      break;
  }
}

// Define menu structure
phi_prompt_struct mainMenu;
phi_prompt_struct setupMenu;
phi_prompt_struct readMenu;
phi_prompt_struct controlMenu;
phi_prompt_struct commandMenu;
phi_prompt_struct inputBin;
phi_prompt_struct inputHex;
phi_prompt_struct singleCmdList;

const char main_menu_item00[] PROGMEM = "Setup";
const char main_menu_item01[] PROGMEM = "Read info";
const char main_menu_item02[] PROGMEM = "Control";
const char * const main_menu_items[] PROGMEM = {main_menu_item00, main_menu_item01, main_menu_item02};

const char setup_menu_item00[] PROGMEM = "Test SMBus";
const char setup_menu_item01[] PROGMEM = "Scan SMBus";
const char setup_menu_item02[] PROGMEM = "Enter address";
const char setup_menu_item03[] PROGMEM = "Set command";
const char setup_menu_item04[] PROGMEM = "Main menu";
const char * const setup_menu_items[] PROGMEM = {setup_menu_item00, setup_menu_item01, setup_menu_item02, setup_menu_item03, setup_menu_item04};

const char read_menu_item00[] PROGMEM = "Battery ID";
const char read_menu_item01[] PROGMEM = "Charge data";
const char read_menu_item02[] PROGMEM = "Statistics";
const char read_menu_item03[] PROGMEM = "Main menu";
PROGMEM const char * const read_menu_items[] = {read_menu_item00, read_menu_item01, read_menu_item02, read_menu_item03};

const char control_menu_item00[] PROGMEM = "Single command";
const char control_menu_item01[] PROGMEM = "Write word";
const char control_menu_item02[] PROGMEM = "Read word";
const char control_menu_item03[] PROGMEM = "Read block";
const char control_menu_item04[] PROGMEM = "Main menu";
PROGMEM const char * const control_menu_items[] = {control_menu_item00, control_menu_item01, control_menu_item02, control_menu_item03, control_menu_item04};

// Set introduction animation
byte lcdCustomCharBuffer[8][8];
const byte lcdStartupLogo_0[8][8] PROGMEM = {
{B00011, B11111, B10000, B10000, B10000, B10000, B10000, B10000},
{B11000, B11111, B00001, B00001, B00001, B00001, B00001, B00001},
{B00011, B11111, B10000, B10000, B10000, B10000, B10000, B10000},
{B11000, B11111, B00001, B00001, B00001, B00001, B00001, B00001},
{B10000, B10000, B10000, B10000, B10000, B10000, B10000, B11111},
{B00001, B00001, B00001, B00001, B00001, B00001, B00001, B11111},
{B10000, B10000, B10000, B10000, B10000, B10000, B10000, B11111},
{B00001, B00001, B00001, B00001, B00001, B00001, B00001, B11111}};
const byte lcdStartupLogo_1[8][8] PROGMEM = {
{B00011, B11111, B10000, B10000, B10000, B10000, B10000, B10000},
{B11000, B11111, B00001, B00001, B00001, B00001, B00001, B00001},
{B00011, B11111, B10000, B10000, B10000, B10000, B10000, B10000},
{B11000, B11111, B00001, B00001, B00001, B00001, B00001, B00001},
{B10000, B10000, B10000, B10111, B10111, B10111, B10000, B11111},
{B00001, B00001, B00001, B11101, B11101, B11101, B00001, B11111},
{B10000, B10000, B10000, B10111, B10111, B10111, B10000, B11111},
{B00001, B00001, B00001, B11101, B11101, B11101, B00001, B11111}};
const byte lcdStartupLogo_2[8][8] PROGMEM = {
{B00011, B11111, B10000, B10000, B10000, B10000, B10000, B10000},
{B11000, B11111, B00001, B00001, B00001, B00001, B00001, B00001},
{B00011, B11111, B10000, B10000, B10000, B10000, B10000, B10000},
{B11000, B11111, B00001, B00001, B00001, B00001, B00001, B00001},
{B10111, B10111, B10111, B10111, B10111, B10111, B10000, B11111},
{B11101, B11101, B11101, B11101, B11101, B11101, B00001, B11111},
{B10111, B10111, B10111, B10111, B10111, B10111, B10000, B11111},
{B11101, B11101, B11101, B11101, B11101, B11101, B00001, B11111}};
const byte lcdStartupLogo_3[8][8] PROGMEM = {
{B00011, B11111, B10000, B10000, B10000, B10111, B10111, B10111},
{B11000, B11111, B00001, B00001, B00001, B11101, B11101, B11101},
{B00011, B11111, B10000, B10000, B10000, B10111, B10111, B10111},
{B11000, B11111, B00001, B00001, B00001, B11101, B11101, B11101},
{B10111, B10111, B10111, B10111, B10111, B10111, B10000, B11111},
{B11101, B11101, B11101, B11101, B11101, B11101, B00001, B11111},
{B10111, B10111, B10111, B10111, B10111, B10111, B10000, B11111},
{B11101, B11101, B11101, B11101, B11101, B11101, B00001, B11111}};
const byte lcdStartupLogo_4[8][8] PROGMEM = {
{B00011, B11111, B10000, B10111, B10111, B10111, B10111, B10111},
{B11000, B11111, B00001, B11101, B11101, B11101, B11101, B11101},
{B00011, B11111, B10000, B10111, B10111, B10111, B10111, B10111},
{B11000, B11111, B00001, B11101, B11101, B11101, B11101, B11101},
{B10111, B10111, B10111, B10111, B10111, B10111, B10000, B11111},
{B11101, B11101, B11101, B11101, B11101, B11101, B00001, B11111},
{B10111, B10111, B10111, B10111, B10111, B10111, B10000, B11111},
{B11101, B11101, B11101, B11101, B11101, B11101, B00001, B11111}};

void setup() {
  i2c_init();
  PORTC = (1 << PORTC4) | (1 << PORTC5); // Enable pullups

  byte x, y, n;
  lcd.begin(lcd_columns, lcd_rows);
  lcd.clear();
  lcd.print("BattMon 2.0 ");
  lcd.write((uint8_t)0); lcd.write((uint8_t)1); lcd.write((uint8_t)2); lcd.write((uint8_t)3);
  lcd.setCursor(0, 1);
  lcd.print("for Arduino ");
  lcd.write(4); lcd.write(5); lcd.write(6); lcd.write(7);
  for (n=0; n<2; n++) {
    for (x=0; x<=7; x++) for (y=0; y<=7; y++) lcdCustomCharBuffer[x][y] = pgm_read_byte(&lcdStartupLogo_0[x][y]);
    for (x=0; x<=7; x++) lcd.createChar(x, lcdCustomCharBuffer[x]);
    delay(600);
    for (x=0; x<=7; x++) for (y=0; y<=7; y++) lcdCustomCharBuffer[x][y] = pgm_read_byte(&lcdStartupLogo_1[x][y]);
    for (x=0; x<=7; x++) lcd.createChar(x, lcdCustomCharBuffer[x]);
    delay(400);
    for (x=0; x<=7; x++) for (y=0; y<=7; y++) lcdCustomCharBuffer[x][y] = pgm_read_byte(&lcdStartupLogo_2[x][y]);
    for (x=0; x<=7; x++) lcd.createChar(x, lcdCustomCharBuffer[x]);
    delay(400);
    for (x=0; x<=7; x++) for (y=0; y<=7; y++) lcdCustomCharBuffer[x][y] = pgm_read_byte(&lcdStartupLogo_3[x][y]);
    for (x=0; x<=7; x++) lcd.createChar(x, lcdCustomCharBuffer[x]);
    delay(400);
    for (x=0; x<=7; x++) for (y=0; y<=7; y++) lcdCustomCharBuffer[x][y] = pgm_read_byte(&lcdStartupLogo_4[x][y]);
    for (x=0; x<=7; x++) lcd.createChar(x, lcdCustomCharBuffer[x]);
    delay(400);
  }
  delay(600);
  for (x=0; x<=20; x++) {
    lcdCharShiftRight(0, 3);
    lcdCharShiftRight(4, 7);
    delay(20);
  }
  delay(200);

  lcdReinitPhi();
  // Initialize the menus only once, so they stay persistent between submenus
  commandMenu.low.i = mainMenu.low.i = setupMenu.low.i = readMenu.low.i = controlMenu.low.i = singleCmdList.low.i = 0; // Default item highlighted on the list
  commandMenu.width = mainMenu.width = setupMenu.width = readMenu.width = controlMenu.width = singleCmdList.width = lcd_columns - ((global_style&phi_prompt_arrow_dot) != 0) - ((global_style&phi_prompt_scroll_bar) != 0); // Auto fit the size of the list to the screen. Length in characters of the longest list item
  commandMenu.step.c_arr[0] = mainMenu.step.c_arr[0] = setupMenu.step.c_arr[0] = readMenu.step.c_arr[0] = controlMenu.step.c_arr[0] = singleCmdList.step.c_arr[0] = lcd_rows; // Rows to auto fit entire screen
  commandMenu.step.c_arr[1] = mainMenu.step.c_arr[1] = setupMenu.step.c_arr[1] = readMenu.step.c_arr[1] = controlMenu.step.c_arr[1] = singleCmdList.step.c_arr[1] = 1; // One col list
  commandMenu.step.c_arr[2] = mainMenu.step.c_arr[2] = setupMenu.step.c_arr[2] = readMenu.step.c_arr[2] = controlMenu.step.c_arr[2] = singleCmdList.step.c_arr[2] = 0; // Row for current/total indicator, or 123^56 list (row 0)
  commandMenu.step.c_arr[3] = mainMenu.step.c_arr[3] = setupMenu.step.c_arr[3] = readMenu.step.c_arr[3] = controlMenu.step.c_arr[3] = singleCmdList.step.c_arr[3] = lcd_columns - 4 - ((global_style&phi_prompt_index_list) != 0) - ((global_style&phi_prompt_scroll_bar) != 0); // Col for current/total indicator or list (scrollbar minus 4 minus one for index list)
  commandMenu.col = mainMenu.col = setupMenu.col = readMenu.col = controlMenu.col = singleCmdList.col = 0; // Display menu at column 0
  commandMenu.row = mainMenu.row = setupMenu.row = readMenu.row = controlMenu.row = singleCmdList.row = 0; // Display menu at row 1
  commandMenu.option = mainMenu.option = setupMenu.option = readMenu.option = controlMenu.option = singleCmdList.option = global_style; // Option 0 display classic list, option 1 display 2x2 list, option 2 display list with index, option 3 display list with index2

  mainMenu.ptr.list = (char**)&main_menu_items; // Assign the list to the pointer
  mainMenu.high.i = (sizeof(main_menu_items) / sizeof(&main_menu_items)) - 1; // Last item of the list is size of the list - 1
  setupMenu.ptr.list = (char**)&setup_menu_items;
  setupMenu.high.i = (sizeof(setup_menu_items) / sizeof(&setup_menu_items)) - 1;
  readMenu.ptr.list = (char**)&read_menu_items;
  readMenu.high.i = (sizeof(read_menu_items) / sizeof(&read_menu_items)) - 1;
  controlMenu.ptr.list = (char**)&control_menu_items;
  controlMenu.high.i = (sizeof(control_menu_items) / sizeof(&control_menu_items)) - 1;
  commandMenu.ptr.list = (char**)&cmdset_items;
  commandMenu.high.i = (sizeof(cmdset_items) / sizeof(&cmdset_items)) - 1;
}

void loop() {
  while (true) { // This loops every time a menu item is selected
    lcd.clear(); // Refresh menu if a button has been pushed
    select_list(&mainMenu); // Use the select_list to ask the user to select an item of the list, that is a menu item from your menu
    while (wait_on_escape(25)); // Let go please
    switch (mainMenu.low.i) {
      case 0:
        DisplaySetupMenu();
        break;
      case 1:
        DisplayReadMenu();
        break;
      case 2:
        DisplayControlMenu(); // Display submenu, will return here upon exiting it
        break;
      default:
        break;
    }
    delay(500);
  }
}

void DisplaySetupMenu() {
  while (true) {
    lcd.clear();
    if (select_list(&setupMenu) == -3) return; // Left arrow -> go back to main menu
    while (wait_on_escape(25)); // Wait for buttons to be up, may have residual press from menu
    switch (setupMenu.low.i) {
      case 0:
        TestSMBus();
        break;
      case 1:
        ScanSMBus();
        break;
      case 2:
        EnterAddress();
        break;
      case 3:
        SetCommand();
        break;
      case 4:
        return; // Go back to main menu
        break;
      default:
        break;
    }
    delay(500);
  }
}

void DisplayReadMenu() {
  while (true) {
    lcd.clear();
    if (select_list(&readMenu) == -3) return; // Left arrow -> go back to main menu
    while (wait_on_escape(25)); // Wait for buttons to be up, may have residual press from menu
    switch (readMenu.low.i) {
      case 0:
        BatteryID();
        break;
      case 1:
        ChargeData();
        break;
      case 2:
        Statistics();
        break;
      case 3:
        return; // Go back to main menu
        break;
      default:
        break;
    }
    delay(500);
  }
}

void DisplayControlMenu() {
  while (true) {
    lcd.clear();
    if (select_list(&controlMenu) == -3) return; // Left arrow -> go back to main menu
    while (wait_on_escape(25)); // Wait for buttons to be up, may have residual press from menu
    switch (controlMenu.low.i) {
      case 0:
        SingleCommand();
        break;
      case 1:
        ControlWriteWord();
        break;
      case 2:
        ControlReadWord();
        break;
      case 3:
        ControlReadBlock();
        break;
      case 4:
        return; // Go back to main menu
        break;
      default:
        break;
    }
    delay(500);
  }
}

void TestSMBus() {
  byte x = 0;
  lcd.clear();
  msg_lcd(PSTR("Address: "));
  lcdPadBinary(deviceAddress, 7);
  lcd.setCursor(0, 1);

  if (deviceAddress == 0) {
    msg_lcd(PSTR("Invalid address!"));
    while (wait_on_escape(500) == 0); // Wait for button press
    while (wait_on_escape(25)); // Wait for release
    return; // Go back to setup menu
  }

  lcd.noCursor();
  lcd.noBlink();
  x = 0;
  msg_lcd(PSTR("... Checking ..."));

  while (!i2c_detect_device(deviceAddress)) {
    lcd.setCursor((x&B100)?x + 8:x, 1);
    lcd.write('.');
    x++;
    x &= B111; // Keep it under 8
    lcd.setCursor((x&B100)?x + 8:x, 1);
    lcd.write('>');
    if (wait_on_escape(25)) return;
  }
  lcd.setCursor(0, 1);
  msg_lcd(PSTR("  !! Found !!   "));

  while (wait_on_escape(500) == 0); // Wait for button press
  while (wait_on_escape(25)); // Wait for release
}

void ScanSMBus() {
  byte addr = 1;
  byte foundI2C = 0;
  byte cursorPos = 1;
  byte checkPos = 1;
  byte scanDirection = 2; // 0: don't change, 1: reverse scan, 2: forward scan (display function)
  byte i2cBitmap[8] = {0};
  int button_status;
  lcd.clear();
  msg_lcd(PSTR("Address    Found"));

  while ((addr > 0) && (addr < 128)) {
    lcd.setCursor(0, 1);
    lcdPadBinary(addr, 7);
    if (i2c_detect_device(addr)) {
      i2cBitmap[foundI2C] = addr;
      foundI2C++;
    }
    lcd.setCursor(11, 1);
    lcd.print(foundI2C, DEC);
    addr++;
    // Reserved addresses
    if (addr == 120) addr = 0;
    if (wait_on_escape(25)) return;
  }

  if (foundI2C == 0) {
    lcd.clear();
    ok_dialog(noDevicesFound);
    while (wait_on_escape(25)); // Wait for release
    return; // Go back to setup menu
  }
  if (foundI2C > 8) foundI2C = 8;

  while (true) {
    lcd.clear();
    msg_lcd(PSTR("Select     "));
    lcd.write(0x7E); // Place a '➔' special character here
    lcd.print(cursorPos, DEC);
    lcd.write('/');
    lcd.print(foundI2C, DEC);
    lcd.setCursor(0, 1);
    lcdPadBinary(i2cBitmap[cursorPos-1], 7);
    lcd.setCursor(11, 1);
    msg_lcd(PSTR(">OK<"));
    checkPos = cursorPos;
    scroll_bar_v(cursorPos * 99 / foundI2C, lcd_columns - 1, 0, lcd_rows);
    while ((button_status = wait_on_escape(500)) == 0);
    switch (button_status) {
      case 1:
        if ((--checkPos < 1) || (--checkPos > foundI2C)) {
          cursorPos = 1;
          scanDirection = 0;
        }
        else scanDirection = 1;
        break;
      case 2:
        if ((++checkPos < 1) || (++checkPos > foundI2C)) {
          cursorPos = foundI2C;
          scanDirection = 0;
        }
        else scanDirection = 2;
        break;
      case 5:
        deviceAddress = i2cBitmap[cursorPos-1];
        while (wait_on_escape(25)); // Wait for release
        return; // Go back to setup menu
        break;
      default:
        scanDirection = 0;
        break;
    }
    if (scanDirection == 1) cursorPos--;
    else if (scanDirection == 2) cursorPos++;
  }
}

void EnterAddress() {
  lcd.clear();
  msg_lcd(PSTR("Enter address:  ")); // Prompt user for input

  char textAddress[8] = {char('0' + bitRead(deviceAddress, 6)), char('0' + bitRead(deviceAddress, 5)), char('0' + bitRead(deviceAddress, 4)), char('0' + bitRead(deviceAddress, 3)), char('0' + bitRead(deviceAddress, 2)), char('0' + bitRead(deviceAddress, 1)), char('0' + bitRead(deviceAddress, 0))}; // This is the buffer that will store the content of the text panel
  inputBin.ptr.msg = textAddress; // Assign the text buffer address
  inputBin.low.c = '0'; // Text panel valid input starts with character '0'
  inputBin.high.c = '1'; // Text panel valid input ends with character '1'
  inputBin.width = 7; // Length of the input panel is 2 characters
  inputBin.col = 0; // Display input panel at column 2
  inputBin.row = 1; // Display input panel at row 1
  inputBin.option = 0;

  switch (input_panel(&inputBin)) {
    case 1:
      deviceAddress = strtoul(textAddress, NULL, 2);
      lcdClearSpace(0, 1, 7);
      lcdPadBinary(deviceAddress, 7);
      lcd.setCursor(12, 1);
      msg_lcd(PSTR(">OK<"));
      break;
    case -1: // Escape (go back to setup menu)
    case -3: // Left (at MSB, go back to setup menu)
      return;
      break;
    default:
      break;
  }

  while (wait_on_escape(500) == 0); // Wait for button press
  while (wait_on_escape(25)); // Wait for release
}

void SetCommand() {
  while (true) {
    lcd.clear();
    if (select_list(&commandMenu) == -3) return; // Left arrow -> go back to setup menu

    while (wait_on_escape(25)); // Wait for buttons to be up, may have residual press from menu
    if (commandMenu.low.i >= 0 && commandMenu.low.i <= (sizeof(cmdset_items) / sizeof(&cmdset_items)) - 1) {
      cmdSet = commandMenu.low.i; // Just set the selection to the new command set, looks valid
      deviceAddress = cmdset_addrs[cmdSet];
      return; // Go back to setup menu
    }
  }
}

void BatteryID() {
  lcd.clear();
  msg_lcd(PSTR("Mfg  Device Chem"));
  lcd.setCursor(0, 1);
  msg_lcd(PSTR("...Connecting..."));

  if (i2c_detect_device(deviceAddress)) {
    i2c_smbus_read_block(cmd_getCode(Cmd_ManufacturerName), i2cBuffer, bufferLen);
    lcdClearSpace(0, 1, 16);
    lcd.print(i2cBuffer);
    lcd.setCursor(5, 1);
    i2c_smbus_read_block(cmd_getCode(Cmd_DeviceName), i2cBuffer, bufferLen);
    lcd.print(i2cBuffer);
    lcdClearSpace(11, 1, 1); // Place a space character between DeviceName and DeviceChemistry
    lcd.setCursor(12, 1);
    i2c_smbus_read_block(cmd_getCode(Cmd_DeviceChemistry), i2cBuffer, bufferLen);
    lcd.print(i2cBuffer);
  }

  while (wait_on_escape(500) == 0); // Wait for button press
  while (wait_on_escape(25)); // Wait for release
}

void ChargeData() {
  int currVoltage, currAmps, estPercent, currTemp;
  int lastVoltage, lastAmps, lastPercent, lastTemp;
  int lowVoltage, lowAmps, lowTemp, highVoltage, highAmps, highTemp;
  byte x, y;
  lcd.clear();

  // Initialize the custom char buffer & clear characters
  for (x=0; x<=7; x++) {
    for (y=0; y<=7; y++) lcdCustomCharBuffer[x][y] = 0;
    lcd.createChar(x, lcdCustomCharBuffer[x]);
  }
  lcd.setCursor(0, 0);
  msg_lcd(PSTR("V    A    % "));
  lcd.write(0xDF); // Place a '°' special character here
  lcd.write('C');
  lcd.setCursor(2, 0);
  lcd.write((uint8_t)0); lcd.write((uint8_t)1);
  lcd.setCursor(7, 0);
  lcd.write(2); lcd.write(3);
  lcd.setCursor(14, 0);
  lcd.write(4); lcd.write(5);
  lcd.setCursor(0, 1);
  msg_lcd(PSTR("...Connecting..."));

  if (i2c_detect_device(deviceAddress)) {
    lowVoltage = i2c_smbus_read_word(cmd_getCode(Cmd_EDVF));
    lowVoltage = lastVoltage = abs(lowVoltage);
    highVoltage = i2c_smbus_read_word(cmd_getCode(Cmd_ChargingVoltage));
    lowAmps = lastAmps = 32767;
    highAmps = 0;
    lastPercent = 0;
    lowTemp = lastTemp = 32767;
    highTemp = 0;

    do {
      currVoltage = i2c_smbus_read_word(cmd_getCode(Cmd_Voltage));
      currAmps = i2c_smbus_read_word(cmd_getCode(Cmd_Current));
      estPercent = i2c_smbus_read_word(cmd_getCode(Cmd_RelativeStateOfCharge));
      currTemp = i2c_smbus_read_word(cmd_getCode(Cmd_Temperature));
      if (currVoltage != lastVoltage && currVoltage >= (lowVoltage / 2)) {
        lastVoltage = currVoltage;
        lcdClearSpace(0, 1, 5);
        lcd.print((float)currVoltage / 1000, 2);
        lcdCharShiftLeft(0, 1);
        y = map(currVoltage, lowVoltage, highVoltage, 8, 0);
        for (x=0; x<=7; x++) if (x >= y) bitSet(lcdCustomCharBuffer[1][x], 0);
        lcd.createChar(1, lcdCustomCharBuffer[1]);
      }
      if (currAmps != lastAmps) {
        lastAmps = currAmps;
        lcdClearSpace(5, 1, 4);
        lcd.print((float)currAmps / 1000, 1);
        highAmps = max(highAmps, currAmps + 100); // +/- 0.1 Amps
        lowAmps = min(lowAmps, currAmps - 100);
        lcdCharShiftLeft(2, 3);
        y = map(currAmps, lowAmps, highAmps, 8, 0);
        for (x=0; x<=7; x++) if (x >= y) bitSet(lcdCustomCharBuffer[3][x], 0);
        lcd.createChar(3, lcdCustomCharBuffer[3]);
      }
      if (estPercent != lastPercent) {
        lastPercent = estPercent;
        lcdClearSpace(9, 1, 3);
        lcd.print(estPercent, DEC);
      }
      if (currTemp != lastTemp) {
        lastTemp = currTemp;
        lcdClearSpace(12, 1, 4);
        lcd.print((float)currTemp / 10 - 273.15, 1);
        highTemp = max(highTemp, currTemp + 10); // +/- 0.1 deg K
        lowTemp = min(lowTemp, currTemp - 10);
        lcdCharShiftLeft(4, 5);
        y = map(currTemp, lowTemp, highTemp, 8, 0);
        for (x=0; x<=7; x++) if (x >= y) bitSet(lcdCustomCharBuffer[5][x], 0);
        lcd.createChar(5, lcdCustomCharBuffer[5]);
      }
    } while (wait_on_escape(500) == 0); // Wait for button press
    while (wait_on_escape(25)); // Wait for release
  }
  else {
    while (wait_on_escape(500) == 0); // Wait for button press
    while (wait_on_escape(25)); // Wait for release
  }

  lcd.clear();
  msg_lcd(PSTR("V    A    % ")); // Display charge data submenu again during the lcd object reinitialization
  lcd.write(0xDF); // Place a '°' special character here
  lcd.write('C');
  lcd.setCursor(0, 1);
  msg_lcd(PSTR(".Reinitializing."));
  lcdReinitPhi();
}

void Statistics() {
  int wordBuffer;
  lcd.clear();
  msg_lcd(PSTR("RmgCapacity #Cyc"));
  lcd.setCursor(0, 1);
  msg_lcd(PSTR("...Connecting..."));

  if (i2c_detect_device(deviceAddress)) {
    wordBuffer = i2c_smbus_read_word(cmd_getCode(Cmd_RemainingCapacity));
    lcdClearSpace(0, 1, 16);
    lcd.print(wordBuffer, DEC);
    lcd.setCursor(12, 1);
    wordBuffer = i2c_smbus_read_word(cmd_getCode(Cmd_CycleCount));
    lcd.print(wordBuffer, DEC);
  }

  while (wait_on_escape(500) == 0); // Wait for button press
  while (wait_on_escape(25)); // Wait for release
}

void SingleCommand() {
  int wordBuffer;
  double valueBuffer;
  singleCmdList.ptr.list = cmd_getPtr(); // Grab the currently selected command list in a pointer
  singleCmdList.high.i = cmd_getLength() - 1;

  while (true) {
    lcd.clear();
    if (select_list(&singleCmdList) == -3) return; // Left arrow -> go back to control menu
    lcd.clear();
    msg_lcd(PSTR("Hung? Check con."));
    lcd.setCursor(0, 1);
    msg_lcd(PSTR("...Connecting..."));

    if (i2c_detect_device(deviceAddress)) {
      if (cmd_getType(singleCmdList.low.i) < BATT_STRING) {
        wordBuffer = i2c_smbus_read_word(cmd_getCode(singleCmdList.low.i));
      }
      else if (cmd_getType(singleCmdList.low.i) == BATT_STRING) {
        i2c_smbus_read_block(cmd_getCode(singleCmdList.low.i), i2cBuffer, bufferLen);
      }
      else return;

      switch (cmd_getType(singleCmdList.low.i)) {
        case BATT_BITFIELD:
          fmtBinary((uint16_t)wordBuffer, 16, i2cBuffer, bufferLen);
          break;
        case BATT_DEC:
          sprintf(i2cBuffer, "%d", wordBuffer);
          break;
        case BATT_HEX:
          strcpy_P(i2cBuffer, PSTR("0x"));
          sprintf(i2cBuffer + strcspn(i2cBuffer, 0), "%04X", wordBuffer);
          break;
        case BATT_MAH:
          valueBuffer = (float)wordBuffer / 1000;
          fmtDouble(valueBuffer, 6, i2cBuffer, bufferLen);
          strcpy_P(i2cBuffer + strcspn(i2cBuffer, 0), PSTR(" Ah"));
          break;
        case BATT_MA:
          valueBuffer = (float)wordBuffer / 1000;
          fmtDouble(valueBuffer, 6, i2cBuffer, bufferLen);
          strcpy_P(i2cBuffer + strcspn(i2cBuffer, 0), PSTR(" A"));
          break;
        case BATT_MV:
          valueBuffer = (float)wordBuffer / 1000;
          fmtDouble(valueBuffer, 6, i2cBuffer, bufferLen);
          strcpy_P(i2cBuffer + strcspn(i2cBuffer, 0), PSTR(" V"));
          break;
        case BATT_MINUTES:
          sprintf(i2cBuffer, "%d", wordBuffer);
          strcpy_P(i2cBuffer + strcspn(i2cBuffer, 0), PSTR(" Minutes"));
          break;
        case BATT_PERCENT:
          sprintf(i2cBuffer, "%d", wordBuffer);
          strcpy_P(i2cBuffer + strcspn(i2cBuffer, 0), PSTR(" %"));
          break;
        case BATT_TENTH_K:
          valueBuffer = (float)wordBuffer / 10 - 273.15;
          fmtDouble(valueBuffer, 6, i2cBuffer, bufferLen);
          strcpy_P(i2cBuffer + strcspn(i2cBuffer, 0), PSTR("\xDF")); // Place a '°' special character here
          strcpy_P(i2cBuffer + strcspn(i2cBuffer, 0), PSTR("C"));
          break;
        case BATT_DATE:
          sprintf(i2cBuffer, "%02d", (uint8_t)wordBuffer & B00001111); // Day value
          strcpy_P(i2cBuffer + strcspn(i2cBuffer, 0), PSTR("/"));
          sprintf(i2cBuffer + strcspn(i2cBuffer, 0), "%02d", (uint8_t)(wordBuffer >> 5) & B00001111); // Month value
          strcpy_P(i2cBuffer + strcspn(i2cBuffer, 0), PSTR("/"));
          sprintf(i2cBuffer + strcspn(i2cBuffer, 0), "%04d", (wordBuffer >> 9) + 1980); // Year value
          break;
      }
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print(i2cBuffer);
      lcd.setCursor(0, 0);
      cmd_getLabel(singleCmdList.low.i, i2cBuffer);
      lcd.print(i2cBuffer);
    }

    while (wait_on_escape(500) == 0); // Wait for button press
    while (wait_on_escape(25)); // Wait for release
    return; // Go back to control menu
  }
}

void ControlWriteWord() {
  char textAddress[3] = "00"; // This is the buffer that will store the content of the text panel
  char textValue[5] = "0000";
  int menuSelection;
  lcd.clear(); // Clear the lcd
  msg_lcd(PSTR("Addr -WRITE- Val")); // Prompt user for input
  lcd.setCursor(0, 1);
  msg_lcd(PSTR("...Connecting..."));

  if (i2c_detect_device(deviceAddress)) {
    lcd.setCursor(0, 1);
    msg_lcd(PSTR("0x..      0x0000"));

    // Common parameters
    inputHex.low.c = 'A'; // Text panel valid input starts with character 'A'
    inputHex.high.c = 'F'; // Text panel valid input ends with character 'F'
    inputHex.row = 1; // Display input panel at row 1
    inputHex.option = 1; // Option 1 incluess 0-9 as valid characters

    while (true) { // This way we can go back and forth, come back to it when we loop back from value field
      inputHex.ptr.msg = textAddress; // Assign the text buffer address
      inputHex.width = 2; // Length of the input panel is 2 characters
      inputHex.col = 2; // Display input panel at column 2 (at "0x__")
      switch (input_panel(&inputHex)) {
        case -4: // Right (at LSB, go right to the value)
        case 1: // Enter (confirm address, go to the value)
          serialCommand = strtoul(textAddress, NULL, 16);
          lcdClearSpace(2, 1, 2);
          lcd.print(serialCommand, HEX);
          inputHex.ptr.msg = textValue; // Assign the text buffer value
          inputHex.width = 4; // Length of the input panel is 4 characters
          inputHex.col = 12; // Display input panel at column 12 (right side of screen, at "0x____")
          while (wait_on_escape(25)); // Wait for buttons to be up, may have residual press from menu
          menuSelection = input_panel(&inputHex);
          while (wait_on_escape(25)); // Wait for release
          serialData = strtoul(textValue, NULL, 16);
          lcdClearSpace(12, 1, 4);
          lcd.print(serialData, HEX);
          switch (menuSelection) {
            case 1: // Enter (confirm all, perform write)
              i2c_smbus_write_word(serialCommand, serialData); // Write value with command (value converted from string in default above, command converted before we got here)
              lcd.setCursor(5, 1);
              msg_lcd(PSTR(">OK<"));
              while (wait_on_escape(500) == 0); // Wait for button press
              while (wait_on_escape(25)); // Wait for release
            case -1: // Escape (go back to control menu)
              return;
              break;
            case -3: // Left (at MSB, go back to address)
              break;
          }
          break;
        case -1: // Escape (go back to control menu)
        case -3: // Left (at MSB, go back to control menu)
          return;
          break;
        default: // Invalid
          break;
      }
    }
  }
  else {
    while (wait_on_escape(500) == 0); // Wait for button press
    while (wait_on_escape(25)); // Wait for release
  }
}

void ControlReadWord() {
  char textAddress[3] = "00"; // This is the buffer that will store the content of the text panel
  lcd.clear(); // Clear the lcd
  msg_lcd(PSTR("Addr -READ-  Val")); // Prompt user for input
  lcd.setCursor(0, 1);
  msg_lcd(PSTR("...Connecting..."));

  if (i2c_detect_device(deviceAddress)) {
    lcd.setCursor(0, 1);
    msg_lcd(PSTR("0x..      0x...."));

    // Common parameters
    inputHex.ptr.msg = textAddress; // Assign the text buffer address
    inputHex.low.c = 'A'; // Text panel valid input starts with character 'A'
    inputHex.high.c = 'F'; // Text panel valid input ends with character 'F'
    inputHex.width = 2; // Length of the input panel is 2 characters
    inputHex.col = 2; // Display input panel at column 2
    inputHex.row = 1; // Display input panel at row 1
    inputHex.option = 1; // Option 1 incluess 0-9 as valid characters

    switch (input_panel(&inputHex)) {
      case 1:
        serialCommand = strtoul(textAddress, NULL, 16);
        serialData = i2c_smbus_read_word(serialCommand);
        lcdClearSpace(12, 1, 4);
        lcd.print(serialData, HEX);
        break;
      case -1: // Escape (go back to control menu)
      case -3: // Left (at MSB, go back to control menu)
        return;
        break;
      default: // Invalid
        break;
    }
  }

  while (wait_on_escape(500) == 0); // Wait for button press
  while (wait_on_escape(25)); // Wait for release
}

void ControlReadBlock() {
  char textAddress[3] = "00"; // This is the buffer that will store the content of the text panel
  byte bytesReceived;
  lcd.clear(); // Clear the lcd
  msg_lcd(PSTR("Block Read: Addr")); // Prompt user for input
  lcd.setCursor(0, 1);
  msg_lcd(PSTR("...Connecting..."));

  if (i2c_detect_device(deviceAddress)) {
    lcdClearSpace(0, 1, 16);

    // Common parameters
    inputHex.ptr.msg = textAddress; // Assign the text buffer address
    inputHex.low.c = 'A'; // Text panel valid input starts with character 'A'
    inputHex.high.c = 'F'; // Text panel valid input ends with character 'F'
    inputHex.width = 3; // Length of the input panel is 3 characters
    inputHex.col = 7; // Display input panel at column 7
    inputHex.row = 1; // Display input panel at row 1
    inputHex.option = 1; // Option 1 incluess 0-9 as valid characters. Option 0, default, option 1 include 0-9 as valid inputs

    if (input_panel(&inputHex) == 1) { // Only one case here, we want ENTER. Everything else escapes back to control menu
      serialCommand = strtoul(textAddress, NULL, 16);
      bytesReceived = i2c_smbus_read_block(serialCommand, i2cBuffer, bufferLen);
      lcdClearSpace(0, 0, 16);
      msg_lcd(PSTR("Cmd "));
      lcd.print(serialCommand, HEX);
      msg_lcd(PSTR(": "));
      lcd.print(bytesReceived, DEC);
      msg_lcd(PSTR(" Bytes"));
      lcd.setCursor(0, 1);
      lcdClearSpace(0, 1, 16);
      if (bytesReceived > 0 && bytesReceived <= 16) lcd.print(i2cBuffer); // More than 0 bytes, less than 16
      else msg_lcd(PSTR("ERR:Invalid data"));
    }
  }

  while (wait_on_escape(500) == 0); // Wait for button press
  while (wait_on_escape(25)); // Wait for release
}
