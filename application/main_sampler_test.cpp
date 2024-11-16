/*****************************************************************//**
 * @file main_sampler_test.cpp
 *
 * @brief Basic test of nexys4 ddr mmio cores
 *
 * @author p chu
 * @version v1.0: initial release
 *********************************************************************/

// #define _DEBUG
#include "chu_init.h"
#include "i2c_core.h"
#include "pmodtof.h"
/**
 * blink once per second for 5 times.
 * provide a sanity check for timer (based on SYS_CLK_FREQ)
 * @param led_p pointer to led instance
 */

int main() {
	I2cCore i2c(get_slot_addr(BRIDGE_BASE, S10_I2C));
	PmodToF tof(i2c);
//	if (!tof.initialize()) {
//				uart.disp("Failed to initialize ToF sensor\n\r");
//			}
	if (!tof.initialize()) {
			uart.disp("Failed to initialize ToF sensor\n\r");
		}
	while (1) {

//		uart.disp("Getting Distance...\n\r");
		double distance = tof.getDistance();

		uart.disp("Measured distance: ");
		uart.disp(distance);
		uart.disp(" meters\n\r");
		sleep_ms(500);
	}
    //read test on tof device id reg
//       uint8_t DEV_ADDR = 0x57;
//	   uint8_t wbytes[2], bytes[2];
//
//	   wbytes[0] = 0x00;
//	   i2c.write_transaction(DEV_ADDR, wbytes, 1, 1);
//
//	   i2c.read_transaction(DEV_ADDR, bytes, 1, 0);
//	   uart.disp("read id (should be): ");
//	   uart.disp(bytes[0], 16);
//	   uart.disp("\n\r");
}

