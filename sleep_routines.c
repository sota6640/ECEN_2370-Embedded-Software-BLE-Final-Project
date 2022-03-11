/**
 * @file sleep_routines.c
 * @author Sonal Tamrakar
 * @date 09/24/2021
 * @brief This .c file manages the sleep routine by setting a static variable which maintains that state
 * that the microcontroller cannot enter depending on what peripherals it is using.
 */
/***
 * @file sleep.c
 * **
 * @section License*
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com/</b>
 * *
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 *
 *  DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 *  obligation to support this Software. Silicon Labs is providing the
 *  Software "AS IS", with no express or implied warranties of any kind,
 *  including, but not limited to, any implied warranties of merchantability
 *  or fitness for any particular purpose or warranties against infringement
 *  of any proprietary rights of a third party.
 *
 *  Silicon Labs will not be liable for any consequential, incidental, or
 *  special damages, or any other relief, or for any claim by any third party,
 *  arising from your use of this Software.
 ***/

#include "sleep_routines.h"


//private variables

static int lowest_energy_mode[MAX_ENERGY_MODES];
//***********************************************


/***************************************************************************//**
  * @brief
  * Checks every energy mode using the lowest_energy_mode[] array and will enter the appropriate one that is non-zero.
  *
  * @details
  * In this function, If/else loops were used to cycle through the elements of the lowest_energy_mode[] array.
  * The If/Else statements are made atomic.
  *
  *
  * @note
  * CORE statements makes sure that there is isolation from concurrent processes that are running.
  ******************************************************************************/

void enter_sleep(void) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  if (lowest_energy_mode[EM0] > 0) {
      return;
  }

  else if(lowest_energy_mode[EM1] > 0) {
      return;
  }

  else if (lowest_energy_mode[EM2] > 0) {
      EMU_EnterEM1();
      return;
  }

  else if (lowest_energy_mode[EM3] > 0) {
      EMU_EnterEM2(true);
      return;
  }

  else {
      EMU_EnterEM3(true);
      return;
  }

  CORE_EXIT_CRITICAL();

}


/***************************************************************************//**
  * @brief
  * The function returns the mode that the system is not able to enter, meaning that energy mode is currently blocked.
  *
  * @details
  * Runs a for loop to from 0 to MAX_ENERGY_MODES and if that element in the static variable != 0, then it returns that specific element within
  * the lowest_energy_mode[i].
  * @note
  *
  * @param[out] i
  *  Returns the energy mode the program cannot enter, in this case, that
  *  energy mode is represented by  the variable i.
*****************************************************************************/


uint32_t current_block_energy_mode(void) {
  int i = 0;
  for (i = 0; i < MAX_ENERGY_MODES; i++) {
      if(lowest_energy_mode[i] != 0) {
          return i;
      }
  }
  return (MAX_ENERGY_MODES - 1); //-1 due to array starts at 0

}

/***************************************************************************//**
  * @brief
  * The void sleep_unblock_mode(uint32_t EM) function is used to release the processor from going into sleep mode.
  *
  * @details
  * the lowest_energy_mode[EM] will be decreased by 1 indicating that the energy state is no longer active.
  *
  * @note
  * The decrement is atomic. The EFM_ASSERT at the end of the function is to make sure that the state of any sleep is not going to 0.
  *  @param[in] EM
  * uint32_t EM is a value from 0-4 which indicates the very first state not to enter
*****************************************************************************/

void sleep_unblock_mode(uint32_t EM) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  lowest_energy_mode[EM] = lowest_energy_mode[EM] -1 ;

  CORE_EXIT_CRITICAL();
  EFM_ASSERT(lowest_energy_mode[EM] >= 0);
}


/***************************************************************************//**
  * @brief
  * The void sleep_block_mode(uint32_t EM) function is used when the peripheral is active to prevent the microcontroller from going into that sleep mode.
  *
  * @details
  * the lowest_energy_mode[EM] will be increased by 1. The increment is atomic.
  * The increment is atomic. The EFM_ASSERT at the end of the function is to make sure that the state of any sleep is less than 5.
  * @note
  *
  *  @param[in] EM
  * EM is the input variable which accesses the certain element in the lowest_energy_mode array.
*****************************************************************************/
void sleep_block_mode(uint32_t EM) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  lowest_energy_mode[EM] = lowest_energy_mode[EM] + 1 ;

  CORE_EXIT_CRITICAL();
  EFM_ASSERT(lowest_energy_mode[EM] < 5);
}


/***************************************************************************//**
  * @brief
  * This functions sets all of the private/static array elements to zero.
  *
  * @details
  * For all the energy modes there are, 5, the function will run a for loop and initialize the entire lowest_energy_modes[] to 0.
  *
  * @note
  *
  ******************************************************************************/
void sleep_open(void) {

  for (int j=0; j < MAX_ENERGY_MODES; j++) {
      lowest_energy_mode[j] = 0;
  }

}






