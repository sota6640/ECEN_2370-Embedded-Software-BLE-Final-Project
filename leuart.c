/**
 * @file leuart.c
 * @author Sonal Tamrakar
 * @date  11/9/2021
 * @brief Contains/handles all the functions of the LEUART peripheral
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Library includes
#include <string.h>

//** Silicon Labs include files
#include "em_gpio.h"
#include "em_cmu.h"

//** Developer/user include files
#include "leuart.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************
uint32_t	rx_done_evt;
uint32_t	tx_done_evt;
bool		leuart0_tx_busy;
static LEUART0_STATE_MACHINE leuart0_state_machine_vals;
static LEUART0_STATE_MACHINE_READ leuart0_read_vals;
//static char unused_data_string[50];
//static uint32_t unused_data_counter = 0;


/***************************************************************************//**
 * @brief LEUART driver
 * @details
 *  This module contains all the functions to support the driver's state
 *  machine to transmit a string of data across the LEUART bus.  There are
 *  additional functions to support the Test Driven Development test that
 *  is used to validate the basic set up of the LEUART peripheral.  The
 *  TDD test for this class assumes that the LEUART is connected to the HM-18
 *  BLE module.  These TDD support functions could be used for any TDD test
 *  to validate the correct setup of the LEUART.
 *
 ******************************************************************************/

//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *  This is the LEUART driver function that initializes LEUART0 peripheral
 *
 * @details
 *   The open function sets up the clock for LEUART0. If it can't choose LEUART0, then it asserts false. The LEUART_Init() values are also set in
 *   this function. The function also contains all the NVIC vectors for interrupt purposes. The route location and the route pen are also set. We make sure
 *   that all startframes are all set up in this function.
 *
 * @note
 *    After all the initialization, at the very end, it calls the LEUART_Enable to make sure
 *    the peripheral is enabled.
 *
 * @param[in] *leuart
 *   Pointer to the base peripheral address of the LEUART0 peripheral being opened
 *
 *@param[in] leuart_settings
 *   The values that are coming from the ble.c struct values for the HM10
 ******************************************************************************/

void leuart_open(LEUART_TypeDef *leuart, LEUART_OPEN_STRUCT *leuart_settings){

  if (leuart == LEUART0) {
        CMU_ClockEnable(cmuClock_LEUART0, true);
    }

  leuart->STARTFRAME = true;
  while (leuart->SYNCBUSY);
  EFM_ASSERT(leuart->STARTFRAME & true);
  leuart->STARTFRAME = false;
  while (leuart->SYNCBUSY);
  EFM_ASSERT(~leuart->STARTFRAME & true);

  LEUART_Init_TypeDef leuart_vals_input;
  leuart_vals_input.baudrate = leuart_settings->baudrate;
  leuart_vals_input.databits = leuart_settings->databits;
  leuart_vals_input.enable = leuart_settings->enable;
  leuart_vals_input.parity = leuart_settings->parity;
  leuart_vals_input.refFreq = REF_FREQ_NULL; //not sure about this one, but this is 0
  leuart_vals_input.stopbits = leuart_settings->stopbits;


  LEUART_Init(leuart, &leuart_vals_input);

  leuart->ROUTELOC0 = leuart_settings->tx_loc | leuart_settings->rx_loc ; //either has to be route or route1
  leuart->ROUTEPEN = (leuart_settings->tx_en * LEUART_ROUTEPEN_TXPEN ) | (leuart_settings->rx_en * LEUART_ROUTEPEN_RXPEN);


   NVIC_EnableIRQ(LEUART0_IRQn);


  leuart0_read_vals.leuart_state_read = LEUART0;


  leuart->STARTFRAME = leuart_settings->startframe;
  leuart->SIGFRAME = leuart_settings->sigframe;

  leuart->CMD |= LEUART_CMD_RXBLOCKEN;
  leuart-> CTRL |= LEUART_CTRL_SFUBRX;
  leuart->IFC |= _LEUART_IFC_MASK;
  leuart->IEN |= LEUART_IEN_STARTF ;
  LEUART_Enable(leuart, leuartEnable);

  leuart0_read_vals.read_counter = 0;
  leuart0_read_vals.current_state_read = INIT_READ;

  leuart_rx_tdd();

}

