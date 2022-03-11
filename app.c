/**
 * @file app.c
 * @author Sonal Tamrakar
 * @date 10/17/2021
 * @brief The app.c module is responsible for setting up the peripheral, specifically LETIMER0 for PWM purposes. It configures the LETIMER peripheral.
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "app.h"
#include <stdio.h>


//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************
static uint32_t x = 3;
static uint32_t y = 0;
#define BLE_TEST_ENABLED

//***********************************************************************************
// Private functions
//***********************************************************************************

static void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route);

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * app_peripheral_setup provides all of the needed function calls, such as the cmu, gpio and the LETIMER.
 *
 * @details
 * On app_peripheral_setup(void), the function calls are being completed line by line and making sure that the peripheral can utilize it's open driver functions.
 *
 * @note
 *  The functions give a setup on how the peripheral is being setup. It's a step by step process. First on cmu_open() which sets up the clock tree, then next step then is to enable the general
 * purpose input/output clock in GPIO_open function, and so on.
 ******************************************************************************/

void app_peripheral_setup(void){
  scheduler_open();
  sleep_open();

  cmu_open();
  gpio_open();
  si1133_i2c_open();
  rgb_init();
  app_letimer_pwm_open(PWM_PER, PWM_ACT_PER, PWM_ROUTE_0, PWM_ROUTE_1);
  ble_open(TX_CB, RX_CB);
  letimer_start(LETIMER0, true);  //This command will initiate the start of the LETIMER0
  add_scheduled_event(BOOT_UP_CB);
  sleep_block_mode(SYSTEM_BLOCK_EM);
}

/***************************************************************************//**
 * @brief
 * This function configure LETIMER0 with PWM mode and sends all the needed elements to our driver function letimer_pwm_open() function.
 * The LETIMER PWM is initialized in this function using STRUCT typedef.
 *
 * @details
 * This function contains all of the elements that need to be initialized for proper PWM functionality. There are various parameters that have to be tweaked. Defined values such as ON
 * and OFF times are setup in this function.
 *
 * @note
 *  At the end of the function, we send all of the parameters/input values to the letimer_pwm_open() using our struct TYPEDEF. The elements in this function are the application
 *  requirements for the driver function.
 *
 * @param[in] period
 *  The total period of our PWM cycle defined in the app.h file.
 *
 * @param[in] out_pin_route0
 * The location that will be used to route the LETIMER0 outputs to the output pins of the Gecko green LED.
 *
 ******************************************************************************/

void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route){
  // Initializing LETIMER0 for PWM operation by creating the
  // letimer_pwm_struct and initializing all of its elements
  // APP_LETIMER_PWM_TypeDef is defined in letimer.h
  APP_LETIMER_PWM_TypeDef   letimer_pwm_struct; //15) d) the first line of code must be the declaration of a local APP_LETIMER_PWM_TypeDef STRUCT
  //letimer_pwm_struct is the struct name of your choice

  letimer_pwm_struct.debugRun = false;
  letimer_pwm_struct.enable = false; //15) f) don't want to enable or turn-on the LETIMER until the peripheral is completely programmed
  letimer_pwm_struct.out_pin_0_en = false;
  letimer_pwm_struct.out_pin_1_en = false;
  letimer_pwm_struct.period = period; //seconds 3.000 seconds from the defined files
  letimer_pwm_struct.active_period = act_period; //seconds 0.250 seconds from the defined files
  letimer_pwm_struct.out_pin_route0 = out0_route;
  letimer_pwm_struct.out_pin_route1 = out1_route;

  letimer_pwm_struct.uf_irq_enable = true; //6)g)i
  letimer_pwm_struct.comp1_irq_enable = true; //6)g)i
  letimer_pwm_struct.comp0_irq_enable = false; //6)g)ii
  letimer_pwm_struct.comp0_cb = LETIMER0_COMP0_CB;
  letimer_pwm_struct.comp1_cb = LETIMER0_COMP1_CB;
  letimer_pwm_struct.uf_cb = LETIMER0_UF_CB;





  letimer_pwm_open(LETIMER0, &letimer_pwm_struct); // 15) e) elements that will be sent to letimer_pwm_open() function
  //letimer_pwm_struct is the struct name of your choice
  //I. the last line of code for this routine  should be:
}

/***************************************************************************//**
 * @brief
 * The void scheduled_letimer0_uf_cb is responsible for requesting results from the Si1133 as well as perform
 * a simple mathematical operation and send the result through bluetooth.
 *
 * @details
 * In this underflow function, the request res() will be called which will then call
 * the Si1133 read function to read the sensor values. The function has three variables,
 * x, y and z which keeps changing and the z value is sent out to the ble_write function
 * as a string.
 *
 * @note
 *  In this project, uf, comp0 and comp1 are the interrupts that are being called. Note that we are
 *  using sprintf to format the data that we desire in a certain way.
 ******************************************************************************/
void scheduled_letimer0_uf_cb(void){
  EFM_ASSERT(!(get_scheduled_events() & LETIMER0_UF_CB));
  request_res();
  float z;
  x = x + 3;
  y = y + 1;
  z = (float)x / (float)y;
  char string_app[50];
  sprintf(string_app, "Z = %2.1f\n", z);

  ble_write(string_app);
}

/***************************************************************************//**
 * @brief
 *  For this application specifically we are not using the LETIMER Comp 0 interrupt.
 *
 * @details
 * We are inserting an ASSERT statement which returns false so it it ever enters this function,
 * the code will go into an infinite while(1) loop.
 * @note
 * This function can be used as a diagnostic step in the debugger. If it ever enters this function, we will
 * know that the interrupt was gone uninitialized.
 ******************************************************************************/

