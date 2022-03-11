//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef	LEUART_GUARD_H
#define	LEUART_GUARD_H

#include "brd_config.h"
#include "em_leuart.h"
#include "sleep_routines.h"
#include "HW_delay.h"


//***********************************************************************************
// defined files
//***********************************************************************************

#define LEUART_TX_EM		3
#define LEUART_RX_EM		2
#define LEUART_TX_CBB   0x00000020
#define LEUART_RX_CBB   0x00000040

/***************************************************************************//**
 * @addtogroup leuart
 * @{
 ******************************************************************************/

//***********************************************************************************
// defined variables
//***********************************************************************************

#define NO_DATA       0



typedef enum { //my three states
  INITIALIZATION_STATE, //never actually ended up needing an initialization State
  TRANSMISSION_STATE,
  STOP_STATE
}DEFINED_STATES_LEUART;

typedef enum {
  INIT_READ,
  RECEIVE_DATA,
  STOP_READ
}DEFINED_STATES_LEUART_READ;


typedef struct {
	uint32_t					baudrate;
	LEUART_Databits_TypeDef		databits;
	LEUART_Enable_TypeDef		enable;
	LEUART_Parity_TypeDef 		parity;
	LEUART_Stopbits_TypeDef		stopbits;
	bool						rxblocken;
	uint32_t        refFreq;
	bool						sfubrx;
	bool						startframe_en;
	char						startframe;
	bool						sigframe_en;
	char						sigframe;
	uint32_t					rx_loc;
	uint32_t					rx_pin_en;
	uint32_t					tx_loc;
	uint32_t					tx_pin_en;
	bool						rx_en;
	bool						tx_en;
	uint32_t					rx_done_evt;
	uint32_t					tx_done_evt;
} LEUART_OPEN_STRUCT;


/** @} (end addtogroup leuart) */


typedef struct {
  LEUART_TypeDef *leuart_state;
  volatile bool not_available;
  uint32_t cb_tx;
  uint32_t count_char; //should be incrementing in the TXBL and TXC state machines
  uint32_t data_string_length;
  char data_string[50];
  DEFINED_STATES_LEUART current_state;

}LEUART0_STATE_MACHINE;

typedef struct {
  LEUART_TypeDef *leuart_state_read;
  volatile bool not_available_read; //make it volatile to get it working in -O2
  uint32_t cb_rx; //not sure if I need to edit this


  char data_string_rx[50]; //could be any value, not just 50
  uint32_t read_counter;


  DEFINED_STATES_LEUART_READ current_state_read;

}LEUART0_STATE_MACHINE_READ;

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void leuart_open(LEUART_TypeDef *leuart, LEUART_OPEN_STRUCT *leuart_settings);
void LEUART0_IRQHandler(void);
void leuart_start(LEUART_TypeDef *leuart, char *string, uint32_t string_len);
bool leuart_tx_busy();

uint32_t leuart_status(LEUART_TypeDef *leuart);
void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update);
void leuart_if_reset(LEUART_TypeDef *leuart);
void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out);
uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart);

void leuart_txbel(LEUART0_STATE_MACHINE *sm);
void leuart_txc(LEUART0_STATE_MACHINE *sm);
void leuart_rxdatav(LEUART0_STATE_MACHINE_READ *sm_read); //should be asynchronous so use a different state machine
void leuart_startframe(LEUART0_STATE_MACHINE_READ *sm_read); // should be asynchronous so use a different state machine
void leuart_sigframe(LEUART0_STATE_MACHINE_READ *sm_read); //should be asynchronous so use a differnt state machine
void leuart_rx_tdd(void); //FOR READING PURPOSES
void return_read_val (char * ret_read); //doxygen done






#endif