/***************************************************************************//**
 * @brief
 *  The leuart_txbel function deals with the TXBL interrupts the master gets from the slave.
 *
 * @details
 *   In order to send data, we need to make sure that the data_string_length is greater than zero.
 *   If the data_string_length == 0, then it means there is no more data to be transferred. When the length of the string
 *   is greater than 0, we constantly send once character at a time and update the characters that are left to be sent. We do this
 *   by implementing a counter. The TRANSMISSiON_STATE handles the case in sending over the data while the STOP_STATE state
 *   deals with stopping the communication between the master and the slave.
 *
 * @note
 *
 *
 * @param[in] *sm
 *   Pointer to the STATE_MACHINE_STRUCT that was created in leuart.h
 ******************************************************************************/

void leuart_txbel(LEUART0_STATE_MACHINE *sm) {
  switch (sm->current_state) {
    case TRANSMISSION_STATE:
      {
        if (sm->data_string_length > 0) {
            sm->leuart_state->TXDATA = sm->data_string[sm->count_char];
            sm->count_char = sm->count_char + 1;
            sm->data_string_length = sm->data_string_length - 1;
            sm->current_state = TRANSMISSION_STATE; //stay in this state until the data_string_length is 0;
        }
        else if (sm->data_string_length == NO_DATA)  { //data_string_length == 0 , there is no data to be sent, the process is done
            LEUART_IntDisable(sm->leuart_state, LEUART_IEN_TXBL);
            //LEUART_IntEnable(sm->leuart_state, LEUART_IEN_TXC);
            //sm->leuart_state->IEN &= ~LEUART_IEN_TXBL; // clearing
            sm->leuart_state->IEN |= LEUART_IEN_TXC; //enabling
            sm->current_state = STOP_STATE;
            sm->leuart_state->IEN |= LEUART_IEN_TXC; //can this just be addded in the else if statement?
            LEUART_IntDisable(sm->leuart_state, LEUART_IEN_TXBL);
            break;
        }
        break;
      }
    case STOP_STATE:
      {
        break;
      }
    default:
      EFM_ASSERT(false);
      break;
  }
}

/***************************************************************************//**
 * @brief
 *  The leuart_txc function deals with the TXC interrupts the master gets from the slave.
 *
 * @details
 *  In this function, we are basically stopping all of the processes. We disabled the TXC interrupt and
 *  call the add_scheduled_event for the transmit feedback.
 *
 * @note
 *
 *
 * @param[in] *sm
 *   Pointer to the STATE_MACHINE_STRUCT that was created in leuart.h
 ******************************************************************************/
void leuart_txc(LEUART0_STATE_MACHINE *sm) {
  switch (sm->current_state) {

    case TRANSMISSION_STATE:
          break;
    case STOP_STATE:
      {
        sleep_unblock_mode(LEUART_TX_EM);
        sm->not_available = false;

        add_scheduled_event(LEUART_TX_CBB);
         // sm->leuart_state->IEN &= ~LEUART_IEN_TXC & ~LEUART_IEN_TXBL;
        //either disble or clear them not sure
        LEUART_IntDisable(sm->leuart_state, LEUART_IEN_TXC);
        //sm->current_state = INITIALIZATION_STATE;
        break;
      }

    //case INITIALIZATION_STATE:
     // { EFM_ASSERT(false);
      //   break;   }
    //case INITIALIZATION_STATE:
   // case TRANSMISSION_STATE:
    default:
      EFM_ASSERT(false);
      break;
    }
}

