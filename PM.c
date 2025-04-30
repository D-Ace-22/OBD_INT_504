/*
 * File:        PM.c
 * Author:      Muhammad Sulaiman (GitHub: SulaimanNiazi)
 * Comments:    This is the source file for Power Management
 * Created on   March 8, 2025, 4:41 PM
 */

// Section of included header files

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <p33EP256GP506.h>
#include "xc.h"

#include "PM.h"
#include "mcc_generated_files/uart1.h"
#include "mcc_generated_files/tmr1.h"
#include "mcc_generated_files/adc1.h"
#include "parameters.h"
#include "commands.h"

// Section of functions

void PM_Initialize(){
    TRISAbits.TRISA8 = 0;   // Set RA8 (PWR_CTRL) pin as output.
    TRISCbits.TRISC2 = 1;   // Set RC2 (EXT SLEEP) pin as input
    CNENCbits.CNIEC2 = 1;   // Enable CN for RC2
    IEC1bits.CNIE = 1;      // Enable CN interrupts
    IFS1bits.CNIF = 0;      // Clear CN interrupt flag
    
    CNPUCbits.CNPUC2 = 1;   // Enable internal pull-up for RC2
    INTCON2bits.GIE = 1;    // Enable global interrupts
    
    TRISBbits.TRISB14 = 0;  // Set RB14 (LED_STATUS) pin as output
    PORTBbits.RB14 = 1;     // Set LED_STATUS pin as high (by default)
        
    strcpy(LAST_SLEEP_TRIG, "NONE");
    strcpy(LAST_WAKE_TRIG, "NONE");
    UART_SLEEP = EXT_INPUT = EXT_SLEEP = VL_SLEEP = VL_WAKE = VCHG_WAKE = STSLXP_VAL = UART_ALERT = false;
    UART_WAKE = EXT_WAKE = VL_WAKE_CONVERT = VCHG_CONVERT = VL_SLEEP_CONVERT = true;
    UART_SLEEP_TIM = 1200;
    UART_WAKE_TIM_L = 0;
    UART_WAKE_TIM_H = 30000;
    VL_SLEEP_TIM = 600;
    VL_WAKE_TIM = 1;
    VCHG_WAKE_TIM = 1000;
    EXT_SLEEP_TIM = 3000;
    EXT_WAKE_TIM = 2000;
    VL_SLEEP_VOLT = 13;
    VL_WAKE_VOLT = 13.2;
    VCHG_WAKE_VOLT = 0.2;
    VL_SLEEP_SYMBOL = '<';
    VL_WAKE_SYMBOL = '>';
    
    strcpy(CTRL_MODE, "NATIVE");
    EXT_WAKE_TIM = 0;
    PM_STSLPCP(false);
    
    //g_obdConfig.atppConfig[0x0E][0] = 0x5A; //default value
    
    uint8_t ELM327_config = g_obdConfig.atppConfig[0x0E][0];
    MASTER_EN = (ELM327_config & 0x7F) != 0;
            
    if(MASTER_EN){
        strcpy(CTRL_MODE, "ELM327");
        if((ELM327_config & 0x40) != 0){
            PM_STSLPCP(true);
        }
        if((ELM327_config & 0x20) == 0){
            UART_SLEEP = true;
        }
        if((ELM327_config & 0x10) != 0){
            UART_SLEEP_TIM = 20*60;
        }else{
            UART_SLEEP_TIM = 5*60;
        }
        if((ELM327_config & 0x04) != 0){
            UART_ALERT = true;
        }
        if((ELM327_config & 0x02) != 0){
            EXT_WAKE_TIM = 5000;
        }else{
            EXT_WAKE_TIM = 1000;
        }
    }
}

