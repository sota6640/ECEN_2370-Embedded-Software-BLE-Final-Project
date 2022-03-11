/**
 * @file gpio.c
 * @author Sonal Tamrakar
 * @date 09/11/2021
 * @brief Enabling the GPIO clock through the CMU for the LEDs
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************


//***********************************************************************************
// functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * Enabling/disabling cmuCLOCK_GPIO for LED pins (red + green ports)
 *
 * @details
 *  Sets the drive strength and the pin modes for gpioPortD8(red) and gpioPortD9(green)
 *
 * @note
 *  brd.config.h file has all the needed defines that's used to connect the GPIO pins the LED's are connected to the correct LETIMER register(ROUTELOC0) in the driver file.
 *
 ******************************************************************************/

void gpio_open(void){

  CMU_ClockEnable(cmuClock_GPIO, true);

	// Configure LED pins
	GPIO_DriveStrengthSet(LED_RED_PORT, LED_RED_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED_RED_PORT, LED_RED_PIN, LED_RED_GPIOMODE, LED_RED_DEFAULT);

	GPIO_DriveStrengthSet(LED_GREEN_PORT, LED_GREEN_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED_GREEN_PORT, LED_GREEN_PIN, LED_GREEN_GPIOMODE, LED_GREEN_DEFAULT);



	// Set RGB LED gpiopin configurations
	GPIO_PinModeSet(RGB_ENABLE_PORT, RGB_ENABLE_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
	GPIO_PinModeSet(RGB0_PORT, RGB0_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
	GPIO_PinModeSet(RGB1_PORT, RGB1_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
	GPIO_PinModeSet(RGB2_PORT, RGB2_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
	GPIO_PinModeSet(RGB3_PORT, RGB3_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
	GPIO_PinModeSet(RGB_RED_PORT, RGB_RED_PIN, gpioModePushPull, COLOR_DEFAULT_OFF);
	GPIO_PinModeSet(RGB_GREEN_PORT, RGB_GREEN_PIN, gpioModePushPull, COLOR_DEFAULT_OFF);
	GPIO_PinModeSet(RGB_BLUE_PORT, RGB_BLUE_PIN, gpioModePushPull, COLOR_DEFAULT_OFF);

	//Set Sensor configs
	GPIO_PinModeSet(SI1133_SENSOR_EN_PORT, SI1133_SENSOR_EN_PIN, gpioModePushPull, SI1133_SENSOR_DEFAULT_ASSERT_TRUE); //page 6 of lab 4 towards the top
	GPIO_PinModeSet(SI1133_SCL_PORT, SI1133_SCL_PIN, gpioModeWiredAnd, SI1133_SCL_ASSERT_TRUE );
	GPIO_PinModeSet(SI1133_SDA_PORT, SI1133_SDA_PIN, gpioModeWiredAnd, SI1133_SDA_ASSERT_TRUE );

	GPIO_PinModeSet(LEUART_TX_PORT, LEUART_TX_PIN, gpioModePushPull, LEUART_TX_ASSERT_FALSE);
	GPIO_PinModeSet(LEUART_RX_PORT, LEUART_RX_PIN, gpioModeInput, LEUART_RX_ASSERT_FALSE);

	// Configure Sensor Enable
	  GPIO_DriveStrengthSet(SI1133_SENSOR_EN_PORT, gpioDriveStrengthWeakAlternateWeak);  //page 6 of lab 4 towards the top

	  GPIO_DriveStrengthSet(LEUART_TX_PORT, gpioDriveStrengthStrongAlternateWeak);
}