/***************************************************************************//**
 * @brief
 *  The function leuart_startframe(LEUART0_STATE_MACHINE_READ *sm_read) does the handling
 *  after the STARTFRAME interrupt is triggered.
 *
 * @details
 *  In this function, we are in the enum INIT_READ, where the SIGF and the RXDATAV interrupts are
 *  enabled. We also set the read_counter to 0 to make sure that we don't iterate at the wrong location.
 *  The counter makes sure that this is the beginning of data. We then change the current_state_read
 *  to RECEIVE_DATA. The LEUART_CMD_BLOCKDIS command clears the RXBLOCK,  and all incoming
 *  data can be loaded into the receive buffer.
 *
 * @note
 *
 *
 * @param[in] *sm
 *   Pointer to the STATE_MACHINE_STRUCT_READ that was created in leuart.h. We need to be able to enable interrupts
 *   and write commands by accessing the state machine.
 ******************************************************************************/
void leuart_startframe(LEUART0_STATE_MACHINE_READ *sm_read) {
  switch(sm_read->current_state_read) {
    case INIT_READ:
      {
        sm_read->current_state_read = RECEIVE_DATA;
        sm_read->leuart_state_read->IEN |= LEUART_IEN_SIGF;
       // sm_read->leuart_state_read->IEN |= LEUART_IEN_STARTF ; //pretty sure this is done in leuart_open
        sm_read->leuart_state_read->CMD |= LEUART_CMD_RXBLOCKDIS; //receiver block disbale, meaning receive buffer can be populated
        sm_read->leuart_state_read->IEN |= LEUART_IEN_RXDATAV;

        sm_read->read_counter = 0;

      break;
      }
    default:
      EFM_ASSERT(false);
      break;
  }
}



/***************************************************************************//**
 * @brief
 * In the function leuart_rxdatav(LEUART0_STATE_MACHINE_READ *sm_read), it does all the handling
 * once the RXDATAV interrupt is triggered.
 *
 * @details
 *  Because we setup our counter as zero in the STARTFRAME interrupt handler, now we can start receiving
 *  data and increment the counter.We read data from the RXDATA register and put it into the data_string_rx
 *  array that was created in the LEAURT0_STATE_MACHINE_READ. We increment the counter as we go.
 *
 * @note
 *
 *
 * @param[in] *sm
 *   Pointer to the STATE_MACHINE_STRUCT_READ that was created in leuart.h. We need to be able to enable interrupts
 *   and write commands by accessing the state machine.
 ******************************************************************************/
void leuart_rxdatav(LEUART0_STATE_MACHINE_READ *sm_read) {
  switch(sm_read->current_state_read) {
    case RECEIVE_DATA:
      {
          sm_read->data_string_rx[sm_read->read_counter] = sm_read->leuart_state_read->RXDATA;
          sm_read->read_counter = sm_read->read_counter + 1;
         break;
      }
    default:
          EFM_ASSERT(false);
          break;

      }
}


/***************************************************************************//**
 * @brief
 *  In the function leuart_sigframe(LEUART0_STATE_MACHINE_READ *sm_read), it does all of the handling once
 *  the SIGFRAME interrupt is triggered.
 *
 * @details
 *  Enables the LEUART_CMD_RXBLOCKEN command into the command register so that it doesn't allow any
 *  more data to get into RXDATA. This has to be manually done. We also need to schedule an event so the command
 *  can be evaluated and parsed. Along with that, we need to clear out the data_string_rx array, and await for
 *  a STARTFRAME.
 * @note
 *
 *
 * @param[in] *sm
 *   Pointer to the STATE_MACHINE_STRUCT_READ that was created in leuart.h. We need to be able to enable interrupts
 *   and write commands by accessing the state machine.
 ******************************************************************************/
void leuart_sigframe(LEUART0_STATE_MACHINE_READ *sm_read) {
  switch(sm_read->current_state_read) {
    case RECEIVE_DATA:
      {
        sm_read->leuart_state_read->CMD |= LEUART_CMD_RXBLOCKEN;


        while(sm_read->leuart_state_read->SYNCBUSY);

        sm_read->data_string_rx[sm_read->read_counter] = 0;
        sm_read->read_counter = sm_read->read_counter + 1;

        sm_read->current_state_read = INIT_READ;
        add_scheduled_event(0x40);
        break;
      }
    default:
      EFM_ASSERT(false);
      break;

  }
}


