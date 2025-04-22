/*
 * File:        ADC1.c
 * Author:      Muhammad Sulaiman (GitHub: SulaimanNiazi)
 * Comments:    This is the source file for ADC1 module.
 * Created on   March 20, 2025
*/

#include <xc.h>
#include <stdint.h>
#include "ADC1.h"
#include "../PM.h"

void ADC1_Initialize ( void )
{
    TRISAbits.TRISA0 = 1;           // Set RA0 pin as input

    AD1CON1bits.FORM = 0;           // Data Output Format: Unsigned Integer Decimal Value
    AD1CON1bits.SSRC = 0;           // Clear SAMP bit ends sampling and starts conversion
    AD1CON1bits.SSRCG = 0;
    AD1CON1bits.ASAM = 1;           // ADC Sample Control: Sampling begins immediately after conversion
    AD1CON1bits.AD12B = 0;          // 10-bit ADC operation
    AD1CON2bits.CHPS = 0;           // Converts CH0

    // Set up A/D conversion clock to ADC Internal RC so that it will work in SLEEP mode
    AD1CON3 = ( AD1CON3 | 0x8000 );
    AD1CON3bits.SAMC = 1;           // Auto Sample Time = 1*Tad
    AD1CON3bits.ADCS = 0;           // ADC Conversion Clock Tad=Trc
    AD1CON2bits.SMPI = 0;           // SMPI must be 0

    //AD1CHS0: A/D Input Select Register
    AD1CHS0bits.CH0SA = 0;          // MUXA +ve input selection (AIN5) for CH0
    AD1CHS0bits.CH0NA = 0;          // MUXA -ve input selection (Vref-) for CH0

    //All inputs are analog by default
    ANSELBbits.ANSB0 = 1;           // AN0 as Analog Input
    IFS0bits.AD1IF = 0;             // Clear the A/D interrupt flag bit
    IEC0bits.AD1IE = 0;             // Enable A/D interrupt
    AD1CON1bits.ADON = 1;           // Turn on the A/D converter
    
    AD1CON3bits.ADRC = 1;           // ADC clock source is internal RC oscillator
    AD1CON1bits.SSRC = 0b111;       // Internal counter ends sampling and starts conversion (auto-convert)
    AD1CON1bits.ADSIDL = 0;         // ADC operates in Idle mode
    
    ADC_Step = 315;                 // Set default Calibration value (Steps per Volt)
}

void ADC1_Calib(uint16_t value){
    ADC_Step = value;
}

float ADC1_Get_Voltage(void){
    return (float)((uint16_t)ADC1BUF0/ADC_Step);
}

uint16_t ADC1_Get_Steps(void){
    return (uint16_t)ADC1BUF0;
}

void ADC1_Enable(void){
    AD1CON1bits.SAMP = 1;
    for(uint16_t i = 0; i < 50; i++ );
}

void ADC1_Begin_Converting(void){
    AD1CON1bits.SAMP = 0;           // Generate Start of Conversion   
}

uint16_t ADC1_Get_Measurement(void){
    ADC1_Begin_Converting();
    while(!AD1CON1bits.DONE);
    return ADC1_Get_Steps();
}

void __attribute__ ( (interrupt, no_auto_psv) ) _AD1Interrupt( void )
{
    IFS0bits.AD1IF = 0;             // Clear the ADC1 Interrupt Flag
    if(PM_Check_Reset_Recent_Sleep()){
        if(VL_WAKE){
            if(PM_Check_VL_WAKE){
                PM_Set_Wake_Trig("VL");
                return;
            }else{
                PM_Sleep();
            }
        }
        if(VCHG_WAKE){
            if(PM_Check_VCHG()){
                PM_Set_Wake_Trig("VCHG");
                return;
            }else{
                PM_Sleep();
            }
        }
        PM_Sleep();
    }
}
