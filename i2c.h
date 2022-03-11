#ifndef HEADER_FILES_I2C_H_
#define HEADER_FILES_I2C_H_


#include "em_i2c.h"
#include "em_cmu.h"
#include "sleep_routines.h"
#include "Si1133.h"
#include "scheduler.h"

#define READ_OP 1
#define WRITE_OP 0
#define BIT_SHIFT_EIGHT 8

typedef struct {  //Used by a device such as the Si1133 module to open an i2c peripheral

  bool enable; // Enable I2C peripheral when initialization completed.
  bool master; // Set to master (true) or slave (false) mode.
  uint32_t refFreq; //  I2C reference clock assumed when configuring bus frequency setup.
  uint32_t freq; // (Max) I2C bus frequency to use.
  I2C_ClockHLR_TypeDef clhr; // Clock low/high ratio control.
  bool scl_enable;
  bool sda_enable;
  uint32_t scl_route;
  uint32_t sda_route;

} I2C_OPEN_STRUCT;


typedef enum { //my three states
  init_process,
  read_data,
  stop_data,
  write_data
}DEFINED_STATES;


typedef struct { //Defines the I2C operation and keeps state of the I2C state machine
    I2C_TypeDef *i2c_state; //could be either I2C1 or I2C0

    uint32_t counter_var; //a counter for all the ACK interrupts that will be used in the ACK state machine.
    uint32_t receive_counter; //another counter on the RXDATA state machine.
    DEFINED_STATES curr_state;
    bool not_available; // third set of elements
    uint32_t dev_address; //I2C device address
    uint32_t register_address; //I2C register address being requested
    uint32_t read_write_op; // for read or write, either READ_OP or WRITE_OP
    uint32_t *num_data; //Pointer of where to store a read result or get the write data
    uint32_t number_bytes; //how many bytes to transfer
    uint32_t callback_i2c; // The callback event to request upon completion of the I2C operation


} I2C_STATE_MACHINE; //page 26


void i2c_start(I2C_TypeDef *i2c, uint32_t device_add, uint32_t register_add, uint32_t read_write, uint32_t callback, uint32_t *data, uint32_t number_bytes);
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup);
void I2C0_IRQHandler(void);
void I2C1_IRQHandler(void);
uint32_t send_si1133_data(); //this is for the scheduled callback function that will be used in app.c
bool is_busy();

#endif /* HEADER_FILES_I2C_H_ */