void PM_STSLCS(){
    char line[40] = "", subline[10] = "", symbols[10] = "";
    snprintf(line, 40, "CTRL MODE:  %s", CTRL_MODE);
    UART1_Write_String(line);
    snprintf(line, 40, "PWR_CTRL:   LOW POWER = %s", PWR_CTRL?"HIGH":"LOW");
    UART1_Write_String(line);
    snprintf(line, 40, "UART SLEEP: %s,  %d s", UART_SLEEP?"ON":"OFF", UART_SLEEP_TIM);
    UART1_Write_String(line);
    snprintf(line, 40, "UART WAKE:  %s,  %d-%d us", UART_WAKE?"ON":"OFF", UART_WAKE_TIM_L, UART_WAKE_TIM_H);
    UART1_Write_String(line);
    snprintf(line, 40, "EXT INPUT:  %s = SLEEP", EXT_INPUT?"HIGH":"LOW");
    UART1_Write_String(line);
    snprintf(line, 40, "EXT SLEEP:  %s, %s FOR %d ms", EXT_SLEEP? "ON": "OFF", STSLXP_VAL? "HIGH": "LOW", EXT_SLEEP_TIM);
    UART1_Write_String(line);
    snprintf(line, 40, "EXT WAKE:   %s, %s FOR %d ms", EXT_WAKE? "ON": "OFF", STSLXP_VAL? "LOW": "HIGH", EXT_WAKE_TIM);
    UART1_Write_String(line);
    symbols[0] = VL_SLEEP_SYMBOL;
    if(VL_SLEEP_CONVERT){
        snprintf(subline, 10, "%.2f", VL_SLEEP_VOLT);
        symbols[1] = PM_Is_Voltage_Valid(VL_SLEEP_VOLT)?'\0':'!';
    }else{
        snprintf(subline, 10, "%#X", VL_SLEEP_STEPS);
        symbols[1]='\0';
    }
    snprintf(line, 40, "VL SLEEP:   %s, %s%sV FOR %d s", VL_SLEEP?"ON":"OFF", symbols, subline, VL_SLEEP_TIM);
    UART1_Write_String(line);
    subline[0]='\0';
    symbols[0] = VL_WAKE_SYMBOL;
    if(VL_WAKE_CONVERT){
        snprintf(subline, 10, "%.2f", VL_WAKE_VOLT);
        symbols[1] = PM_Is_Voltage_Valid(VL_WAKE_VOLT)?'\0':'!';
    }else{
        snprintf(subline, 10, "%#X", VL_WAKE_STEPS);
        symbols[1]='\0';
    }
    snprintf(line, 40, "VL WAKE:    %s, %s%sV FOR %d s", VL_WAKE?"ON":"OFF", symbols, subline, VL_WAKE_TIM);
    UART1_Write_String(line);
    subline[0]='\0';
    uint8_t ind = 0;
    switch(VCHG_SYMBOL){
        case '+':
        case '-':
            symbols[ind++] = VCHG_SYMBOL;
            break;
        default:
            symbols[0]='\0';
    }
    if(VCHG_CONVERT){
        snprintf(subline, 10, "%.2f", VCHG_WAKE_VOLT);
        symbols[ind] = PM_Is_Voltage_Valid(VCHG_WAKE_VOLT)?'\0':'!';
        symbols[ind+1]='\0';
    }else{
        snprintf(subline, 10, "%#X", VCHG_WAKE_STEPS);
        symbols[ind] = '\0';
    }
    snprintf(line, 40, "VCHG WAKE:  %s, %s%sV IN %d ms", VCHG_WAKE?"ON":"OFF", symbols, subline, VCHG_WAKE_TIM);
    UART1_Write_String(line);
}

void PM_Manage_Power(){
    PORTBbits.RB14 = 1; //Set LED_STATUS pin as high
    
    if(VL_SLEEP){
        if(PM_Check_VL_SLEEP()){
            PM_Set_Sleep("VL", 0);
        }
    }
    
}

void PM_STSLLT(){
    char line[30];
    sprintf(line, "SLEEP: %s", LAST_SLEEP_TRIG);
    UART1_Write_String(line);
    sprintf(line, "WAKE:  %s", LAST_WAKE_TRIG);
    UART1_Write_String(line);
}

void PM_STSLU(bool sleep, bool wakeup){
    UART_WAKE = wakeup;
    if(MASTER_EN)return;
    UART_SLEEP = sleep;
}

void PM_STSLVG(bool trig){
    if(PM_Is_Voltage_Valid(VCHG_WAKE_VOLT) && trig){
        VCHG_WAKE = true;
        ADC1_Enable();
    }else{
        VCHG_WAKE = false;
    }
}

