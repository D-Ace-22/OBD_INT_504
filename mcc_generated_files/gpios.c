#include "gpios.h"
#include "uart1.h"

/* Private types *********************************************************** */

/* Private constants ******************************************************* */

/* Private macros ********************************************************** */

/* Private variables ******************************************************* */
const GPIO_INFO c_GPIO_INFO[GPIO_ID_CNT] = 
{
    { //GPIO_ID_0
        .pin = 2, 
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_1
        .pin = 1,
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_2
        .pin = 3,
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_3
        .pin = 4,
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_4
        .pin = 0,
        .tris = (uint16_t *) &TRISC,
        .lat = (uint16_t *) &LATC,
        .port = (uint16_t *) &PORTC,
        .odc = (uint16_t *) &ODCC,
        .cnpu = (uint16_t *) &CNPUC,
        .cnpd = (uint16_t *) &CNPDC,
    },
    { //GPIO_ID_5
        .pin = 0,
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_6
        .pin = 7,
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_7
        .pin = 8,
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_8
        .pin = 12,
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_9
        .pin = 10,
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_10
        .pin = 13,
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_11
        .pin = 11,
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_12
        .pin = 6,
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_13
        .pin = 5,
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_14
        .pin = 9,
        .tris = (uint16_t *) &TRISA,
        .lat = (uint16_t *) &LATA,
        .port = (uint16_t *) &PORTA,
        .odc = (uint16_t *) &ODCA,
        .cnpu = (uint16_t *) &CNPUA,
        .cnpd = (uint16_t *) &CNPDA,
    },
    { //GPIO_ID_15
        .pin = 3,
        .tris = (uint16_t *) &TRISC,
        .lat = (uint16_t *) &LATC,
        .port = (uint16_t *) &PORTC,
        .odc = (uint16_t *) &ODCC,
        .cnpu = (uint16_t *) &CNPUC,
        .cnpd = (uint16_t *) &CNPDC,
    },
    { //GPIO_ID_16
        .pin = 4,
        .tris = (uint16_t *) &TRISA,
        .lat = (uint16_t *) &LATA,
        .port = (uint16_t *) &PORTA,
        .odc = (uint16_t *) &ODCA,
        .cnpu = (uint16_t *) &CNPUA,
        .cnpd = (uint16_t *) &CNPDA,
    },
    { //GPIO_ID_17
        .pin = 0xff,
        .tris = (uint16_t *) &TRISA,
        .lat = (uint16_t *) &LATA,
        .port = (uint16_t *) &PORTA,
        .odc = (uint16_t *) &ODCA,
        .cnpu = (uint16_t *) &CNPUA,
        .cnpd = (uint16_t *) &CNPDA,
    },
    { //GPIO_ID_18
        .pin = 2,
        .tris = (uint16_t *) &TRISC,
        .lat = (uint16_t *) &LATC,
        .port = (uint16_t *) &PORTC,
        .odc = (uint16_t *) &ODCC,
        .cnpu = (uint16_t *) &CNPUC,
        .cnpd = (uint16_t *) &CNPDC,
    },
    { //GPIO_ID_19
        .pin = 8,
        .tris = (uint16_t *) &TRISA,
        .lat = (uint16_t *) &LATA,
        .port = (uint16_t *) &PORTA,
        .odc = (uint16_t *) &ODCA,
        .cnpu = (uint16_t *) &CNPUA,
        .cnpd = (uint16_t *) &CNPDA,
    },
    { //GPIO_ID_20
        .pin = 0xff,
        .tris = (uint16_t *) &TRISA,
        .lat = (uint16_t *) &LATA,
        .port = (uint16_t *) &PORTA,
        .odc = (uint16_t *) &ODCA,
        .cnpu = (uint16_t *) &CNPUA,
        .cnpd = (uint16_t *) &CNPDA,
    },
    { //GPIO_ID_21
        .pin = 0xff,
        .tris = (uint16_t *) &TRISA,
        .lat = (uint16_t *) &LATA,
        .port = (uint16_t *) &PORTA,
        .odc = (uint16_t *) &ODCA,
        .cnpu = (uint16_t *) &CNPUA,
        .cnpd = (uint16_t *) &CNPDA,
    },
    { //GPIO_ID_22
        .pin = 10,
        .tris = (uint16_t *) &TRISA,
        .lat = (uint16_t *) &LATA,
        .port = (uint16_t *) &PORTA,
        .odc = (uint16_t *) &ODCA,
        .cnpu = (uint16_t *) &CNPUA,
        .cnpd = (uint16_t *) &CNPDA,
    },
    { //GPIO_ID_23
        .pin = 0xff,
        .tris = (uint16_t *) &TRISA,
        .lat = (uint16_t *) &LATA,
        .port = (uint16_t *) &PORTA,
        .odc = (uint16_t *) &ODCA,
        .cnpu = (uint16_t *) &CNPUA,
        .cnpd = (uint16_t *) &CNPDA,
    },
    { //GPIO_ID_24
        .pin = 7,
        .tris = (uint16_t *) &TRISA,
        .lat = (uint16_t *) &LATA,
        .port = (uint16_t *) &PORTA,
        .odc = (uint16_t *) &ODCA,
        .cnpu = (uint16_t *) &CNPUA,
        .cnpd = (uint16_t *) &CNPDA,
    },
    { //GPIO_ID_25
        .pin = 0xff,
        .tris = (uint16_t *) &TRISA,
        .lat = (uint16_t *) &LATA,
        .port = (uint16_t *) &PORTA,
        .odc = (uint16_t *) &ODCA,
        .cnpu = (uint16_t *) &CNPUA,
        .cnpd = (uint16_t *) &CNPDA,
    },
    { //GPIO_ID_26
        .pin = 14,
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_27
        .pin = 0xff,
        .tris = (uint16_t *) &TRISA,
        .lat = (uint16_t *) &LATA,
        .port = (uint16_t *) &PORTA,
        .odc = (uint16_t *) &ODCA,
        .cnpu = (uint16_t *) &CNPUA,
        .cnpd = (uint16_t *) &CNPDA,
    },
    { //GPIO_ID_28
        .pin = 9,
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_29
        .pin = 6,
        .tris = (uint16_t *) &TRISC,
        .lat = (uint16_t *) &LATC,
        .port = (uint16_t *) &PORTC,
        .odc = (uint16_t *) &ODCC,
        .cnpu = (uint16_t *) &CNPUC,
        .cnpd = (uint16_t *) &CNPDC,
    },
    { //GPIO_ID_30
        .pin = 7,
        .tris = (uint16_t *) &TRISC,
        .lat = (uint16_t *) &LATC,
        .port = (uint16_t *) &PORTC,
        .odc = (uint16_t *) &ODCC,
        .cnpu = (uint16_t *) &CNPUC,
        .cnpd = (uint16_t *) &CNPDC,
    },
    { //GPIO_ID_31
        .pin = 15,
        .tris = (uint16_t *) &TRISB,
        .lat = (uint16_t *) &LATB,
        .port = (uint16_t *) &PORTB,
        .odc = (uint16_t *) &ODCB,
        .cnpu = (uint16_t *) &CNPUB,
        .cnpd = (uint16_t *) &CNPDB,
    },
    { //GPIO_ID_32
        .pin = 1,
        .tris = (uint16_t *) &TRISA,
        .lat = (uint16_t *) &LATA,
        .port = (uint16_t *) &PORTA,
        .odc = (uint16_t *) &ODCA,
        .cnpu = (uint16_t *) &CNPUA,
        .cnpd = (uint16_t *) &CNPDA,
    },
    { //GPIO_ID_33
        .pin = 1,
        .tris = (uint16_t *) &TRISC,
        .lat = (uint16_t *) &LATC,
        .port = (uint16_t *) &PORTC,
        .odc = (uint16_t *) &ODCC,
        .cnpu = (uint16_t *) &CNPUC,
        .cnpd = (uint16_t *) &CNPDC,
    },
    { //GPIO_ID_34
        .pin = 4,
        .tris = (uint16_t *) &TRISC,
        .lat = (uint16_t *) &LATC,
        .port = (uint16_t *) &PORTC,
        .odc = (uint16_t *) &ODCC,
        .cnpu = (uint16_t *) &CNPUC,
        .cnpd = (uint16_t *) &CNPDC,
    },
    { //GPIO_ID_35
        .pin = 5,
        .tris = (uint16_t *) &TRISC,
        .lat = (uint16_t *) &LATC,
        .port = (uint16_t *) &PORTC,
        .odc = (uint16_t *) &ODCC,
        .cnpu = (uint16_t *) &CNPUC,
        .cnpd = (uint16_t *) &CNPDC,
    }
};

