/**
 * @file Si1133.c
 * @author Sonal Tamrakar
 * @date 10/17/2021
 * @brief Si1133 configuration and handling using I2C
 */


#include "Si1133.h"
#include "brd_config.h"



static uint32_t read_data_si1133;
static uint32_t si1133_write_data;
uint32_t si1133_i2c_address = 0x55; //The Si1133 responds to the I2C address of 0x55 (from Si1133 datasheet)

static void si1133_configure();
/***************************************************************************//**
 * @brief
 * Si1133 driver/initialization function
 *
 * @details
 * Initializes all the I2C_OPEN_STRUCT values and sends the values to the i2c_open() function.
 *
 * @note
 *  This function will call the i2c_open, where the input will be the I2C peripheral(I2C0 or I2C1) and the sensor values. This happens in app.c.
 ******************************************************************************/
void si1133_i2c_open() {

  timer_delay(POWER_UP_DELAY);

  I2C_OPEN_STRUCT si_sensor_vals;

  si_sensor_vals.enable = true;
  si_sensor_vals.master = true;
  si_sensor_vals.freq = I2C_FREQ_FAST_MAX;
  si_sensor_vals.refFreq = 0;
  si_sensor_vals.scl_enable = true;
  si_sensor_vals.sda_enable = true;
  si_sensor_vals.scl_route = I2C_SCL;
  si_sensor_vals.sda_route = I2C_SDA;
  si_sensor_vals.clhr = i2cClockHLRAsymetric;
  i2c_open(I2C1, &si_sensor_vals);

  si1133_configure();
}


/***************************************************************************//**
 * @brief
 * The Si1133_read()function calls the i2c_start() function with needed input parameters. Through this function, we are allowing the Si1133 to send values to the master.
 *
 * @details
 * The i2c_start is called with the scheduled event callback,the register address, dereferenced pointer that contains the read data, the specific I2Cx peripheral we want, the number of bytes
 * that will be incoming to the master and either if its a READ or WRITE operation.
 *
 * @note
 * The input parameters in this function are the register address, number of bytes to receive from Si1133 and the scheduled event callback
 *
 * @param[in] reg_addy
 *  I2C Address for Part_ID on Si1133 Datasheet
 *
 * @param[in] number_bytes
 *  Bytes that the slave is sending to the master (Si1133 to the Thunderboard)
 *
 * @param[in] cb
 * Requests the read of of the Si1133 Part ID.
 *
 ******************************************************************************/

void Si1133_read(uint32_t reg_addy,uint32_t number_bytes, uint32_t cb) {

  i2c_start(I2C1, si1133_i2c_address, reg_addy, READ_OP, cb, &read_data_si1133, number_bytes);

}




/***************************************************************************//**
 * @brief
 * The Si1133_write()function calls the i2c_start() function with needed input parameters. Through this function, we are requesting the Si1133 to send values to the master.
 *
 * @details
 * The i2c_start is called with the scheduled event callback,the register address, dereferenced pointer that contains the read data, the specific I2Cx peripheral we want, the number of bytes
 * that will be outgoing to the slave and either if its a READ or WRITE operation. In this case we are using WRITE_OP, which is being used as a write operation.
 *
 * @note
 * The input parameters in this function are the register address, number of bytes to write to the Si1133 and the scheduled event callback
 *
 * @param[in] reg_addy
 *  I2C Register Address the master is willing to write onto the Si1133.
 *
 * @param[in] number_bytes
 *  Number of bytes the master is sending to the Si1133.
 *
 * @param[in] cb
 * Callbacks shouldn't be serviced in the write operation.
 *
 ******************************************************************************/
void Si1133_write(uint32_t reg_addy, uint32_t number_bytes, uint32_t cb) {

  i2c_start(I2C1, si1133_i2c_address, reg_addy, WRITE_OP, cb, &si1133_write_data, number_bytes);

}


void si1133_configure() {
  uint32_t cmd_ctr;
  uint32_t cmd_ctr_new;
  uint32_t cmd_ctr_new2;
  uint32_t cmd_ctr_masked;
  uint32_t cmd_ctr_masked2;
  //Lab Flow start
  Si1133_read(RESPONSE0, NUM_READ, NULL_CB); // read RESPONSE0 reg
  while(is_busy());//1
  cmd_ctr = read_data_si1133 & MASK_BIT;
  si1133_write_data = WHITE_COLOR;
  Si1133_write(INPUT0, NUM_READ, NULL_CB);
  while(is_busy());
  si1133_write_data = PARAMTABLE_WRT | ADCCONFIG0;
  Si1133_write(COMMANDREG, NUM_READ, NULL_CB);
  while(is_busy());
  Si1133_read(RESPONSE0, NUM_READ, NULL_CB);
  while(is_busy());
  cmd_ctr_masked = (cmd_ctr + 1) & MASK_BIT;
  cmd_ctr_new = read_data_si1133 & MASK_BIT;
  if (cmd_ctr_new != cmd_ctr_masked) {
      EFM_ASSERT(false);
  }
  else {
      si1133_write_data = CHANNEL0;
      Si1133_write(INPUT0, NUM_READ, NULL_CB);
      while(is_busy());
      si1133_write_data = PARAMTABLE_WRT | CHAN_LIST;
      Si1133_write(COMMANDREG, NUM_READ, NULL_CB);
      while(is_busy());
      Si1133_read(RESPONSE0, NUM_READ, NULL_CB);
      while(is_busy());
      cmd_ctr_masked2 = (cmd_ctr + 2) & MASK_BIT;
      cmd_ctr_new2 = read_data_si1133 & MASK_BIT;
      if(cmd_ctr_new2 != cmd_ctr_masked2) {
          EFM_ASSERT(false);
      }
      else {
          return;
      }
  }


  }

/***************************************************************************//**
 * @brief
 * The send_si1133_data() function returns the read_data_si1133 to wherever it is called.
 *
 * @details
 * The primary reason for this function is to be used in the scheduled callback function in app.c. The read_data_si1133 should be the Part ID response from our slave.
 *
 * @note
 *  This value will be used in app.c to either turn the LED on or off depending on if the read_data_si1133 is matching the expected data.
 *
 * @param[out] read_data_si1133
 *  read_data_si1133 is being sent out to any function that calls it, in this case the scheduled_si1133_read_cb(void) in app.c is calling this function.
 ******************************************************************************/
uint32_t send_si1133_data() {
  return read_data_si1133;
}

/***************************************************************************//**
 * @brief
 * The force_send() function will write the FORCE command to a static variable then will write that to the command register of the Si1133.
 *
 * @details
 * This function is being used as an initialization to get the Si1133 readings. It initiates a set of measurements within the CHAN_LIST parameter.
 *
 * @note
 *
 ******************************************************************************/
void force_send() {
  si1133_write_data = FORCE_CMD;
  Si1133_write(COMMANDREG, NUM_READ, NULL_CB);
}

/***************************************************************************//**
 * @brief
 * The request_res() function will request the readings from the Si1133 as the FORCE command is sent.
 *
 * @details
 * In this function, we are reading from the HOSTOUT1, which contains the captured sensor data and we are reading one byte at a time, using the
 * I2C_CB appropriate for the I2C peripheral we are using (I2C0 or I2C1).
 *
 * @note
 *
 ******************************************************************************/
void request_res() {
  Si1133_read(HOSTOUT0, NUM_READ_TWO, I2C_CB);
}