void PM_STSLVL(bool sleep, bool wakeup){
    VL_SLEEP = PM_Is_Voltage_Valid(VL_SLEEP_VOLT)? sleep: false;
    VL_WAKE = PM_Is_Voltage_Valid(VL_WAKE_VOLT)? wakeup: false;
    if(VL_SLEEP){
        ADC1_Enable();
    }
}

void PM_STSLUIT(uint16_t time){
    if(MASTER_EN)return;
    UART_SLEEP_TIM = time;
}

void PM_STSLPCP(bool lowPower){
    if(MASTER_EN)return;
    PWR_CTRL = lowPower;
    PORTAbits.RA8 = PWR_CTRL?1:0;
}

void PM_STSLUWP(uint16_t min, uint16_t max){
    UART_WAKE_TIM_L = min;
    UART_WAKE_TIM_H = max;
}

void PM_STSLXP(bool STSLXP){
    STSLXP_VAL = STSLXP;
}

void PM_STSLX(bool sleep, bool wake){
    EXT_SLEEP = sleep;
    EXT_WAKE = wake;
}

void PM_STSLVLS(char symbol, int steps, uint16_t time){
    VL_SLEEP_CONVERT = false;
    VL_SLEEP_SYMBOL = symbol;
    VL_SLEEP_STEPS = steps;
    VL_SLEEP_TIM = time;
}

void PM_STSLVLS_Set_Volt(float volt){
    VL_SLEEP_CONVERT = true;
    VL_SLEEP_VOLT = volt;
}

void PM_STSLVLW(char symbol, int steps, uint16_t time){
    VL_WAKE_SYMBOL = symbol;
    VL_WAKE_STEPS = steps;
    VL_WAKE_TIM = time;
}

void PM_STSLVLW_Set_Volt(float volt){
    VL_WAKE_VOLT = volt;
}

void PM_STSLVGW(char symbol, int steps, uint16_t time){
    uint16_t newTime = time/250;
    newTime*=250;
    VCHG_CONVERT = false;
    if((newTime + 250 - time < time - newTime)||(newTime == 0)){
        newTime = newTime+250;
    }
    VCHG_WAKE_STEPS = steps;
    VCHG_SYMBOL = symbol;
    VCHG_WAKE_TIM = newTime;
}

void PM_STSLVGW_Set_Volt(float volt){
    VCHG_CONVERT = true;
    VCHG_WAKE_VOLT = volt;
}

bool PM_Is_Voltage_Valid(float volt){
    return 0 <= volt < ((float)g_obdInfo.voltageCalibration)/1000;
}

bool PM_Get_Inactivity_trig(){
    return UART_SLEEP;
}

uint16_t PM_Get_Inactivity_time(){
    return UART_SLEEP_TIM;
}

void PM_Set_Sleep(char* cause, uint16_t delay){
    if(strcmp(cause, "CMD")==0 || strcmp(cause, "UART")==0 || strcmp(cause, "VL")==0 || strcmp(cause, "EXT")==0){
        if(delay > 0){
            TMR1_SoftwareCounterClear();
            TMR1_Start();
            while(TMR1_SoftwareCounterGet() < delay);
            TMR1_Stop();
        }
        while(!UART1_IsTxDone()); //Wait for any UART transmissions to complete
        PORTBbits.RB14 = 0; // Set LED_STATUS pin as low. 
        PM_Sleep();
        strcpy(LAST_SLEEP_TRIG, cause);
    }
}

void PM_Sleep(){
    TMR1_Start();
    if(UART_WAKE)CNENBbits.CNIEB6 = 1;              //set CNEN interrupt on RX pin if UART Wake trigger is enabled.
    if(VL_WAKE || VCHG_WAKE){
        ADC1_Enable();
        ADC1_Begin_Converting();   //begin an ADC conversion 
    }
    __builtin_pwrsav(0);
    TMR1_Stop();
    TMR1_SoftwareCounterClear();
}

void PM_Set_Wake_Trig(char* trig){
    if(strcmp(trig, "UART")==0 || strcmp(trig, "EXT")==0 || strcmp(trig, "VCHG")==0 || strcmp(trig, "VL")==0){
        strcpy(LAST_WAKE_TRIG, trig);
    }
    loadDefaultOnWarmReset(); //Perform ATWS reset
}

