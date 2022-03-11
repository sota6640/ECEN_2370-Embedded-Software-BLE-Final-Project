/**
 * @file i2c.c
 * @author Sonal Tamrakar
 * @date 10/17/2021
 * @brief I2C configuration and enabling
 */

#include "i2c.h"

//***********************************************************************************
// Private/Static functions
//***********************************************************************************
static void i2c_bus_reset(I2C_TypeDef *i2c);
static void i2c_ack_sm(I2C_STATE_MACHINE *i2c_sm);
static void i2c_msstop_sm(I2C_STATE_MACHINE *i2c_sm);
static void i2c_receive_sm(I2C_STATE_MACHINE *i2c_sm);

//***********************************************************************************
// Private/Static Variables
//***********************************************************************************
static I2C_STATE_MACHINE i2c_state_machine_vals_I2C0;
static I2C_STATE_MACHINE i2c_state_machine_vals_I2C1;

//static I2C_STATE_MACHINE *sm;


/***************************************************************************//**
 * @brief
 *  Routine to reset the I2C State machines of the mighty gecko, and also to reset the I2C state machines of external devices
 *
 * @details
 *   This functions does a BUS reset which can be done by adjusting the Start and Stop commands.We also need to make sure that all
 *   interrupts have been disabled using the IEN and IF register.
 *
 * @note
 *
 *
 * @param[in] i2c
 *   Pointer to the base peripheral address of the I2C peripheral being opened
 *
 ******************************************************************************/
void i2c_bus_reset(I2C_TypeDef *i2c) {
  uint32_t ien_save_state;
  i2c ->CMD = I2C_CMD_ABORT;
  ien_save_state = i2c->IEN;
  i2c->IEN = false;
  i2c->IFC = i2c->IF;
  i2c -> CMD = I2C_CMD_CLEARTX;
  i2c->CMD = I2C_CMD_START | I2C_CMD_STOP;
  while(!(i2c->IF & I2C_IF_MSTOP));
  i2c->IFC = i2c->IF;
  i2c ->CMD = I2C_CMD_ABORT;
  i2c ->IEN = ien_save_state;
}

/***************************************************************************//**
 * @brief
 *  This is the I2C driver function that initializes I2C0 or I2C1
 *
 * @details
 *   The open function sets up the clock for both I2C0 and I2C1, depending on which one is chosen. The I2C_Init() values are also set in
 *   this function. The function also contains all the NVIC vectors for interrupt purposes. The route location and the route pen is set.
 *
 * @note
 *    This function calls the i2c_bus_reset() with the i2c TypeDef all initialized.
 *
 * @param[in] i2c
 *   Pointer to the base peripheral address of the I2C peripheral being opened
 *
 *@param[in] i2c_setup
 *   The values that are coming from the Si1133.c struct values
 ******************************************************************************/
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup) {

  if (i2c == I2C0) {
      CMU_ClockEnable(cmuClock_I2C0, true);
  }

  if (i2c == I2C1) {
      CMU_ClockEnable(cmuClock_I2C1, true);
  }


  if ((i2c->IF & 0x01) == 0) {

      i2c->IFS = 0x01;
      EFM_ASSERT(i2c->IF & 0x01);
      i2c->IFC = 0x01;
      }
  else {
      i2c->IFC = 0x01;
      EFM_ASSERT(!(i2c->IF & 0x01));
    }

  I2C_Init_TypeDef i2c_values;

  i2c_values.enable = i2c_setup->enable;
  i2c_values.master = i2c_setup->master;
  i2c_values.refFreq = i2c_setup->refFreq;
  i2c_values.freq = i2c_setup->freq;
  i2c_values.clhr = i2c_setup->clhr;

  I2C_Init(i2c, &i2c_values);

  i2c->ROUTELOC0 = i2c_setup->sda_route | i2c_setup->scl_route ; //either has to be route or route1
  i2c->ROUTEPEN = (i2c_setup->sda_enable * I2C_ROUTEPEN_SDAPEN) | (i2c_setup -> scl_enable * I2C_ROUTEPEN_SCLPEN);

  //interrupts
  i2c->IFC |= _I2C_IFC_MASK;
  i2c->IEN |= I2C_IEN_ACK | I2C_IEN_NACK | I2C_IEN_RXDATAV | I2C_IEN_MSTOP;

  //nvic enable
  if (i2c == I2C1) { NVIC_EnableIRQ(I2C1_IRQn);}
  else if (i2c == I2C0) {NVIC_EnableIRQ(I2C0_IRQn);}

  i2c_bus_reset(i2c);

}


