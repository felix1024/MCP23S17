/* MIT License

Copyright (c) 2024 Felix Thommen

Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Parts of this software is taken from existing library to the MCP23017 chip (i2c).
It has a identical software interface but using SPI hardware interface. -> MCP23S17
i2c chip library -> https://github.com/blemasle/arduino-mcp23017
*/

#include "MCP23S17.h"

MCP23S17::MCP23S17(uint8_t address, uint8_t ss) {
	_address = address;
	_ss = ss;
};

void MCP23S17::begin()
{
	// initialize chipselect PIN
  	::digitalWrite(_ss, HIGH);
  	::pinMode(_ss, OUTPUT);
  	// define SPI clockspeed, endian and mode
  	_spiSettings = SPISettings(1000000UL, MSBFIRST, SPI_MODE0);
  	SPI.begin();
  	
  	//BANK = 	0 : sequential register addresses
	//MIRROR = 	0 : use configureInterrupt 
	//SEQOP = 	1 : sequential operation disabled, address pointer does not increment
	//DISSLW = 	0 : slew rate enabled
	//HAEN = 	1 : hardware address
	//ODR = 	0 : active driver output (INTPOL bit sets the polarity.)
	//INTPOL = 	0 : interrupt active low
	//UNIMPLMENTED 	0 : unimplemented: Read as ‘0’
	writeRegister(MCP23S17Register::IOCON, 0b00101000);
}

void MCP23S17::portMode(MCP23S17Port port, uint8_t directions, uint8_t pullups, uint8_t inverted)
{
	writeRegister(MCP23S17Register::IODIR_A + port, directions);
	writeRegister(MCP23S17Register::GPPU_A + port, pullups);
	writeRegister(MCP23S17Register::IPOL_A + port, inverted);
}

void MCP23S17::pinMode(uint8_t pin, uint8_t mode, bool inverted)
{
	MCP23S17Register iodirreg = MCP23S17Register::IODIR_A;
	MCP23S17Register pullupreg = MCP23S17Register::GPPU_A;
	MCP23S17Register polreg = MCP23S17Register::IPOL_A;
	uint8_t iodir, pol, pull;

	if(pin > 7)
	{
		iodirreg = MCP23S17Register::IODIR_B;
		pullupreg = MCP23S17Register::GPPU_B;
		polreg = MCP23S17Register::IPOL_B;
		pin -= 8;
	}

	iodir = readRegister(iodirreg);
	if(mode == INPUT || mode == INPUT_PULLUP) bitSet(iodir, pin);
	else bitClear(iodir, pin);

	pull = readRegister(pullupreg);
	if(mode == INPUT_PULLUP) bitSet(pull, pin);
	else bitClear(pull, pin);

	pol = readRegister(polreg);
	if(inverted) bitSet(pol, pin);
	else bitClear(pol, pin);

	writeRegister(iodirreg, iodir);
	writeRegister(pullupreg, pull);
	writeRegister(polreg, pol);
}

void MCP23S17::digitalWrite(uint8_t pin, uint8_t state)
{
	MCP23S17Register gpioreg = MCP23S17Register::GPIO_A;
	uint8_t gpio;
	if(pin > 7)
	{
		gpioreg = MCP23S17Register::GPIO_B;
		pin -= 8;
	}

	gpio = readRegister(gpioreg);
	if(state == HIGH) bitSet(gpio, pin);
	else bitClear(gpio, pin);

	writeRegister(gpioreg, gpio);
}

uint8_t MCP23S17::digitalRead(uint8_t pin)
{
	MCP23S17Register gpioreg = MCP23S17Register::GPIO_A;
	uint8_t gpio;
	if(pin > 7)
	{
		gpioreg = MCP23S17Register::GPIO_B;
		pin -=8;
	}

	gpio = readRegister(gpioreg);
	if(bitRead(gpio, pin)) return HIGH;
	return LOW;
}

void MCP23S17::writePort(MCP23S17Port port, uint8_t value)
{
	writeRegister(MCP23S17Register::GPIO_A + port, value);
}

void MCP23S17::write(uint16_t value)
{
	writeRegister(MCP23S17Register::GPIO_A, lowByte(value), highByte(value));
}

