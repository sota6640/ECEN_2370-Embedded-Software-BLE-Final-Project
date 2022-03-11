#ifndef HEADER_FILES_SI1133_H_
#define HEADER_FILES_SI1133_H_

#include "em_i2c.h"
#include "i2c.h"
#include "HW_delay.h"

#define POWER_UP_DELAY 25
#define RESPONSE0 0x11
#define NULL_CB 0x0
#define NUM_READ 1
#define INPUT0  0x0A
#define WHITE_COLOR 0b01011
#define PARAMTABLE_WRT 0b10000000
#define ADCCONFIG0 0x02
#define COMMANDREG 0x0B
#define CHANNEL0 0b00000001
#define CHAN_LIST 0x01
#define FORCE_CMD 0x11
#define HOSTOUT0 0x13
#define HOSTOUT1 0x14
#define NUM_READ_TWO 2
#define I2C_CB 0x00000008
#define MASK_BIT 0x0F





void si1133_i2c_open();
void Si1133_read(uint32_t reg_addy,uint32_t number_bytes, uint32_t cb);
void Si1133_write(uint32_t reg_addy, uint32_t number_bytes, uint32_t cb);
uint32_t send_si1133_data();
void force_send();
void request_res();


#endif /* HEADER_FILES_SI1133_H_ */