/***************************************************************************//**
 * @brief
 *  The i2c_ack_sm function deals with all the ACK interrupts the master gets from the slave.
 *
 * @details
 *   A counter is setup so that after the desired number of ACKs have been received, it goes to the next state.
 *   In this state machine, the master first transmits the dev_address (with bit shifting) to the slave. After we have
 *   an ACK from the slave, the counter increases by 1. Then the register_address is transmitted to the slave, after which we get
 *   another ACK, and the counter increases by 1. Then the master(gecko) has to send the start command again, which
 *   is the dev_address but this time OR'ed with the the READ_OP to indicate that the master is ready to start receiving data.
 *   All of the mentioned statements are happening in one state.All the other cases are asserted to false so that it never enters those.
 *   Once counter_var reaches 3, meaning we have reached the desired number of ACKs, then it will go to the next state, which is the
 *   read_data case. In this function we are also checking when the read_write_op equals 0 in case we are dealing with a write_data and in this case
 *   we will move to the write_data case where we are writing data to the Si1133.
 *
 * @note
 *  For all three state machines, there exists a default case where there is an EFM_ASSERT set to false for debugging purposes, if anything goes wrong in the state machine above.
 *
 ******************************************************************************/
uint32_t left_shift_val = 0;

void i2c_ack_sm(I2C_STATE_MACHINE *i2c_sm){
  switch(i2c_sm->curr_state) {
    case init_process:
      if (i2c_sm->counter_var == 0){
          i2c_sm->i2c_state->TXDATA = (i2c_sm->dev_address) << 1 | WRITE_OP;
          i2c_sm->counter_var += 1;
          i2c_sm->curr_state = init_process;
      }
      else if (i2c_sm->counter_var == 1) {
          i2c_sm->i2c_state->TXDATA = i2c_sm->register_address;
          i2c_sm->counter_var += 1;

          i2c_sm->curr_state = init_process;
          if (i2c_sm->read_write_op == 0){

              i2c_sm->curr_state = write_data;
              i2c_sm->i2c_state->TXDATA = (*(i2c_sm->num_data)) << (left_shift_val*8);
              i2c_sm->number_bytes -= 1;
          }
      }
      else if (i2c_sm->counter_var == 2) {
          i2c_sm->i2c_state->CMD = I2C_CMD_START;
          i2c_sm->i2c_state->TXDATA = (i2c_sm->dev_address) << 1 | READ_OP; //getting ready to read from Si1133
          i2c_sm->counter_var += 1;
          i2c_sm->curr_state = init_process;
      }
      else if (i2c_sm->counter_var == 3) {
          i2c_sm->curr_state = read_data;
          //i2c_sm->curr_state =
      }
      break;
    case read_data:
    case stop_data:
     // i2c_sm->i2c_state->CMD = I2C_CMD_STOP;
      break;
    case write_data:

      if(i2c_sm->number_bytes > 0)
              {
                i2c_sm->i2c_state->TXDATA = (*(i2c_sm->num_data)) << (left_shift_val*8);
                left_shift_val +=1;
                i2c_sm->number_bytes -= 1;
                i2c_sm->curr_state = write_data;
              }
            else if (i2c_sm-> number_bytes == 0)
              {
                i2c_sm->i2c_state->CMD = I2C_CMD_STOP;
                i2c_sm->curr_state = stop_data;
              }
      break;
    default:
      EFM_ASSERT(false);
     break;
  }
}

