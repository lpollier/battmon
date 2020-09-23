# BattMon 2.0

Arduino Uno + 2x16 LCD keypad shield -> BattMon 2.0

BattMon is an Arduino program that works with an Arduino Uno and a LCD keypad shield to monitor data coming from bq2040 gas gauge equipped in Lithium Ion Battery Pack such as for Aibo ERS2xx (ERA-201B1), ERS3xx (ERA-301B1) and ERS7 (ERA-7B1/ERA-7B2) series.

## Menus

```sh
Main menu
├── Setup
│   ├── Test SMBus
│   ├── Scan SMBus
│   ├── Enter address
│   ├── Set command
│   ├── Main menu
├── Read info
│   ├── Battery ID
│   ├── Charge data
│   ├── Statistics
│   ├── Main menu
├── Control
    ├── Single command
    ├── Write word
    ├── Read word
    ├── Read block
    ├── Main menu
```

## Libraries

This program requires the LiquidCrystal internal library and the following external libraries to import on Arduino IDE:

| Name | Link |
| ------ | ------ |
| phi_prompt | [libraries/phi_prompt.zip][phi_prompt] |
| phi_interfaces | [libraries/phi_interfaces.zip][phi_interfaces] |
| phi_buttons | [libraries/phi_buttons.zip][phi_buttons] |
| phi_interfaces | [libraries/phi_keypads.zip][phi_keypads] |
| i2cmaster | [libraries/i2cmaster.zip][i2cmaster] |

## Connection

Here is one connection example between a Sony Aibo ERA-201B1 battery and an Arduino Uno + 2x16 LCD keypad shield. The pin closest to the edge is 1 and the one at the opposite is 6:

* Pin 1 = Plus (+VBat)
* Pin 2 = SMBus Clock
* Pin 3 = SMBus Data
* Pin 4 = /EN has to be connected to pin 6(-VBat) to activate the Arduino
* Pin 5 = Reserve (Internal temperature thermistor)
* Pin 6 = Minus (-VBat)

The Arduino is powered by the battery when pin 4 (/EN) is connected to pin 6 (-VBat) using a switch for example.

You can refer to the [ERA-201B1 wiring diagram][ERA-201B1_wiring_diagram] for more details.

## Documentation

* [Arduino Uno R3 schematic](https://content.arduino.cc/assets/UNO-TH_Rev3e_sch.pdf)
* [LCD Keypad Shield schematic](https://www.dfrobot.com/image/data/DFR0009/LCDKeypad%20Shield%20V1.0%20SCH.pdf)

## Setting

You may have to modify the keypads values in the [battmon][battmon] main file. Two sets are already available in the code for the values[] array, but if neither of the two works, you can find your own values using the [keypad_values_test][keypad_values_test] file.

## Caution

Requires some electronics skills. There is no guarantee. Use this tool at your own risk.

## To do

Add images to illustrate menu and submenus display.

## License
----

MIT

[i2cmaster]: <https://github.com/lpollier/battmon/blob/master/libraries/i2cmaster.zip>
[phi_buttons]: <https://github.com/lpollier/battmon/blob/master/libraries/phi_buttons.zip>
[phi_interfaces]: <https://github.com/lpollier/battmon/blob/master/libraries/phi_interfaces.zip>
[phi_keypads]: <https://github.com/lpollier/battmon/blob/master/libraries/phi_keypads.zip>
[phi_prompt]: <https://github.com/lpollier/battmon/blob/master/libraries/phi_prompt.zip>

[ERA-201B1_wiring_diagram]: <https://github.com/lpollier/battmon/blob/master/connection/ERA-201B1_wiring_diagram.png>

[battmon]: <https://github.com/lpollier/battmon/blob/master/battmon.ino>
[keypad_values_test]: <https://github.com/lpollier/battmon/blob/master/test/keypad_values_test.ino>