/***************************************************************************//**
 * @brief
 * The void LEUART0_IRQHandler function sets/develops the Interrupt Service Routine for LEUART0
 *
 * @details
 * if statements to check which interrupt it was and calls the appropriate state machines functions.
 * If it's the TXBL interrupt, it goes to the leuart_txbel(). If it's the TXC interrupt, it goes to the
 * leuart_txc(). If the STARTF interrupt is triggered, it goes into the leuart_startframe() interrupt handler
 * utilizing the LEUART0_STATE_MACHINE_READ private struct. Same if a SIGF interrupt is triggerged, but into leuart_sigframe()
 * interrupt handler. Same if RXDATAV interrupt is triggered, but into leuart_rxdatav() interrupt handler
 *
 * @note
 *
 ******************************************************************************/

void LEUART0_IRQHandler(void){
  uint32_t int_flag = LEUART0->IF & LEUART0->IEN;
    LEUART0->IFC = int_flag;
    if (int_flag & LEUART_IF_TXBL)
      {
        leuart_txbel(&leuart0_state_machine_vals);
      }


    if (int_flag & LEUART_IF_TXC)
      {

        leuart_txc(&leuart0_state_machine_vals);
    }



    if (int_flag & LEUART_IF_STARTF)
      {
        leuart_startframe(&leuart0_read_vals);
      }

    if (int_flag & LEUART_IF_RXDATAV)
      {
         leuart_rxdatav(&leuart0_read_vals);
      }

    if (int_flag & LEUART_IF_SIGF)
      {
        leuart_sigframe(&leuart0_read_vals);
      }





}


/***************************************************************************//**
 * @brief
 *  Initializes a private STRUCT that will keep track of what operation LEUART is in. Initiates the transfer of data
 *  via bluetooth by initializing everything
 *
 * @details
 *   Need to make sure that out code is atomic so that any other external interrupts won't occur.
 *   Initializing all the state machine variables before heading into the IRQ_Handlers.
 *
 * @note
 *
 *
 * @param[in] *leuart
 * Pointer to the base peripheral address of the LEUART0 peripheral being opened. Starts
 * communicating by going into the TRANSMISSION_STATE of the IRQ handlers.
 *
 *@param[in] *string
 *  The char pointer string to the information(data) the master is sending to the slave.
 * @param[in] string_len
 *  The length of the string, named string_len
 ******************************************************************************/

void leuart_start(LEUART_TypeDef *leuart, char *string, uint32_t string_len){
  static LEUART0_STATE_MACHINE *sm;



  if (leuart == LEUART0) {
      sm = &leuart0_state_machine_vals;
  }

  else {
      EFM_ASSERT(false);
  }

  while(sm->not_available);
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();





  sleep_block_mode(LEUART_TX_EM); //page 25, point v.
  sm->not_available = true;
  sm->leuart_state = leuart;
  sm->data_string_length = string_len;
  sm->count_char = NO_DATA;
  strcpy(sm->data_string, string);
  sm->cb_tx = LEUART_TX_CBB;
  sm->current_state = TRANSMISSION_STATE;
  sm->leuart_state->IEN &= ~LEUART_IEN_TXC;
  sm->leuart_state->IEN |= LEUART_IEN_TXBL; //start communication

  CORE_EXIT_CRITICAL();


}




/***************************************************************************//**
 * @brief
 *   LEUART STATUS function returns the STATUS of the peripheral for the
 *   TDD test
 *
 * @details
 * 	 This function enables the LEUART STATUS register to be provided to
 * 	 a function outside this .c module.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the STATUS register value as an uint32_t value
 *
 ******************************************************************************/

uint32_t leuart_status(LEUART_TypeDef *leuart){
	uint32_t	status_reg;
	status_reg = leuart->STATUS;
	return status_reg;
}

