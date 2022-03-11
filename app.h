//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef APP_HG
#define APP_HG

/* System include statements */


/* Silicon Labs include statements */
#include "em_cmu.h"
#include "em_assert.h"

/* The developer's include statements */
#include "cmu.h"
#include "gpio.h"
#include "letimer.h"
#include "brd_config.h"
#include "scheduler.h"
#include "LEDs_thunderboard.h"
#include "Si1133.h"
#include "ble.h"
#include "leuart.h"

//***********************************************************************************
// defined files and defined variables
//***********************************************************************************
#define   PWM_PER         2.000   // PWM period in seconds
#define   PWM_ACT_PER     0.002  // PWM active period in seconds

//Si1133 read variables defines
#define   RETURN_READ       51 //This is the expected return read from the Si1133 and should be used for scheduled_si1133_read_cb(void) function
#define   QUANTITY_BYTES    1 // only receiving 1 byte of information from the sensor
#define   PART_ID_SI        0  //I2C Address for Part_ID on Si1133 Datasheet
#define   READ_RES_TWENTY   20

//Application scheduled events

// Application scheduled events, 6)f)
#define LETIMER0_COMP0_CB     0x00000001 //0b0001
#define LETIMER0_COMP1_CB     0x00000002 //0b0010
#define LETIMER0_UF_CB        0x00000004 //0b0100
#define SI1133_LIGHT_CB       0x00000008 //0b1000
#define BOOT_UP_CB            0x00000010 //0b10000
#define TX_CB                 0x00000020
#define RX_CB                 0x00000040
//each callback is represented by a unique bit

#define SYSTEM_BLOCK_EM       EM3
#define TWO_SEC_DELAY         2000



#define BLE_MOD_NAME            "SONALBLE"




//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void app_peripheral_setup(void);
void scheduled_letimer0_uf_cb(void);
void scheduled_letimer0_comp0_cb(void);
void scheduled_letimer0_comp1_cb(void);
void scheduled_si1133_read_cb(void);
void scheduled_bootup_cb(void);
void scheduled_rx_cb(void);
void scheduled_tx_cb(void);

#endif
