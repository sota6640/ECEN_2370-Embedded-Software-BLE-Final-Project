/**
 * @file cmu.c
 * @author Sonal Tamrakar
 * @date 09/11/2021
 * @brief This function utilized the clock management unit to establish/set up the clock tree
 *
 */
//***********************************************************************************
// Include files
//***********************************************************************************
#include "cmu.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************


//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * Routing the needed oscillator(ULFRCO) to the proper Low frequency clock branch and connecting to the LETIMER0 peripheral through a clock gate.
 *
 * @details
 * In order to select the oscillator we desire(ULFRCO), we disable the other two, and select ULFRCO. LFA(low frequency) is selected then to connect to the LETIMER0 peripheral, need to
 * go through the clock gate. This function sets up the clock tree making sure that the clocks are working in the energy modes that we are operating in.
 *
 * @note
 *  All clocks are disabled at first in order to save energy. In order to have data written to a register, we need to properly configure it. ASSERTION tests can be
 *  helpful in determining if the clock is enabled becuase if it isn't, the program isn't going to crash.
 *
 ******************************************************************************/

void cmu_open(void){

    CMU_ClockEnable(cmuClock_HFPER, true);

    // By default, LFRCO is enabled, disable the LFRCO oscillator
    // Disable the LFRCO oscillator
    // What is the enumeration required for LFRCO?
    // It can be found in the online HAL documentation
    CMU_OscillatorEnable(cmuOsc_LFRCO , false, false);
    //disabling the LFRCO oscillator

    // Disable the LFXO oscillator
    // What is the enumeration required for LFXO?
    // It can be found in the online HAL documentation
    CMU_OscillatorEnable(cmuOsc_LFXO , true, true);
    //disabled the LFXO oscillator

    // No requirement to enable the ULFRCO oscillator.  It is always enabled in EM0-4H1

    // Route LF clock to the LF clock tree
    // What is the enumeration required to placed the ULFRCO onto the proper clock branch?
    // It can be found in the online HAL documentation
    CMU_ClockSelectSet(cmuClock_LFA  , cmuSelect_ULFRCO);    // routing ULFRCO to proper Low Freq clock tree
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

    // What is the proper enumeration to enable the clock tree onto the LE clock branches?
    // It can be found in the Assignment 2 documentation
    CMU_ClockEnable(cmuClock_CORELE, true); // CMU_ClockEnable(cmuClock_CORELE, true);

    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

    // didnt highlight blue

    //possibly 16) iv.) here , the CMU_ClockEnable() will either assert, set to 1, or deassert, clear to 0, the CMU_LFACLKEN0.LETIMER0 signal

}

