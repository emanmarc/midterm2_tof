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
#include "sseg_core.h"
#include "gpio_cores.h"

void led_check(GpoCore *led_p, int n) {
   int i;

   for (i = 0; i < n; i++) {
      led_p->write(1, i);
      sleep_ms(100);
      led_p->write(0, i);
      sleep_ms(100);
   }
}

void uart_check(PmodToF *tof){
	double distance = tof->getDistance();

	uart.disp("Measured distance: ");
	uart.disp(distance);
	uart.disp(" meters\n\r");
	sleep_ms(500);
}

void sseg_check_with_distance(SsegCore *sseg_p, PmodToF *tof) {

    double distance = tof->getDistance();
    int int_part = static_cast<int>(distance);
    int frac_part = static_cast<int>((distance - int_part) * 100); // Fractional part

    sseg_p->write_1ptn(sseg_p->h2s((int_part / 10) % 10), 3); // Tens
    sseg_p->write_1ptn(sseg_p->h2s(int_part % 10), 2);        // Ones
    sseg_p->write_1ptn(sseg_p->h2s((frac_part / 10) % 10), 1); // Tenths
    sseg_p->write_1ptn(sseg_p->h2s(frac_part % 10), 0);        // Hundredths

    sseg_p->set_dp(0b000100);
    sleep_ms(1000);
}

void pwm_3color_led_with_distance(PwmCore *pwm_p, PmodToF *tof) {
    double distance = tof->getDistance();
    const double MAX_DISTANCE = 2.0; // Maximum distance to map
    double normalized_dist = distance / MAX_DISTANCE;
    if (normalized_dist > 1.0) normalized_dist = 1.0; // Cap to max

    // Map normalized distance to brightness
    double brightness = normalized_dist; // Scale directly

    // Set brightness on all 3 LEDs
    pwm_p->set_duty(brightness, 0); // Red
    pwm_p->set_duty(brightness, 1); // Green
    pwm_p->set_duty(brightness, 2); // Blue

    uart.disp("LED Brightness set to: ");
    uart.disp(brightness * 100); // Display percentage
    uart.disp("%\n\r");

    sleep_ms(500);
}

I2cCore i2c(get_slot_addr(BRIDGE_BASE, S10_I2C));
PmodToF tof(i2c);
SsegCore sseg(get_slot_addr(BRIDGE_BASE, S8_SSEG));
PwmCore pwm(get_slot_addr(BRIDGE_BASE, S6_PWM));
GpoCore led(get_slot_addr(BRIDGE_BASE, S2_LED));

int main() {

//	tof.initialize();

	while (1) {
		uart_check(&tof);
		sseg_check_with_distance(&sseg, &tof);
		pwm_3color_led_with_distance(&pwm, &tof);
	}
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

