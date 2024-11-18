#include "pmodtof.h"

PmodToF::PmodToF(I2cCore& i2c) : i2cInterface(i2c) {
}
PmodToF::~PmodToF() {
}

bool PmodToF::initialize() {
    if (!writeRegister(0x10, 0x04)) return false; // integration time
    if (!writeRegister(0x11, 0x6E)) return false; // measurement period
    if (!writeRegister(0x13, 0x71)) return false; // single-shot mode
    if (!writeRegister(0x60, 0x01)) return false; // Interrupt when data ready
    if (!writeRegister(0x18, 0x1B)) return false; // AGC
	if (!writeRegister(0x19, 0x23)) return false; // VGA
	if (!writeRegister(0x90, 0x0F)) return false; // emitter scale multiplier
	if (!writeRegister(0x91, 0xFF)) return false; // emitter current

    return true;
}

double PmodToF::getDistance() {
	if (!writeRegister(0x13, 0x71)) return -1; // Setup single-shot mode
	if (!writeRegister(0x60, 0x01)) return -1; // Interrupt when data ready

    i2cInterface.set_tof_ss(0);
    sleep_us(5600);
    i2cInterface.set_tof_ss(1);
    sleep_us(14400);
    uint8_t unused;
    readRegister(0x69, unused); // clear interrupt flag
    while (i2cInterface.get_tof_irq() == 0) {
    }

    // Read distance registers
    uint8_t msb, lsb;
    if (!readRegister(0xD1, msb)) return -1; // Distance MSB
    if (!readRegister(0xD2, lsb)) return -1; // Distance LSB
    double distance = 1;
    distance = (((double)msb * 256 + (double)lsb) / 65536.0) * 33.31;

    return distance;
}

bool PmodToF::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t data[2] = { reg, value };
    i2cInterface.write_transaction(ISL29501_I2C_ADDRESS, data, 2, 0);
    //int write_result = i2cInterface.write_transaction(ISL29501_I2C_ADDRESS, data, 2, 0);
        // debugging
        uint8_t read_value = 0;
        if (!readRegister(reg, read_value)) {
//            uart.disp("Failed to verify register 0x");
//            uart.disp(reg, 16);
//            uart.disp("\n\r");
            return false;
        }
        if (read_value != value) {
//            uart.disp("Mismatch in register 0x");
//            uart.disp(reg, 16);
//            uart.disp(": wrote 0x");
//            uart.disp(value, 16);
//            uart.disp(", read 0x");
//            uart.disp(read_value, 16);
//            uart.disp("\n\r");
            return false;
        }

//        uart.disp("Verified register 0x");
//        uart.disp(reg, 16);
//        uart.disp(": 0x");
//        uart.disp(value, 16);
//        uart.disp("\n\r");
//        sleep_ms(1000);
        return true;
}

bool PmodToF::readRegister(uint8_t reg, uint8_t& value) {
    uint8_t temp = reg;
    int result = i2cInterface.write_transaction(ISL29501_I2C_ADDRESS, &temp, 1, 1);
    result = i2cInterface.read_transaction(ISL29501_I2C_ADDRESS, &value, 1, 0);
    //debugging
    if (result == 0) {
//        uart.disp("Read from register 0x");
//        uart.disp(reg, 16);
//        uart.disp(": 0x");
//        uart.disp(value, 16);
//        uart.disp(" (Success)\n\r");
    } else {
//        uart.disp("Failed to read from register 0x");
//        uart.disp(reg, 16);
//        uart.disp("\n\r");
    }
    return result == 0;
}

