#ifndef PMODTOF_H
#define PMODTOF_H

#include "i2c_core.h"

// ISL29501 I2C Address
#define ISL29501_I2C_ADDRESS 0x57

class PmodToF {
public:
    PmodToF(I2cCore& i2c); // Constructor
    ~PmodToF();

    bool initialize();
    double getDistance();

private:
    I2cCore& i2cInterface;

    // functions for register read/write using i2c
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t& value);

};

#endif // PMODTOF_H