/***************************************************************************//**
 * @brief
 *   LEUART CMD Write sends a command to the CMD register
 *
 * @details
 * 	 This function is used by the TDD test function to program the LEUART
 * 	 for the TDD tests.
 *
 * @note
 *   Before exiting this function to update  the CMD register, it must
 *   perform a SYNCBUSY while loop to ensure that the CMD has by synchronized
 *   to the lower frequency LEUART domain.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] cmd_update
 * 	 The value to write into the CMD register
 *
 ******************************************************************************/

void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update){

	leuart->CMD = cmd_update;
	while(leuart->SYNCBUSY);
}

/***************************************************************************//**
 * @brief
 *   LEUART IF Reset resets all interrupt flag bits that can be cleared
 *   through the Interrupt Flag Clear register
 *
 * @details
 * 	 This function is used by the TDD test program to clear interrupts before
 * 	 the TDD tests and to reset the LEUART interrupts before the TDD
 * 	 exits
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 ******************************************************************************/

void leuart_if_reset(LEUART_TypeDef *leuart){
	leuart->IFC = 0xffffffff;
}

/***************************************************************************//**
 * @brief
 *   LEUART App Transmit Byte transmits a byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a transmit byte, a while statement checking for the TXBL
 *   bit in the Interrupt Flag register is required before writing the
 *   TXDATA register.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] data_out
 *   Byte to be transmitted by the LEUART peripheral
 *
 ******************************************************************************/

void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out){
	while (!(leuart->IF & LEUART_IF_TXBL));
	leuart->TXDATA = data_out;
}


/***************************************************************************//**
 * @brief
 *   LEUART App Receive Byte polls a receive byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a receive byte, a while statement checking for the RXDATAV
 *   bit in the Interrupt Flag register is required before reading the
 *   RXDATA register.
 *
 * @param[in] leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the byte read from the LEUART peripheral
 *
 ******************************************************************************/

uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart){
	uint8_t leuart_data;
	while (!(leuart->IF & LEUART_IF_RXDATAV));
	leuart_data = leuart->RXDATA;
	return leuart_data;
}

/***************************************************************************//**
 * @brief
 * The leuart_tx_busy() function returns the status of the LEUART0, and checks it its
 * busy or not.
 *
 * @details
 *
 * @param[out] leuart0_state_machine_vals.not_available;
 * Returns a bool variable that shows the status of the LEAURT0 peripheral.
 *
 *
 *
 ******************************************************************************/
bool leuart_tx_busy() {
  return leuart0_state_machine_vals.not_available;
}


/***************************************************************************//**
 * @brief
 * The leuart_rx_tdd(void) checks to validate that the LEUART RX function has been properly
 * set up in our leuart_open (driver) function. This function serves as the "planning" for receiving
 * data from the LEUART peripheral. This function also makes sure that our state machine is functioning
 * properly.
 *
 * @details
 *  To test out our LEUART RX function, we use loopback mode to ensure that we have a reception of data
 *  and can check what is being transmitted. We run this function after we initialize our leuart_open function,
 *  then call this at the end. In this function, we test whether the peripheral blocks a non-start
 *  frame character and will accept a start frame character. This follows with whatever data is coming in
 *  and tests whether our data is completed being received which is through a sigframe interrupt.
 *
 * @note
 * Whatever data we transmit, the function should isolate whatever is after the startframe and before the
 * sigframe and is recepted back through loopback.
 *
 ******************************************************************************/
