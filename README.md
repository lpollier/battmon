# BattMon 2.0

Monitors the characteristics of your battery equipped with the bq2040 gas gauge.

BattMon 2.0 is a toolkit that works with an Arduino Uno and a 2x16 LCD keypad shield to access parameters from a bq2040 gas gauge such as Voltage, Current, State of Charge, Temperature, Remaining Capacity or Cycle Count. This fuel gauge and other TI products like the bq2060, have been included in Lithium-ion battery pack, especially for the Sony Aibo ERS-2xx (ERA-201B1), ERS-3xx (ERA-301B1) and ERS-7 (ERA-7B1/ERA-7B2) robot series.

## Connection

Here is an example of connection between a Sony Aibo ERA-201B1 battery pack and the assembly of the Arduino Uno + 2x16 LCD keypad shield. The pin closest to the edge is 1 and the one opposite is 6:

![ERA-201B1 battery pack connection with BattMon 2.0 toolkit](https://raw.githubusercontent.com/lpollier/battmon/master/example/ERA-201B1_wiring_diagram.png)

* Pin 1 = Plus [6.0V-8.4V] (Pack+)
* Pin 2 = SMBus Clock
* Pin 3 = SMBus Data
* Pin 4 = /EN has to be connected to pin 6 (Pack-) to activate the Arduino Uno
* Pin 5 = Reserved (Internal temperature thermistor)
* Pin 6 = Minus [0V] (Pack-)

The Arduino Uno is powered by the battery when pin 4 (/EN) is connected to pin 6 (Pack-) using a switch.

To avoid discharging the battery unnecessarily, you can also power the Arduino Uno with a USB cable. In this case, it is useless to connect pin 1 (Pack+) or the switch indicated in the [wiring diagram][ERA-201B1_wiring_diagram].

## Program

The BattMon 2.0 code consists of four Arduino files. A [battmon][battmon] main file and three [format][format], [i2c][i2c] and [lcd][lcd] dependency files. All these files are to be placed in a project created from the [Arduino IDE][Arduino_IDE] software (1.8.+) to compile the code and then upload the program into your Arduino Uno.

Remember to place these four files in a project directory whose name is that of the [battmon][battmon] main file so that it can be compiled.

## Libraries

The code also needs for its compilation the internal LiquidCrystal library and the following external libraries which must be imported into your Arduino IDE:

| Name | Link |
| ------ | ------ |
| phi_prompt | [libraries/phi_prompt.zip][phi_prompt] |
| phi_interfaces | [libraries/phi_interfaces.zip][phi_interfaces] |
| phi_buttons | [libraries/phi_buttons.zip][phi_buttons] |
| phi_keypads | [libraries/phi_keypads.zip][phi_keypads] |
| i2cmaster | [libraries/i2cmaster.zip][i2cmaster] |

Go to Sketch > Include Library > Add .Zip Library and add them one by one. The LiquidCrystal library is already included. More help with this on [instructables][instructables].

## Settings

You may have to modify the keypads values in the [battmon][battmon] main file. Two sets are already available in the code for the values[] array, but if none of them works, you can find your own values using the [keypad_values_test][keypad_values_test] file.

## Menu tree

This is the menu tree of the BattMon 2.0 program. The Setup submenu is used to test the connection, the Read info submenu to display the essential parameters of the pack and the Control submenu to read or write the other registers of the gas gauge.

```
Main menu
├── Setup
│   ├── Test SMBus
│   ├── Scan SMBus
│   ├── Enter address
│   ├── Set command
│   └── Main menu
├── Read info
│   ├── Battery ID
│   ├── Charge data
│   ├── Statistics
│   └── Main menu
└── Control
    ├── Single command
    ├── Write word
    ├── Read word
    ├── Read block
    └── Main menu
```

## Wiki

A [wiki][wiki] page is available to illustrate the display of the entire menu and submenus of the BattMon 2.0 program on the LCD keypad shield.

## Documentation

* [Arduino Uno R3 specification](https://docs.arduino.cc/hardware/uno-rev3)
* [Arduino Uno R3 schematic](https://content.arduino.cc/assets/UNO-TH_Rev3e_sch.pdf)
* [LCD Keypad Shield specification](https://wiki.dfrobot.com/LCD_KeyPad_Shield_For_Arduino_SKU__DFR0009)
* [LCD Keypad Shield schematic](https://www.dfrobot.com/image/data/DFR0009/LCDKeypad%20Shield%20V1.0%20SCH.pdf)

## Credits

BattMon 2.0 project is based on the [SMBusBattery_Phi/BattMon 1.0][SMBusBattery_Phi_BattMon_1.0] code available on the Arduino forum topic initiated by FalconFour.

## Bonus

If you are motivated, qualified and experienced in electronics, I propose a [reverse engineering schematic][ERA-201B1_reverse_engineering_schematic] of the Sony Aibo ERA-201B1 battery pack PCB and for the curious and do-it-yourself repair enthusiasts, I offer a [re-cell tutorial][ERA-201B1_recell_tutorial] on how to refill it.

There is also a promising [Aibo-ERS-xxx-battery][Aibo_ERS_xxx_battery] project by [r00li][r00li] who wants to use newer components to replace the original battery packs while remaining compatible with Aibo robots.

## Disclaimer

It requires electronics skills. There is no guarantee. Use this repository code and associated data at your own risk.

## License

This project is licensed under the terms of the MIT license.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

[ERA-201B1_wiring_diagram]: <https://github.com/lpollier/battmon/blob/master/example/ERA-201B1_wiring_diagram.png>
[ERA-201B1_reverse_engineering_schematic]: <https://github.com/lpollier/battmon/blob/master/example/ERA-201B1_reverse_engineering_schematic.pdf>
[ERA-201B1_recell_tutorial]: <https://github.com/lpollier/battmon/blob/master/example/ERA-201B1_recell_tutorial.pdf>

[battmon]: <https://github.com/lpollier/battmon/blob/master/battmon.ino>
[format]: <https://github.com/lpollier/battmon/blob/master/format.ino>
[i2c]: <https://github.com/lpollier/battmon/blob/master/i2c.ino>
[lcd]: <https://github.com/lpollier/battmon/blob/master/lcd.ino>

[Arduino_IDE]: <https://www.arduino.cc/en/software>

[phi_prompt]: <https://github.com/lpollier/battmon/blob/master/libraries/phi_prompt.zip>
[phi_interfaces]: <https://github.com/lpollier/battmon/blob/master/libraries/phi_interfaces.zip>
[phi_buttons]: <https://github.com/lpollier/battmon/blob/master/libraries/phi_buttons.zip>
[phi_keypads]: <https://github.com/lpollier/battmon/blob/master/libraries/phi_keypads.zip>
[i2cmaster]: <https://github.com/lpollier/battmon/blob/master/libraries/i2cmaster.zip>

[instructables]: <https://www.instructables.com/How-to-Add-an-External-Library-to-Arduino/>

[keypad_values_test]: <https://github.com/lpollier/battmon/blob/master/test/keypad_values_test.ino>

[wiki]: <https://github.com/lpollier/battmon/wiki>

[SMBusBattery_Phi_BattMon_1.0]: <https://forum.arduino.cc/t/smbus-laptop-battery-hacker-with-phi_prompt-lcd-user-interface/62728>

[Aibo_ERS_xxx_battery]: <https://github.com/r00li/Aibo-ERS-xxx-battery>
[r00li]: <https://github.com/r00li>