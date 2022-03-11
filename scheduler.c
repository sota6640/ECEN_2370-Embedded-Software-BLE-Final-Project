/**
 * @file scheduler.c
 * @author Sonal Tamrakar
 * @date 09/24/2021
 * @brief Prioritizes an event and cycles through the interrupts until the interrupts have been attended and serviced.
 * Works analogous to a state machine and cycle through interrupts. By using a scheduler, code is less strenuous to write.
 *
 */
#include "scheduler.h"


//*******************
//private variables
 static unsigned int event_scheduled;
//*******************


 /***************************************************************************//**
  * @brief
  * The void scheduler_open(void) opens the scheduler and it's functions.
  *
  * @details
  * Either setting or resetting the event_scheduled to 0. The three CORE statements are ensuring that the code is atomic.
  *
  * @note
  *  The CORE statements are added to ensure that the code in between will definitely occur, and won't be interrupted by global interrupts.
  ******************************************************************************/
void scheduler_open(void) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  event_scheduled = 0;
  CORE_EXIT_CRITICAL();
}


/***************************************************************************//**
  * @brief
  * The void add_scheduled_event utilizes the BITWISE operator OR, to OR the new event with the existing scheduled event.
  *
  * @details
  * event_scheduled is set by the (uint32_t event) which is the input variable to the function. The three CORE statements are ensuring that the code is atomic.
  *
  * @note
  *  The CORE statements are added to ensure that the code in between will definitely occur, and won't be interrupted by global interrupts.
  *
  *  @param[in] event
 *   The input uint32_t argument to the function that gets OR'ed with the event_scheduled.
*****************************************************************************/

void add_scheduled_event(uint32_t event) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  event_scheduled |= event;
  CORE_EXIT_CRITICAL();
}


/***************************************************************************//**
  * @brief
  * The void remove_scheduled_event utilizes the BITWISE operator ~ and &, to remove an event from the event_scheduled.
  *
  * @details
  * ANDing the current event_scheduled with the input variable which will remove an event. The three CORE statements are ensuring that the code is atomic.
  *
  * @note
  *  The CORE statements are added to ensure that the code in between will definitely occur, and won't be interrupted by global interrupts.
  *
  *  @param[in] event
  *   The input uint32_t argument to the function that gets AND'ed with the event_scheduled.
*****************************************************************************/


void remove_scheduled_event(uint32_t event) {
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  event_scheduled = event_scheduled & ~event;
  //CORE_DECLARE_IRQ_STATE;
  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
  * @brief
  * The get_scheduled_event(void) returns the current state of event_scheduled (static variable)
  *
  * @details
  * Outputs event_scheduled, which is the current state of the static variable.
  *
  * @note
  *  The CORE statements are added to ensure that the code in between will definitely occur, and won't be interrupted by global interrupts.
  *
  *  @param[out] event_scheduled
  *  event_scheduled is the static variable declared at the top of the .c file.
  *
*****************************************************************************/
uint32_t get_scheduled_events(void) {
  return event_scheduled; //returns the current state of the private(static) variable event_scheduled
}