uint8_t MCP23S17::readPort(MCP23S17Port port)
{
	return readRegister(MCP23S17Register::GPIO_A + port);
}

uint16_t MCP23S17::read()
{
	uint8_t a = readPort(MCP23S17Port::A);
	uint8_t b = readPort(MCP23S17Port::B);
	return a | b << 8;
}

void MCP23S17::writeRegister(MCP23S17Register reg, uint8_t value)
{
	::digitalWrite(_ss, LOW);
	SPI.beginTransaction(_spiSettings);
	SPI.transfer(OPCODEW | (_address << 1));
	SPI.transfer(static_cast<uint8_t>(reg));
	SPI.transfer(value);
	::digitalWrite(_ss, HIGH);
	SPI.endTransaction();
}

void MCP23S17::writeRegister(MCP23S17Register reg, uint8_t portA, uint8_t portB)
{
	::digitalWrite(_ss, LOW);
	SPI.beginTransaction(_spiSettings);
	SPI.transfer(OPCODEW | (_address << 1));
	SPI.transfer(static_cast<uint8_t>(reg));
	SPI.transfer(portA);
	SPI.transfer(portB);
	::digitalWrite(_ss, HIGH);
	SPI.endTransaction();
}


uint8_t MCP23S17::readRegister(MCP23S17Register reg)
{
	uint8_t value = 0;
	::digitalWrite(_ss, LOW);
	SPI.beginTransaction(_spiSettings);
	SPI.transfer(OPCODER | (_address << 1));
	SPI.transfer(static_cast<uint8_t>(reg));
	value = SPI.transfer(0x00);
	::digitalWrite(_ss, HIGH);
	SPI.endTransaction();
	return value;
}

void MCP23S17::readRegister(MCP23S17Register reg, uint8_t& portA, uint8_t& portB)
{
	::digitalWrite(_ss, LOW);
	SPI.beginTransaction(_spiSettings);
	SPI.transfer(OPCODER | (_address << 1));
	SPI.transfer(static_cast<uint8_t>(reg));
	portA = SPI.transfer(0x00);
	portB = SPI.transfer(0x00);
	::digitalWrite(_ss, HIGH);
	SPI.endTransaction();
}

#ifdef _MCP23S17_INTERRUPT_SUPPORT_

void MCP23S17::interruptMode(MCP23S17InterruptMode intMode)
{
	uint8_t iocon = readRegister(MCP23S17Register::IOCON);
	if(intMode == MCP23S17InterruptMode::Or) iocon |= static_cast<uint8_t>(MCP23S17InterruptMode::Or);
	else iocon &= ~(static_cast<uint8_t>(MCP23S17InterruptMode::Or));

	writeRegister(MCP23S17Register::IOCON, iocon);
}

void MCP23S17::interrupt(MCP23S17Port port, uint8_t mode)
{
	MCP23S17Register defvalreg = MCP23S17Register::DEFVAL_A + port;
	MCP23S17Register intconreg = MCP23S17Register::INTCON_A + port;

	//enable interrupt for port
	writeRegister(MCP23S17Register::GPINTEN_A + port, 0xFF);
	switch(mode)
	{
	case CHANGE:
		//interrupt on change
		writeRegister(intconreg, 0);
		break;
	case FALLING:
		//interrupt falling : compared against defval, 0xff
		writeRegister(intconreg, 0xFF);
		writeRegister(defvalreg, 0xFF);
		break;
	case RISING:
		//interrupt rising : compared against defval, 0x00
		writeRegister(intconreg, 0xFF);
		writeRegister(defvalreg, 0x00);
		break;
	}
}

void MCP23S17::interruptedBy(uint8_t& portA, uint8_t& portB)
{
	readRegister(MCP23S17Register::INTF_A, portA, portB);
}

void MCP23S17::disableInterrupt(MCP23S17Port port)
{
	writeRegister(MCP23S17Register::GPINTEN_A + port, 0x00);
}

void MCP23S17::clearInterrupts()
{
	uint8_t a, b;
	clearInterrupts(a, b);
}

void MCP23S17::clearInterrupts(uint8_t& portA, uint8_t& portB)
{
	readRegister(MCP23S17Register::INTCAP_A, portA, portB);
}

#endif
