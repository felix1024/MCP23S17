# MCP23S17
[![License](https://img.shields.io/badge/license-MIT%20License-blue.svg)](http://doge.mit-license.org)

The library has been derived from https://github.com/blemasle/arduino-mcp23017 which works for Microchip's MCP23017, same chip but with I2C interface. Goal was to provide the same interface to allow adaption of existing code easy to MCP23S17. (see also copyright notices)

This library provides full control over the Microchip's [MCP23S17](https://www.microchip.com/en-us/product/MCP23S17), including interrupt support. The library is provided for Arduino.

## Features
 * Individual pins read & write
 * Ports read & write
 * Registers read & write
 * Full interrupt support

## Usage
Unlike most Arduino library, no default instance is created when the library is included. It's up to you to create one using the appropriate chipselect pin and address based on  MCP23S17 `A0`, `A1` and `A2` pins wirings.
Available addresses go from `0x00` to `0x07`, allowing up to 8 MCP23S17 on the same SPI bus using the same chipselect pin.
Any available arduino GPIO pin can be used as chipselect.

```cpp
#include <Arduino.h>
#include <MCP23S17.h>

MCP23S17 mcp = MCP23S17(0x00);
```

Additionaly, you can specify the `Wire` instance to use as a second argument. For instance `MCP23017(0x24, Wire1)`.  
See included examples for further usage.

## Warning about adressing
When IOCON.HAEN = 0 (hardware addressing disabled): If the A2 pin is high, then the device must be addressed as A2, A1, A0, = 1xx (i.e., OPCODE = b”0100 1XX’.