const uint8_t c_GPIO_CONFIG[GPIO_CONFIG_CNT][3] =
{
    {"I"},
    {"O"},
    {"N0"},
    {"N1"},
    {"U0"},
    {"U1"},
    {"D0"},
    {"D1"},
};



/* Private function prototypes ********************************************* */
static uint8_t getConfigIDGPIO(GPIO_CONFIG config);
/* Exported functions ****************************************************** */

uint8_t assertGPIO(GPIO_CONFIG config)
{
    if(c_GPIO_INFO[config.id].pin == 0xff)
    {
        return GPIO_STATUS_ID_INVALID;
    }
    /*uint8_t res = getConfigIDGPIO(config);
    if(res == GPIO_CONFIG_CNT)
    {
        return GPIO_STATUS_ID_INVALID;
    }*/
    else
    {
        return GPIO_STATUS_ID_VALID;
    }
}

uint8_t configGPIO(GPIO_CONFIG config)
{
    if(c_GPIO_INFO[config.id].pin == 0xff)
    {
        return GPIO_STATUS_ID_INVALID;
    }

    switch(config.ioConfig)//res
    {
        case (GPIO_CONFIG_OUTPUT):
        {
            *(c_GPIO_INFO[config.id].tris) &= ~(0x0001 << c_GPIO_INFO[config.id].pin);
            break;
        }
        case (GPIO_CONFIG_INPUT):
        {
            *(c_GPIO_INFO[config.id].tris) |= (0x0001 << c_GPIO_INFO[config.id].pin);
            break;
        }
        default:
        {
            return GPIO_STATUS_ID_INVALID;
        }
    }
    switch(config.ioTypeConfig)//res1
    {
        case (GPIO_CONFIG_OPEN_DRAIN_DISABLE):
        {
            *(c_GPIO_INFO[config.id].odc) &= ~(0x0001 << c_GPIO_INFO[config.id].pin);
            break;
        }
        case (GPIO_CONFIG_OPEN_DRAIN_ENABLE):
        {
            *(c_GPIO_INFO[config.id].odc) |= (0x0001 << c_GPIO_INFO[config.id].pin);
            break;
        }
        case (GPIO_CONFIG_PULL_UP_DISABLE):
        {
            *(c_GPIO_INFO[config.id].cnpu) &= ~(0x0001 << c_GPIO_INFO[config.id].pin);
            break;
        }
        case (GPIO_CONFIG_PULL_UP_ENABLE):
        {
            *(c_GPIO_INFO[config.id].cnpu) |= (0x0001 << c_GPIO_INFO[config.id].pin);
            break;
        }
        case (GPIO_CONFIG_PULL_DOWN_DISABLE):
        {
            *(c_GPIO_INFO[config.id].cnpd) &= ~(0x0001 << c_GPIO_INFO[config.id].pin);
            break;
        }
        case (GPIO_CONFIG_PULL_DOWN_ENABLE):
        {
            *(c_GPIO_INFO[config.id].cnpd) |= (0x0001 << c_GPIO_INFO[config.id].pin);
            break;
        }
        case ('\0'):
            break;
        default:
        {
            return GPIO_STATUS_ID_INVALID;
        }
    }
    return GPIO_STATUS_ID_VALID;
}