/***************************************************************************//**
 * @brief
 *  The i2c_receive_sm function deals with all the RXDATAV interrupts the master gets from the slave.
 *
 * @details
 *   In this state machine, we are receiving data from the Si1133 sensor. It is also incorporated with a counter. This is the read_data case.
 *   After the master has received the third ACK from the Si1133, then it can start sending the the data. It is running on a receive_counter and will
 *   increment once the needed bytes have been read. number_bytes is being used
 *   to keep track of how many more bytes is there to be read/ If there is no more data to be read, the NACK will be sent from the master to slave followed by a STOP
 *   command.It will then change the current state to stop_data.
 *
 *
 * @note
 *  For all three state machines, there exists a default case where there is an EFM_ASSERT set to false for debugging purposes, if anything goes wrong in the state machine above.
 *
 ******************************************************************************/

void i2c_receive_sm(I2C_STATE_MACHINE *i2c_sm) {
  switch(i2c_sm->curr_state) {
    case read_data:
      {

        i2c_sm->number_bytes = i2c_sm->number_bytes - 1;
        *(i2c_sm->num_data) &= ~(0xff << (BIT_SHIFT_EIGHT*i2c_sm->number_bytes));
        *(i2c_sm->num_data) |= i2c_sm->i2c_state->RXDATA << (BIT_SHIFT_EIGHT*i2c_sm->number_bytes); //two byte functionality
      //  *(i2c_sm->num_data) = i2c_sm -> i2c_state->RXDATA >> (BIT_SHIFT_EIGHT * (i2c_sm->number_bytes -1)); //one byte functionality
       //  i2c_sm->number_bytes = i2c_sm->number_bytes - 1;



        if(i2c_sm->number_bytes == 0)
                {
                  i2c_sm->i2c_state->CMD = I2C_CMD_NACK;
                  i2c_sm->i2c_state->CMD = I2C_CMD_STOP;
                  i2c_sm->curr_state = stop_data;
                }
        else
          {
              i2c_sm->i2c_state->CMD = I2C_CMD_ACK;
              i2c_sm->curr_state = read_data;
          }

              break;
      }
    case init_process:
    case stop_data:
    case write_data:
      default:
        EFM_ASSERT(false);
        break;
}
}

/***************************************************************************//**
 * @brief
 *  The i2c_msstop_sm function is the state machine where all the processes ends, deals with the MSSTOP interrupts
 *
 * @details
 *   In this state machine, the Si1133 has sent all the data we needed and the master sends a NACK
 *   back to the Si1133 to signal the end of transmission. Then the master sends a STOP command which returns back to
 *   the master which is the MSTOP interrupt. It will unblock the energy mode EM2. All the other cases are set to default so they are not used.
 *
 * @note
 *  For all three state machines, there exists a default case where there is an EFM_ASSERT set to false for debugging purposes.
 *
 ******************************************************************************/
void i2c_msstop_sm(I2C_STATE_MACHINE *i2c_sm) {
  switch(i2c_sm->curr_state) {
    case stop_data:
      sleep_unblock_mode(I2C_EM_BLOCK);
       i2c_sm->not_available = false;
       add_scheduled_event(i2c_sm->callback_i2c);
       break;
    case init_process:
    case read_data:
    case write_data:
        default:
          EFM_ASSERT(false);
          break;
       }
  }

/***************************************************************************//**
 * @brief
 * The void I2C0_IRQHandler function sets/develops the Interrupt Service Routine for I2C0
 *
 * @details
 * if statements to check which interrupt it was and calls the appropriate state machines functions.
 * If it's the ACK interrupt, it goes to the i2c_ack_sm(). If it's the RXDATA interrupt, it goes to the
 * i2c_receive_sm(). If it's the MSTOP interrupt, it goes to the i2c_msstop_sm().
 *
 * @note
 *
 ******************************************************************************/

void I2C0_IRQHandler(void) {

  uint32_t int_flag = I2C0->IF & I2C0->IEN;
  I2C0->IFC = int_flag;
  if (int_flag & I2C_IF_ACK)
    {
      //need to include the parameter for the I2C0 state machine
      i2c_ack_sm(&i2c_state_machine_vals_I2C0);
    }
  //Don't need this for NACK

  if (int_flag & I2C_IF_RXDATAV)
    {
      //need to include the parameter for the I2C0 state machine
      i2c_receive_sm(&i2c_state_machine_vals_I2C0);
  }

  if (int_flag & I2C_IF_MSTOP)
    {
      //need to include the parameter for the I2C0 state machine
      i2c_msstop_sm(&i2c_state_machine_vals_I2C0);
    }

}