bool PM_Check_Reset_Recent_Sleep(){;
    if(RCONbits.SLEEP == 1){
        if(UART_WAKE && (UART_WAKE_TIM_L <= (64/g_obdInfo.uartBaudrate)*1000 <= UART_WAKE_TIM_H)){
            RCONbits.SLEEP = 0;
            return true;
        }
        else{
            __builtin_pwrsav(0);
        }
    }
    return false;
}

bool PM_Check_VL_SLEEP(void){
    int stepLevel = ADC1_Get_Measurement();
    float voltageLevel = ADC1_Get_Voltage();
    if(VL_SLEEP_CONVERT){
        if(VL_SLEEP_SYMBOL == '<'){
            return voltageLevel < VL_SLEEP_VOLT;
        }else{
            return voltageLevel > VL_SLEEP_VOLT;
        }
    }else{
        if(VL_SLEEP_SYMBOL == '<'){
            return stepLevel < VL_SLEEP_STEPS;
        }else{
            return stepLevel > VL_SLEEP_STEPS;
        }
    }
}

bool PM_Check_VL_WAKE(){
    int stepLevel = ADC1_Get_Measurement();
    float voltageLevel = ADC1_Get_Voltage();
    if(VL_WAKE_CONVERT){
        if(VL_WAKE_SYMBOL == '>'){
            return voltageLevel > VL_WAKE_VOLT;
        }else{
            return voltageLevel < VL_WAKE_VOLT;
        }
    }else{
        if(VL_WAKE_SYMBOL == '>'){
            return stepLevel > VL_WAKE_STEPS;
        }else{
            return stepLevel < VL_WAKE_STEPS;
        }
    }
}

bool PM_Check_VCHG(void){
    int initialSteps = ADC1_Get_Measurement(), finalSteps, stepDiff;
    float initialVoltage = ADC1_Get_Voltage(), finalVoltage, voltDiff;
    
    TMR1_Start();
    while(TMR1_SoftwareCounterGet() < VCHG_WAKE_TIM);
    TMR1_Stop();
    TMR1_SoftwareCounterClear();
    
    finalSteps = ADC1_Get_Measurement();
    finalVoltage = ADC1_Get_Voltage();
    
    stepDiff = finalSteps - initialSteps;
    voltDiff = finalVoltage - initialVoltage;
    
    if(VCHG_CONVERT){
        switch(VCHG_SYMBOL){
            case '+':
                return (voltDiff >= VCHG_WAKE_VOLT);
                break;
            case '-':
                return (voltDiff <= VCHG_WAKE_VOLT);
                break;
            case '=':
                return ((voltDiff >= VCHG_WAKE_VOLT)||(voltDiff <= VCHG_WAKE_VOLT));
                break;
        }
    }else{
        switch(VCHG_SYMBOL){
            case '+':
                return (stepDiff >= VCHG_WAKE_STEPS);
                break;
            case '-':
                return (stepDiff <= VCHG_WAKE_STEPS);
                break;
            case '=':
                return ((stepDiff >= VCHG_WAKE_STEPS)||(stepDiff <= VCHG_WAKE_STEPS));
                break;
        }
    }
    return false;
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void)
{
    // Clear CN interrupt flag
    IFS1bits.CNIF = 0;

    // Check the state of RC2
    if(EXT_SLEEP){
        while(PORTCbits.RC2 == 0){
            TMR1_Start();
            if(TMR1_SoftwareCounterGet()>=EXT_SLEEP_TIM){
                TMR1_Stop();
                PM_Set_Sleep("EXT", 0);
            }
        }
        TMR1_Stop();
        TMR1_SoftwareCounterClear();
    }
    
    if(PM_Check_Reset_Recent_Sleep()){
        if(EXT_WAKE && PORTCbits.RC2 == 1){
            while(PORTCbits.RC2 == 1){
                TMR1_Start();
                if(TMR1_SoftwareCounterGet()>=EXT_WAKE_TIM){
                    TMR1_Stop();
                    PM_Set_Wake_Trig("EXT");
                    UART1_Write('>');
                }
            }
            TMR1_Stop();
            TMR1_SoftwareCounterClear();
        }
        else if(PORTBbits.RB6 == 1 && UART_WAKE){
            PM_Set_Wake_Trig("UART");
            CNENBbits.CNIEB6 = 0;
        }else{
            PM_Sleep();
        }
    }
}
