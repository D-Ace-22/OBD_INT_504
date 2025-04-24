/**
* @file        led.h
* @author      IRQdesign d.o.o 
* @date        July 2022
* @version     1.0.0
*/

/* Define to prevent recursive inclusion *********************************** */
#ifndef __LED_H
#define __LED_H
/* Includes **************************************************************** */
#include <xc.h>

/* Module configuration **************************************************** */

/* Exported constants ****************************************************** */
#define LED_SEQUENCE_TIMEOUT_PERIOD                 200
typedef
enum 
{
    LED_PIN_STATUS = 0x4000, //  ==> PORTB  - RB14 
    LED_PIN_HOST = 0x0080,   //  ==> PORTA  - RA7 
    LED_PIN_OBD = 0x0400,    //  ==> PORTA ?  RA10
} ledPin_t;

typedef
enum 
{
    LED_STATE_OFF = 0x00,
    LED_STATE_ON,
} ledState_t;

typedef
enum 
{
    LED_STATUS_ON = 0x00,
    LED_STATUS_OFF = 0x01,
}ledStatus_t;

typedef
enum 
{
    LED_ID_STATUS = 0x00,
    LED_ID_OBD,
    LED_ID_HOST,
} ledId_t;

typedef
enum 
{
    LED_STATE_SEQUENCE_INIT = 0x00,
    LED_STATE_SEQUENCE_NO1,
    LED_STATE_SEQUENCE_NO2,
    LED_STATE_SEQUENCE_NO3,
    LED_STATE_SEQUENCE_NO4,
} ledStateSequence_t;

/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */
typedef 
struct
{
    uint8_t state;
    uint32_t timer;
    uint8_t status;

} ledModule_t;

/* Exported variables ****************************************************** */
extern volatile ledModule_t g_Led;

/* Exported functions ****************************************************** */
void initLED(void);
void setLED(uint16_t id, uint8_t state) ;
uint8_t startupSequenceLED(void);
void setStatusLED(uint8_t status);


#endif 