uint8_t setStateGPIO(GPIO_CONFIG config)
{
    if(c_GPIO_INFO[config.id].pin == 0xff)
    {
        return GPIO_STATUS_ID_INVALID;
    }

    
    if(config.state)
    {
        *(c_GPIO_INFO[config.id].lat) |= (0x0001 << c_GPIO_INFO[config.id].pin);
    }
    else
    {
        *(c_GPIO_INFO[config.id].lat) &= ~(0x0001 << c_GPIO_INFO[config.id].pin);
    }
    return GPIO_STATUS_ID_VALID;
}

uint8_t getStateGPIO(GPIO_CONFIG config)
{
    return ((*(c_GPIO_INFO[config.id].lat)) >> c_GPIO_INFO[config.id].pin) & 0x01;
}

uint8_t readInputGPIO(GPIO_CONFIG config)
{
    return ((*(c_GPIO_INFO[config.id].port)) >> c_GPIO_INFO[config.id].pin) & 0x01;
}

/* Private functions ******************************************************* */
uint8_t getConfigIDGPIO(GPIO_CONFIG config)
{
    uint8_t k = 0;
    for(k = 0; k < GPIO_CONFIG_CNT; k++)
    {
        if(strcmp(c_GPIO_CONFIG[k],config.ioConfig) == 0)
        {
            break;
        }
    }
    return k;
}
uint8_t getConfigIDGPIOType(GPIO_CONFIG config)
{
    uint8_t k = 0;
    for(k = 0; k < GPIO_CONFIG_CNT; k++)
    {
        if(strcmp(c_GPIO_CONFIG[k],config.ioTypeConfig) == 0)
        {
            break;
        }
    }
    return k;
}
/* ***************************** END OF FILE ******************************* */
