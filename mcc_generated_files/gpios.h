/* Define to prevent recursive inclusion *********************************** */
#ifndef __GPIO_H
#define __GPIO_H
/* Includes **************************************************************** */
#include <xc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* Module configuration **************************************************** */

/* Exported constants ****************************************************** */
enum GPIO_ID
{
    GPIO_ID_0 = 0x00,
    GPIO_ID_1,
    GPIO_ID_2,
    GPIO_ID_3,
    GPIO_ID_4,
    GPIO_ID_5,
    GPIO_ID_6,
    GPIO_ID_7,
    GPIO_ID_8,
    GPIO_ID_9,
    GPIO_ID_10,
    GPIO_ID_11,
    GPIO_ID_12,
    GPIO_ID_13,
    GPIO_ID_14,
    GPIO_ID_15,
    GPIO_ID_16,
    GPIO_ID_17,
    GPIO_ID_18,
    GPIO_ID_19,
    GPIO_ID_20,
    GPIO_ID_21,
    GPIO_ID_22,
    GPIO_ID_23,
    GPIO_ID_24,
    GPIO_ID_25,
    GPIO_ID_26,
    GPIO_ID_27,
    GPIO_ID_28,
    GPIO_ID_29,
    GPIO_ID_30,
    GPIO_ID_31,
    GPIO_ID_32,
    GPIO_ID_33,
    GPIO_ID_34,
    GPIO_ID_35,

    GPIO_ID_CNT
};

enum GPIO_CONFIG_MODULE
{
    GPIO_CONFIG_INPUT = 0x00,
    GPIO_CONFIG_OUTPUT,
    GPIO_CONFIG_OPEN_DRAIN_DISABLE,
    GPIO_CONFIG_OPEN_DRAIN_ENABLE,
    GPIO_CONFIG_PULL_UP_DISABLE,
    GPIO_CONFIG_PULL_UP_ENABLE,
    GPIO_CONFIG_PULL_DOWN_DISABLE,
    GPIO_CONFIG_PULL_DOWN_ENABLE,
    GPIO_CONFIG_CNT,
};

enum GPIO_STATUS
{
    GPIO_STATUS_ID_VALID = 0x00,
    GPIO_STATUS_ID_INVALID,
};
/* Exported macros ********************************************************* */

/* Exported types ********************************************************** */
typedef struct gpio_module_t
{
    uint16_t pin;
    uint16_t * tris;
    uint16_t * lat;
    uint16_t * port;
    uint16_t * odc;
    uint16_t * cnpu;
    uint16_t * cnpd;
} GPIO_INFO;

typedef struct gpio_config_module_t
{
    uint8_t id;
    uint8_t ioConfig;
    uint8_t ioTypeConfig;
    uint8_t state;
} GPIO_CONFIG;

/* Exported variables ****************************************************** */
extern const GPIO_INFO c_GPIO_INFO[GPIO_ID_CNT];

/* Exported functions ****************************************************** */
uint8_t configGPIO(GPIO_CONFIG config);
uint8_t assertGPIO(GPIO_CONFIG config);
uint8_t setStateGPIO(GPIO_CONFIG config);
uint8_t getStateGPIO(GPIO_CONFIG config);
uint8_t readInputGPIO(GPIO_CONFIG config);
uint8_t getConfigIDGPIOType(GPIO_CONFIG config);
uint8_t readGPIO(uint8_t id);

#endif 