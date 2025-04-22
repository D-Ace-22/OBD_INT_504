#include "delay.h"


#define MS_TIME_OPERATOR 1000

volatile uint32_t gu32Jiffies1ms = 0;
volatile uint32_t g_sysTimeCount = 0x00;
void delayMs(uint32_t ms)
{
    uint32_t dt1 = (getSYSTIM()/MS_TIME_OPERATOR);
    
    while( ms < (getSYSTIM()/MS_TIME_OPERATOR - dt1)){
        Nop();// asm("nop");
    }
    
}

void delayUs(uint32_t us)
{
	//g_systim_cnt				
}

void initSYSTIM(void)
{		

    T3CONbits.TON = 0; // Disable Timer
    T3CONbits.TCS = 0; // Select internal instruction cycle clock
    T3CONbits.TGATE = 0; // Disable Gated Timer mode
    T3CONbits.TCKPS = 0b01; // Select 1:1 Prescaler (??? => should be 1:8)
    TMR3 = 0x00; // Clear timer register
    //PR3 = 19999; // Load the period value
    PR3 = 7500; 
    IPC2bits.T3IP = 0x01; // Set Timer3 Interrupt Priority Level
    IFS0bits.T3IF = 0; // Clear Timer3 Interrupt Flag
    IEC0bits.T3IE = 1; // Enable Timer3 interrupt
    T3CONbits.TON = 1;
    
    T2CONbits.TON = 1; // Start 16-bit Timer							
}

void __attribute__((__interrupt__,auto_psv)) _T3Interrupt(void)
{
    IFS0bits.T3IF = 0; //Clear Timer3 interrupt flag
    g_sysTimeCount++;
}


uint32_t getSYSTIM(void)
{
	//return TIM5->CNT/4;
    return g_sysTimeCount;
}

/**
 * 
 * @param btime system time in millisecound for comparing stored value)
 * @param period period in millisecounds
 * @return SYSTIM_TIMEOUT if period > differnce time otherwise SYSTIM_KEEP_ALIVE
 */
uint8_t chk4TimeoutSYSTIM(uint32_t btime, uint32_t period)
{
	uint32_t time = g_sysTimeCount;
	//uint32_t time = 0x00000000;
	if(time >= btime)
	{
		if((time - btime) >= period)
			return (SYSTIM_TIMEOUT);
		else
			return (SYSTIM_KEEP_ALIVE);
	}
	else
	{
		uint32_t utmp32 = 0x3FFFFFFF - btime;
		if((time + utmp32) >= period)
			return (SYSTIM_TIMEOUT);
		else
			return (SYSTIM_KEEP_ALIVE);
	}
}

uint32_t getElapsedTimeSYSTIM(uint32_t t_beg)
{
	uint32_t time = g_sysTimeCount;
    //uint32_t time;
	t_beg = t_beg;
	
	if(time >= t_beg)
	{
		return (time - t_beg);
	}
	else
	{
		return (time + (0x3FFFFFFF - t_beg));
	}
}
