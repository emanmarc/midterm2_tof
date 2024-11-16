#include "pmodtof.h"

PmodToF::PmodToF(I2cCore& i2c) : i2cInterface(i2c) {}

bool PmodToF::initialize() {
    // Initialize ISL29501 with default settings (based on firmware routines)
    if (!writeRegister(0x10, 0x04)) return false; // Set integration time
    if (!writeRegister(0x11, 0x6E)) return false; // Set measurement period
    if (!writeRegister(0x13, 0x71)) return false; // Setup single-shot mode
    if (!writeRegister(0x60, 0x01)) return false; // Interrupt when data ready
    if (!writeRegister(0x18, 0x1B)) return false; // Optimize AGC
	if (!writeRegister(0x19, 0x23)) return false; // Optimize VGA
	if (!writeRegister(0x90, 0x0F)) return false; // Set emitter scale multiplier
	if (!writeRegister(0x91, 0xFF)) return false; // Set emitter current

    uart.disp("ISL29501 initialized successfully.\n\r");
    return true;
}

double PmodToF::getDistance() {
	if (!writeRegister(0x13, 0x71)) return -1; // Setup single-shot mode
	if (!writeRegister(0x60, 0x01)) return -1; // Interrupt when data ready

    // Wait for measurement to complete
    i2cInterface.set_tof_ss(0); // Pull SS low to start
    sleep_us(5600);
    i2cInterface.set_tof_ss(1); // Pull SS high
    sleep_us(14400);
    uint8_t unused;
    	if (!readRegister(0x69, unused)) {
//    	    uart.disp("Failed to clear interrupt register!\n\r");
    	    return -1;
    	}
//    	uart.disp("Interrupt register cleared: 0x");
//    	uart.disp(unused, 16);
//    	uart.disp("\n\r");
    // Poll for IRQ or use interrupt
    while (i2cInterface.get_tof_irq() == 0) {
//        uart.disp("Waiting for measurement to complete...\n\r");
    }

    // Read distance registers
    uint8_t msb, lsb;
    if (!readRegister(0xD1, msb)) return -1; // Distance MSB
    if (!readRegister(0xD2, lsb)) return -1; // Distance LSB
    double distance = 1;
    // Calculate distance using formula in datasheet
    distance = (((double)msb * 256 + (double)lsb) / 65536.0) * 33.31;

    return distance;
}

bool PmodToF::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t data[2] = { reg, value };
    int write_result = i2cInterface.write_transaction(ISL29501_I2C_ADDRESS, data, 2, 0);
//    uart.disp("Write to register 0x");
//    uart.disp(reg, 16);
//    uart.disp(": 0x");
//    uart.disp(value, 16);
//    uart.disp(write_result == 0 ? " (Success)\n\r" : " (Failed)\n\r");
    if (write_result != 0) {
            return false; // Write failed
        }

        // Verify the written value by reading it back
        uint8_t read_value = 0;
        if (!readRegister(reg, read_value)) {
//            uart.disp("Failed to verify register 0x");
//            uart.disp(reg, 16);
//            uart.disp("\n\r");
            return false;
        }

        // Compare written and read values
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
    if (result != 0) {
//        uart.disp("Failed to send read command for register 0x");
//        uart.disp(reg, 16);
//        uart.disp("\n\r");
        return false;
    }
    result = i2cInterface.read_transaction(ISL29501_I2C_ADDRESS, &value, 1, 0);
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

bool PmodToF::readRegisters(uint8_t reg, uint8_t* data, uint8_t length) {
    uint8_t temp = reg;
    int result = i2cInterface.write_transaction(ISL29501_I2C_ADDRESS, &temp, 1, 1);
    if (result != 0) {
//        uart.disp("Failed to send read command for multiple registers starting at 0x");
//        uart.disp(reg, 16);
//        uart.disp("\n\r");
        return false;
    }
    result = i2cInterface.read_transaction(ISL29501_I2C_ADDRESS, data, length, 0);
    if (result == 0) {
//        uart.disp("Read multiple registers starting at 0x");
//        uart.disp(reg, 16);
//        uart.disp(": ");
        for (uint8_t i = 0; i < length; i++) {
//            uart.disp("0x");
//            uart.disp(data[i], 16);
//            uart.disp(" ");
        }
//        uart.disp("(Success)\n\r");
    } else {
//        uart.disp("Failed to read multiple registers starting at 0x");
//        uart.disp(reg, 16);
//        uart.disp("\n\r");
    }
    return result == 0;
}
