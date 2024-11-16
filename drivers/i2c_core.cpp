/*****************************************************************//**
 * @file i2c_core.cpp
 *
 * @brief implementation of I2cCore class
 *
 * @author p chu
 * @version v1.0: initial release
 ********************************************************************/

#include "i2c_core.h"

/* methods */
I2cCore::I2cCore(uint32_t core_base_addr) {
   base_addr = core_base_addr;
   set_freq(100000);  // default 100K Hz
}
I2cCore::~I2cCore() {
}                  // not used

void I2cCore::set_freq(int freq) {
   uint32_t dvsr;

   // 25% of i2c period = (1/freq)/4; sys clock period = 1/f_sys
   // dvsr = # sys clocks =  ((1/freq)/4)/(1/f_sys) = f_sys/freq/4
   dvsr = (uint32_t) (SYS_CLK_FREQ * 1000000 / freq / 4);
   io_write(base_addr, DVSR_REG, dvsr);
}

int I2cCore::ready() {
	int ready_flag = ((int)(io_read(base_addr, RD_REG) >> 8) & 0x01);
//	uart.disp("Ready Flag: ");
//	uart.disp(ready_flag);
//	uart.disp("\n\r");
	return ready_flag;
}

void I2cCore::start() {
//	uart.disp("Waiting for I2C Core to be ready in start()\n\r");
	while (!ready()) {
	}
//	uart.disp("I2C Core is ready. Sending START command.\n\r");
	io_write(base_addr, WR_REG, I2C_START_CMD);
}

void I2cCore::restart() {
   while (!ready()) {
   }
   io_write(base_addr, WR_REG, I2C_RESTART_CMD);
}

void I2cCore::stop() {
   while (!ready()) {
   }
   io_write(base_addr, WR_REG, I2C_STOP_CMD);
}

int I2cCore::write_byte(uint8_t data) {
	int ack, acc_data;

	    acc_data = data | I2C_WR_CMD;
//	    uart.disp("Waiting for I2C Core to be ready in write_byte()\n\r");
	    while (!ready()) {
	    }
//	    uart.disp("I2C Core is ready. Sending data: 0x");
//	    uart.disp(data, 16);
//	    uart.disp("\n\r");

	    io_write(base_addr, WR_REG, acc_data);
	    while (!ready()) {
	    }
	    ack = (io_read(base_addr, RD_REG) & 0x0200) >> 9;

//	    uart.disp("Ack Received in write_byte(): ");
//	    uart.disp(ack);
//	    uart.disp("\n\r");

	    if (ack == 0)
	        return (0);
	    else
	        return (-1);
}

//last: last byte in read cycle (0:no; 1:yes)
//      I2C master generate NACK if LSB of last is 1
int I2cCore::read_byte(int last) {
   int acc_data;

   acc_data = last | I2C_RD_CMD;
   while (!ready()) {
   }
   io_write(base_addr, WR_REG, acc_data);
   while (!ready()) {
   }
   return (io_read(base_addr, RD_REG) & 0x00ff);
}


int I2cCore::read_transaction(uint8_t dev, uint8_t *bytes, int num, int rstart) {
   uint8_t dev_byte;
   int ack1;
   int i;

   dev_byte = (dev << 1) | 0x01;   // LSB=1 for I2C read
//   uart.disp("I2C Read Transaction Start\n\r");
//   uart.disp("Device Address (Read): 0x");
//   uart.disp(dev_byte, 16);
//   uart.disp("\n\r");

   start();
   ack1 = write_byte(dev_byte);    // Send device ID for read
//   uart.disp("Device Address Ack: ");
//   uart.disp(ack1);
//   uart.disp("\n\r");

   for (i = 0; i < (num - 1); i++) {
      *bytes = read_byte(0);
//      uart.disp("Read Byte ");
//      uart.disp(i + 1);
//      uart.disp(": 0x");
//      uart.disp(*bytes, 16);
//      uart.disp("\n\r");
      bytes++;
   }

   *bytes = read_byte(1);   // Last byte in read cycle
//   uart.disp("Last Read Byte: 0x");
//   uart.disp(*bytes, 16);
//   uart.disp("\n\r");

   if (rstart == 1) {
//      uart.disp("Restart Condition\n\r");
      restart();
   } else {
//      uart.disp("Stop Condition\n\r");
      stop();
   }

//   uart.disp("Read Transaction Complete. Ack: ");
//   uart.disp(ack1);
//   uart.disp("\n\r");

   return (ack1);
}


int I2cCore::write_transaction(uint8_t dev, uint8_t *bytes, int num, int rstart) {
   uint8_t dev_byte;
   int ack1, ack;
   int i;

   dev_byte = (dev << 1);   // LSB=0 for I2C write
//   uart.disp("I2C Write Transaction Start\n\r");
//   uart.disp("Device Address (Write): 0x");
//   uart.disp(dev_byte, 16);
//   uart.disp("\n\r");

   start();
   ack = write_byte(dev_byte);  // Send device ID for write
//   uart.disp("Device Address Ack: ");
//   uart.disp(ack);
//   uart.disp("\n\r");

   for (i = 0; i < num; i++) {
//      uart.disp("Writing Byte ");
//      uart.disp(i + 1);
//      uart.disp(": 0x");
//      uart.disp(*bytes, 16);
//      uart.disp("\n\r");

      ack1 = write_byte(*bytes);
      ack = ack + ack1;

//      uart.disp("Ack Received: ");
//      uart.disp(ack1);
//      uart.disp("\n\r");

      bytes++;
   }

   if (rstart == 1) {
      restart();
   } else {
//      uart.disp("Stop Condition\n\r");
      stop();
   }

//   uart.disp("Write Transaction Complete. Total Acks: ");
//   uart.disp(ack);
//   uart.disp("\n\r");

   return (ack);
}
/**< Set the tof_ss signal */
void I2cCore::set_tof_ss(int value) {
//    uart.disp("Setting tof_ss to: ");
//    uart.disp(value);
//    uart.disp("\n\r");
    uint32_t wr_data = (value & 0x1) << 31;
    io_write(base_addr, TOF_SS_IRQ_REG, wr_data); // Write 1-bit value to TOF_SS_REG
}
/**< Get the status of the tof_irq signal */
int I2cCore::get_tof_irq() {
    int irq_status = (io_read(base_addr, TOF_SS_IRQ_REG) & 0x1); // Read the LSB from TOF_IRQ register
//    uart.disp("Reading tof_irq: ");
//    uart.disp(irq_status);
//    uart.disp("\n\r");
    return irq_status;
}




