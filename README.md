# BattMon 2.0

Monitors the charge specifications of your battery equipped with the bq2040 gas gauge.

BattMon 2.0 is a tool kit that works with an Arduino Uno and a 2x16 LCD keypad shield to access parameters from a bq2040 gas gauge such as Voltage, Current, State of charge, Temperature, Remaining capacity or Cycle count. This fuel gauge and other TI products like the bq2060, have been included in Lithium-Ion battery pack, especially for the Sony Aibo ERS-2xx (ERA-201B1), ERS-3xx (ERA-301B1) and ERS-7 (ERA-7B1/ERA-7B2) robot series.

## Menu tree

```sh
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

## Libraries

The program needs for its operation the internal LiquidCrystal library and the following external libraries which must be imported into your Arduino IDE:

| Name | Link |
| ------ | ------ |
| phi_prompt | [libraries/phi_prompt.zip][phi_prompt] |
| phi_interfaces | [libraries/phi_interfaces.zip][phi_interfaces] |
| phi_buttons | [libraries/phi_buttons.zip][phi_buttons] |
| phi_keypads | [libraries/phi_keypads.zip][phi_keypads] |
| i2cmaster | [libraries/i2cmaster.zip][i2cmaster] |

Go to Sketch > Include Library > Add .Zip Library and add them one by one. The LiquidCrystal library is already included. More help with this on [instructable][instructables].

Remember to place the [format][format], [i2c][i2c] and [lcd][lcd] dependency files in the same directory as [battmon][battmon] main file so that it can be compiled.

## Connection

Here is an example of connection between a Sony Aibo ERA-201B1 battery pack and the assembly of the Arduino Uno + 2x16 LCD keypad shield. The pin closest to the edge is 1 and the one opposite is 6:

![ERA-201B1 battery pack connection with BattMon 2.0 tool kit](https://raw.githubusercontent.com/lpollier/battmon/master/example/ERA-201B1_wiring_diagram.png)

* Pin 1 = Plus (Pack+)
* Pin 2 = SMBus Clock
* Pin 3 = SMBus Data
* Pin 4 = /EN has to be connected to pin 6 (Pack-) to activate the Arduino Uno
* Pin 5 = Reserve (Internal temperature thermistor)
* Pin 6 = Minus (Pack-)

The Arduino Uno is powered by the battery when pin 4 (/EN) is connected to pin 6 (Pack-), using a switch for example.

You can refer to the ERA-201B1 [wiring diagram][ERA-201B1_wiring_diagram] for more details.

For those who are interested, I offer a proposal for an electronic [reverse engineering schematic][ERA-201B1_reverse_engineering_schematic] of the PCB inside the ERA-201B1 battery pack and for the curious and do-it-yourself repair enthusiasts, I provide a [re-cell tutorial][ERA-201B1_recell_tutorial] on how to refill it.

## Setting

You may have to modify the keypads values in the [battmon][battmon] file. Two sets are already available in the code for the values[] array, but if neither of the two works, you can find your own values using the [keypad_values_test][keypad_values_test] file.

## Documentation

* [Arduino Uno R3 specification](https://docs.arduino.cc/hardware/uno-rev3)
* [Arduino Uno R3 schematic](https://content.arduino.cc/assets/UNO-TH_Rev3e_sch.pdf)
* [LCD Keypad Shield specification](https://wiki.dfrobot.com/LCD_KeyPad_Shield_For_Arduino_SKU__DFR0009)
* [LCD Keypad Shield schematic](https://www.dfrobot.com/image/data/DFR0009/LCDKeypad%20Shield%20V1.0%20SCH.pdf)

## Wiki

A [wiki][wiki] page is available with images to illustrate the display of the BattMon 2.0 tool kit menu and submenus.

## Disclaimer

It requires electronics skills. There is no guarantee. Use this repository code and associated data at your own risk.

## License

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

[i2cmaster]: <https://github.com/lpollier/battmon/blob/master/libraries/i2cmaster.zip>
[phi_buttons]: <https://github.com/lpollier/battmon/blob/master/libraries/phi_buttons.zip>
[phi_interfaces]: <https://github.com/lpollier/battmon/blob/master/libraries/phi_interfaces.zip>
[phi_keypads]: <https://github.com/lpollier/battmon/blob/master/libraries/phi_keypads.zip>
[phi_prompt]: <https://github.com/lpollier/battmon/blob/master/libraries/phi_prompt.zip>
[instructables]: <https://www.instructables.com/How-to-Add-an-External-Library-to-Arduino/>

[ERA-201B1_wiring_diagram]: <https://github.com/lpollier/battmon/blob/master/example/ERA-201B1_wiring_diagram.png>
[ERA-201B1_reverse_engineering_schematic]: <https://github.com/lpollier/battmon/blob/master/example/ERA-201B1_reverse_engineering_schematic.pdf>
[ERA-201B1_recell_tutorial]: <https://github.com/lpollier/battmon/blob/master/example/ERA-201B1_recell_tutorial.pdf>

[battmon]: <https://github.com/lpollier/battmon/blob/master/battmon.ino>
[format]: <https://github.com/lpollier/battmon/blob/master/format.ino>
[i2c]: <https://github.com/lpollier/battmon/blob/master/i2c.ino>
[lcd]: <https://github.com/lpollier/battmon/blob/master/lcd.ino>
[keypad_values_test]: <https://github.com/lpollier/battmon/blob/master/test/keypad_values_test.ino>

[wiki]: <https://github.com/lpollier/battmon/wiki>