/* Includes **************************************************************** */
#include "led.h"
#include "delay.h"

/* Private types *********************************************************** */

/* Private constants ******************************************************* */

/* Private macros ********************************************************** */

/* Private variables ******************************************************* */
volatile ledModule_t g_Led;

/* Private function prototypes ********************************************* */

/* Exported functions ****************************************************** */

/* Private functions ******************************************************* */

void initLED(void) {
    g_Led.state = LED_STATE_SEQUENCE_INIT;
    g_Led.status = LED_STATUS_ON;
    setLED(LED_ID_OBD, LED_STATE_OFF);
    setLED(LED_ID_HOST, LED_STATE_OFF);
    setLED(LED_ID_STATUS, LED_STATE_OFF);

#ifdef USE_STATUS_LED   
    TRISB &= ~(LED_PIN_STATUS);
#endif   
    TRISA &= ~(LED_PIN_OBD);
#ifdef USE_HOST_LED
    TRISA &= ~(LED_PIN_HOST);
#endif


}

void setLED(uint16_t id, uint8_t state) {
    if (g_Led.status != LED_STATUS_ON) {
        return;
    }
    switch (id) {
        case(LED_ID_STATUS):
        {
#ifdef USE_STATUS_LED 
            if (state == LED_STATE_ON) {
                PORTB |= LED_PIN_STATUS;
            } else {
                PORTB &= ~(LED_PIN_STATUS);
            }
#endif
            break;
        }
        case(LED_ID_OBD):
        {

            if (state == LED_STATE_ON) {
                PORTA &= ~(LED_PIN_OBD);
            } else {
                PORTA |= LED_PIN_OBD;

            }
            break;
        }
        case(LED_ID_HOST):
        {
#ifdef USE_HOST_LED            
            if (state == LED_STATE_ON) {
                PORTA &= ~(LED_PIN_HOST);
            } else {
                PORTA |= (LED_PIN_HOST);
            }
#endif
            break;
        }
    }

}

uint8_t startupSequenceLED(void) {
    switch (g_Led.state) {
        case(LED_STATE_SEQUENCE_INIT):
        {
            g_Led.timer = getSYSTIM();
            g_Led.state = LED_STATE_SEQUENCE_NO1;
            setLED(LED_ID_STATUS, LED_STATE_ON);

            break;
        }
        case(LED_STATE_SEQUENCE_NO1):
        {
            if (chk4TimeoutSYSTIM(g_Led.timer, LED_SEQUENCE_TIMEOUT_PERIOD) == SYSTIM_TIMEOUT) {
                g_Led.timer = getSYSTIM();
                g_Led.state = LED_STATE_SEQUENCE_NO2;
                setLED(LED_ID_OBD, LED_STATE_ON);
            }
            break;
        }
        case(LED_STATE_SEQUENCE_NO2):
        {
            if (chk4TimeoutSYSTIM(g_Led.timer, LED_SEQUENCE_TIMEOUT_PERIOD) == SYSTIM_TIMEOUT) {
                g_Led.timer = getSYSTIM();
                g_Led.state = LED_STATE_SEQUENCE_NO3;
                setLED(LED_ID_HOST, LED_STATE_ON);
            }

            break;
        }
        case(LED_STATE_SEQUENCE_NO3):
        {
            if (chk4TimeoutSYSTIM(g_Led.timer, LED_SEQUENCE_TIMEOUT_PERIOD) == SYSTIM_TIMEOUT) {
                g_Led.timer = getSYSTIM();
                g_Led.state = LED_STATE_SEQUENCE_NO4;
                setLED(LED_ID_OBD, LED_STATE_OFF);
            }
            break;
        }
        case(LED_STATE_SEQUENCE_NO4):
        {
            if (chk4TimeoutSYSTIM(g_Led.timer, LED_SEQUENCE_TIMEOUT_PERIOD) == SYSTIM_TIMEOUT) {
                g_Led.timer = getSYSTIM();
                setLED(LED_ID_HOST, LED_STATE_OFF);
                return 1;
            }
            break;
        }
    }
    return 0;
}

void setStatusLED(uint8_t status) 
{
    g_Led.status = status;
}

/* ***************************** END OF FILE ******************************* */


