//***********************************************************************************
// Include files
//***********************************************************************************
#include "LEDs_thunderboard.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************
bool	rgb_enabled_status;

//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

void rgb_init(void) {
	rgb_enabled_status = false;
  GPIO_PinOutClear(RGB0_PORT,RGB0_PIN);
  GPIO_PinOutClear(RGB1_PORT,RGB1_PIN);
  GPIO_PinOutClear(RGB2_PORT,RGB2_PIN);
  GPIO_PinOutClear(RGB3_PORT,RGB3_PIN);
  GPIO_PinOutSet(RGB_ENABLE_PORT,RGB_ENABLE_PIN);
}

void leds_enabled(uint32_t leds, uint32_t color, bool enable){

    if ((color & COLOR_RED) && enable) {
      GPIO_PinOutSet(RGB_RED_PORT,RGB_RED_PIN);
    } else if ((color & COLOR_RED) && !enable) GPIO_PinOutClear(RGB_RED_PORT,RGB_RED_PIN);

    if ((color & COLOR_GREEN) && enable) {
      GPIO_PinOutSet(RGB_GREEN_PORT,RGB_GREEN_PIN);
    } else if ((color & COLOR_GREEN) && !enable)  GPIO_PinOutClear(RGB_GREEN_PORT,RGB_GREEN_PIN);

    if ((color & COLOR_BLUE) && enable) {
      GPIO_PinOutSet(RGB_BLUE_PORT,RGB_BLUE_PIN);
    } else if ((color & COLOR_BLUE) && !enable)  GPIO_PinOutClear(RGB_BLUE_PORT,RGB_BLUE_PIN);

		if ((leds & RGB_LED_0) && enable) {
			GPIO_PinOutSet(RGB0_PORT,RGB0_PIN);
		} else if ((leds & RGB_LED_0) && !enable) GPIO_PinOutClear(RGB0_PORT,RGB0_PIN);

		if ((leds & RGB_LED_1) && enable) {
			GPIO_PinOutSet(RGB1_PORT,RGB1_PIN);
		} else if ((leds & RGB_LED_1) && !enable)  GPIO_PinOutClear(RGB1_PORT,RGB1_PIN);

		if ((leds & RGB_LED_2) && enable) {
			GPIO_PinOutSet(RGB0_PORT,RGB2_PIN);
		} else if ((leds & RGB_LED_2) && !enable)  GPIO_PinOutClear(RGB2_PORT,RGB2_PIN);

		if ((leds & RGB_LED_3) && enable) {
			GPIO_PinOutSet(RGB3_PORT,RGB3_PIN);
		} else if ((leds & RGB_LED_3) && !enable)  GPIO_PinOutClear(RGB3_PORT,RGB3_PIN);

}