/***************************************************************************//**
 * @brief
 * The void I2C1_IRQHandler function sets/develops the Interrupt Service Routine for I2C1
 *
 * @details
 * if statements to check which interrupt it was and calls the appropritate state machines functions.
 * If it's the ACK interrupt, it goes to the i2c_ack_sm(). If it's the RXDATA interrupt, it goes to the
 * i2c_receive_sm(). If it's the MSTOP interrupt, it goes to the i2c_msstop_sm().
 *
 * @note
 *
 ******************************************************************************/

void I2C1_IRQHandler(void){

  uint32_t int_flag = I2C1->IF & I2C1->IEN;
    I2C1->IFC = int_flag;
    if (int_flag & I2C_IF_ACK)
      {
        //need to include the parameter for the I2C1 state machine
              i2c_ack_sm(&i2c_state_machine_vals_I2C1);
      }

    //Don't need one for NACK

    if (int_flag & I2C_IF_RXDATAV)
      {
        //need to include the parameter for the I2C1 state machine
              i2c_receive_sm(&i2c_state_machine_vals_I2C1);
    }

    if (int_flag & I2C_IF_MSTOP)
      {
        //need to include the parameter for the I2C1 state machine
                      i2c_msstop_sm(&i2c_state_machine_vals_I2C1);
                      //i2c_receive_sm(&i2c_state_machine_vals_I2C1);
      }

}

/***************************************************************************//**
 * @brief
 *  Initializes a private STRUCT that will keep track of what operation I2C is in. Initiates the transfer of data by initializing everything
 *
 * @details
 *   Need to make sure what peripheral we are choosing by including the if statements. The input arguments to this function will initialize the
 *   local struct
 *
 * @note
 *
 *
 * @param[in] i2c
 * Pointer to the base peripheral address of the I2C peripheral being opened
 *
 *@param[in] device_add
 *  The device address that will be passed onto the STRUCT (slave address)
 * @param[in] register_add
 *  Register address that's being passed onto our STRUCT
 *  @param[in] read_write
 *  The operation, READ or WRITE, Read Operation is 1, Write Operation is 0
 *  @param[in] callback
 *  calback that will be passed onto our struct
 *  @param[in] data
 *  The data being received
 *  @param[in]  number_bytes
 *  The number of bytes we get from the slave.
 ******************************************************************************/

void i2c_start(I2C_TypeDef *i2c, uint32_t device_add, uint32_t register_add, uint32_t read_write, uint32_t callback, uint32_t *data, uint32_t number_bytes)
{
  static I2C_STATE_MACHINE *sm;
  if(i2c == I2C0) {
      sm = &i2c_state_machine_vals_I2C0;
  }
  else if (i2c == I2C1) {
      sm = &i2c_state_machine_vals_I2C1;
  }
  else {
      EFM_ASSERT(false);
  }
  while(sm->not_available);
  EFM_ASSERT((sm->curr_state & _I2C_STATE_STATE_MASK) == I2C_STATE_STATE_IDLE); // X = the I2C peripheral #
  sleep_block_mode(I2C_EM_BLOCK);

  sm->i2c_state = i2c;
  sm->not_available = true; //should be true in block mode, busy is true here
  sm->curr_state = init_process;
  sm->dev_address = device_add;
  sm->register_address = register_add;
  sm->read_write_op = read_write;
  sm->num_data = data;
  sm->number_bytes = number_bytes;
  sm->callback_i2c = callback;
  sm->counter_var = 1;
  sm->receive_counter = 0;
  i2c->CMD = I2C_CMD_START;
  sm->i2c_state->TXDATA = (sm->dev_address) << 1 | WRITE_OP;
}

bool is_busy() {
  return i2c_state_machine_vals_I2C1.not_available;
}