void leuart_rx_tdd(void) {


  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  // submission video canvas walkthrough start
  uint32_t save_ien_reg;
  save_ien_reg = LEUART0->IEN;
  LEUART0->IEN = false; //mentioned in the submission canvas video but gets stuck on while(LEUART0->SYNCBUSY)
  //disabling interrupts;
  LEUART0->CTRL |= LEUART_CTRL_LOOPBK; //put it in LOOPBACK mode through control register.
  //LEUART0->IFC = LEUART0->RXDATA;
  while(LEUART0->SYNCBUSY);
  // submission video canvas walkthrough stop

  uint32_t local_startframe = LEUART0->STARTFRAME;
  uint32_t local_sigframe = LEUART0->SIGFRAME;
 //LECTURE video walthrough continued
  LEUART0->TXDATA = ~local_startframe;
  timer_delay(2); //timer delay of 2 milliseconds
  EFM_ASSERT(!(LEUART0->IF & LEUART_IF_RXDATAV));
  LEUART0->TXDATA = local_startframe;
  timer_delay(2); //timer delay of 2 milliseconds
  EFM_ASSERT(LEUART0->IF & LEUART_IF_RXDATAV);
  EFM_ASSERT(local_startframe == LEUART0->RXDATA);
  LEUART0->TXDATA = local_sigframe;
  timer_delay(2); //timer delay of 2 milliseconds
  EFM_ASSERT(LEUART0->IF & LEUART0->SIGFRAME);
  EFM_ASSERT(local_sigframe == LEUART0->RXDATA);
  //monday lecture vid

  LEUART0->CMD |= LEUART_CMD_RXBLOCKEN;
  LEUART0->IFC |= LEUART_IFC_STARTF | LEUART_IFC_SIGF;
  //clear interrupts
  LEUART0->IEN = save_ien_reg; //reenable the interrupts
  while(LEUART0->SYNCBUSY);

  CORE_EXIT_CRITICAL(); //not sure where this goes, might be here or at the very end


  //SLACK WALKTHROUGH ON TESTING OUT THE STRING
  char test_str[] = "hello";
  uint32_t length;
  char tx_str[50], expected_result[50];
  tx_str[0] = 0;
  strcat(tx_str, "abc");
  length = strlen(tx_str);
  tx_str[length] = LEUART0->STARTFRAME;
  tx_str[length+1] = 0;
  strcat(tx_str, test_str);
  length = strlen(tx_str);
  tx_str[length] = LEUART0->SIGFRAME;
  tx_str[length+1] = 0;
  strcat(tx_str, "def");
  expected_result[0] = LEUART0->STARTFRAME;
  expected_result[1] = 0;
  strcat(expected_result, test_str);
  length = strlen(expected_result);
  expected_result[length] = LEUART0->SIGFRAME;
  expected_result[length+1] = 0;
  leuart_start(LEUART0, tx_str, strlen(tx_str));
  while(leuart_tx_busy(LEUART0));
  timer_delay(50);
  char in_test_string[50];


  strcpy(in_test_string,leuart0_read_vals.data_string_rx);


  EFM_ASSERT(!strcmp(in_test_string, expected_result));





  //EFM_ASSERT(LEUART0->STATUS & LEUART_STATUS_RXBLOCK); //"check whether RXBLOCKEN = TRUE in the status register
  EFM_ASSERT(LEUART0->STATUS & LEUART_STATUS_RXENS);
  LEUART0->CTRL &= ~LEUART_CTRL_LOOPBK; //disabling loopback
  while(LEUART0->SYNCBUSY);
  //remove_scheduled_event(LEUART_RX_CBB); //we don't need this


}


/***************************************************************************//**
 * @brief
 *  The return_read_val (char * ret_read) reads from the private LEUART0_STATE_MACHINE_STRUCT defined at the
 *  top of the file into a variable ret_read.
 *
 * @details
 *  The function uses strcpy to put the data_string_rx from the read state machine into ret_read. The
 *  purpose of strcpy is to copy a string pointed by a source into some array pointed by the destination.
 *  In this case, the destination is in the callback function in app.c, which is the scheduled_rx_cb(void) function.
 *
 * @note
 *
 *
 * @param[in] *ret_read
 *  Pointer that points to the destination and copies the data there using strcpy.
 ******************************************************************************/
void return_read_val (char * ret_read) {

  strcpy(ret_read, leuart0_read_vals.data_string_rx);

}