void scheduled_letimer0_comp0_cb(void) {
  //EFM_ASSERT(false);
  //EFM_ASSERT(get_scheduled_events() & LETIMER0_COMP0_CB);
}

/***************************************************************************//**
 * @brief
 *  The void scheduled_letimer0_comp1_cb occurs when the interrupt is triggered by the comp0 for letimer0. We'll use this for I2C purposes.
 *
 * @details
 *  comp1 callback interrupt function takes care of any functions that is within this function that has to be executed. For this lab, we are calling the
 *  FORCE command to the sensor to initiate sensing.
 *
 * @note
 * After the letimer0_comp1 interrupt has occured, for this function, the master will write a FORCE command to the Si1133.
 ******************************************************************************/

void scheduled_letimer0_comp1_cb(void) {
  force_send();
}


/***************************************************************************//**
 * @brief
 *  The void scheduled_si1133_read_cb(void) function actively checks and compare the Si1133 read value to 20.
 *
 * @details
 *  If the Si1133 read data is below 20, then the BLUE LED turns on and if the Si1133 read data
 *  is above 20, then the BLUE LED turns off. Another feature that has been added is that if the Si1133 read data is below 20,
 *  it writes a command to ble_write which transmits the si1133_read_check via bluetooth and write
 *  "It's dark". It will write "It's light outside" along with the si1133_read_check value via
 *  bluetooth.
 * @note
 *  On hardware, the way to implement this is by putting your finger over the sensor, which will cause the
 *  sensor value to go down and the LED to turn on and under sunlight/bright light, the sensor value goes
 *  up turning the LED off.
 *
 ******************************************************************************/
void scheduled_si1133_read_cb(void) {
 // EFM_ASSERT(!(get_scheduled_events() & SI1133_LIGHT_CB));
  uint32_t si1133_read_check = send_si1133_data();

  if (si1133_read_check < READ_RES_TWENTY) {

      leds_enabled(RGB_LED_1, COLOR_BLUE, true);
      char string_read_val[50];
      float read = (float) si1133_read_check;
      sprintf(string_read_val, "It's dark = %3.0f\n", read);
      ble_write(string_read_val);
  }
  else if (si1133_read_check >= READ_RES_TWENTY) {

      leds_enabled(RGB_LED_1, COLOR_BLUE, false);
      char string_read_val_2[50];
      float read2 = (float) si1133_read_check;
      sprintf(string_read_val_2, "It's light outside = %3.0f\n", read2);
      ble_write(string_read_val_2);

  }
 }
/***************************************************************************//**
 * @brief
 *  The scheduled_bootup_cb(void) function is used to set up the BLE module, it gives
 *  the module a unique name, in this case SONALBLE and before starting the LETIMER,
 *  it writes the string "HelloWorld"
 *
 * @details
 * On this function, I am constantly ensuring that the ble_test mod name is the one that
 * I'm expecting. The bootup_cb function is run only once in the program which initializes
 * the entire bluetooth process.
 * @note
 *
 ******************************************************************************/

void scheduled_bootup_cb(void) {

  EFM_ASSERT(!(get_scheduled_events() & BOOT_UP_CB));
#ifndef BLE_TEST_ENABLED
   bool res_value = ble_test(BLE_MOD_NAME);
   EFM_ASSERT(res_value);
  timer_delay(TWO_SEC_DELAY); //MAgic
#endif
  ble_write("\nHelloWorld\n");
  letimer_start(LETIMER0, true);
}

void scheduled_tx_cb(void) {
  //never ended up writing anything here from lab 6, so probably not required for Lab 7
}

/***************************************************************************//**
 * @brief
 *  Application code after receiving a Bluetooth receive callback, LEUART_RX_CB (0x40)
 *
 * @details
 *  This function calls another function, return_read_val(char * ret_read) which puts the ASCII value
 *  that was inputed into the Bluetooth Terminal application in to the char *ret_read.
 *  It treats that ASCII value as an array of characters. It runs the if statements making sure we are getting the correct characters. The 0th character is the startframe
 *  and the last character is the sigframe and the ASCII value we want to access is contained within the
 *  startframe and the sigframe. If the 1st character is 'U', it runs another if statement to check if the 2nd
 *  character is either a '+' or a '-'. If it's '+', it calculates the amount in milliseconds to increment and
 *  if the character is '-', it calculates the amount in milliseconds to decrement and multiplies it by (-1).
 *  In both cases, the change is saved into a local integer within the function.
 *
 * @note
 *
 * The compare_set(LETIMER_TypeDef * letimer, int increment_decrement) function is called at the end of this
 * function with one of the parameters as the change in speed of the active period. The compare_set function is
 * in letimer.c
 ******************************************************************************/
void scheduled_rx_cb(void) {

   char s_string[50];
   uint32_t change_speed = 0;

   return_read_val(s_string);

   if (s_string[1] == 'U') {

       if (s_string[2] == '+')
         {
           change_speed = ((s_string[3] - 0x30)*100)+((s_string[4] - 0x30)*10)+(s_string[5] - 0x30);
         }
       else if (s_string[2] == '-')
         {
           change_speed = (((s_string[3] - 0x30)*100)+((s_string[4] - 0x30)*10)+(s_string[5] - 0x30));
           change_speed = change_speed * (-1);
         }
   }

   compare_set(LETIMER0, change_speed);



}








