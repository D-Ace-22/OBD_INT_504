/* 
 * File: ADC1.h
 * Author: Muhammad Sulaiman (GitHub: SulaimanNiazi)
 * Comments: This is the header file for ADC1 module configured to run during sleep.
 * Revision history: Created on March 20, 2025, Last modified on March 21, 2025.
*/

#ifndef __ADC1_H__
#define __ADC1_H__


    // Section: Parameters
    
    extern uint16_t ADC_Step;
    
    // Section: Interface Routines
    
    /* These functions are used to initialize the ADC1, Timer 3 and DMA0 modules
    and are called by the main function. */
    // External Functions
    
    /**
        @Summary
            Initializes ADC1.

        @Description
            This routine is used to configure A/D to convert channel 5 on Timer event.
            It generates event to DMA on every sample/convert sequence. ADC clock is configured at 625Khz.

        @Preconditions
            None

        @Param
            None

        @Returns
            None

        @Example 
            <code>    
                ADC1_Initialize();
            </code>
    */
    extern void ADC1_Initialize( void );
    
    /**
        @Summary
            Handles the ADC1 interrupt.

        @Description
            Clears the ADC1 interrupt flag.

        @Preconditions
            None

        @Param
            None

        @Returns
            None

        @Example 
            None
    */
    extern void __attribute__ ( (__interrupt__, no_auto_psv) )   _AD1Interrupt( void );
    
    /**
        @Summary
            Gets ADC1 result.

        @Description
            This routine returns the contents of ADC1BUF0 in integer format after dividing it with the voltage calibration.

        @Preconditions
            Initialize ADC1 (using ADC1_Initialize())

        @Param
            None

        @Returns
            float value of conversion.

        @Example 
            <code>    
                float volts = ADC1_Get_Voltage();
            </code>
    */
    extern float ADC1_Get_Voltage(void);
    
        /**
        @Summary
            Gets ADC1 Steps.

        @Description
            This routine returns the contents of ADC1BUF0 in unsigned integer format.

        @Preconditions
            Initialize ADC1 (using ADC1_Initialize())

        @Param
            None

        @Returns
            unsigned integer value of ADC steps.

        @Example 
            <code>    
                uint16_t steps = ADC1_Get_Steps();
            </code>
    */
    extern uint16_t ADC1_Get_Steps(void);
    
    /**
        @Summary
            Calibrates ADC1.

        @Description
            This routine sets the calibration value for the ADC1 module, subsequent voltage readings will be divided with it.

        @Preconditions
            None

        @Param
            unsigned integer value of ADC steps per volt.

        @Returns
            None

        @Example 
            <code>    
                ADC1_Calib(300);
            </code>
    */
    extern void ADC1_Calib(uint16_t);
    
    /**
        @Summary
            Enables ADC1.

        @Description
            This routine enables ADC1 for conversion, it only needs to be called once.

        @Preconditions
            None

        @Param
            None

        @Returns
            None

        @Example 
            <code>    
                ADC1_Enable();
            </code>
    */
    extern void ADC1_Enable(void);
    
    /**
        @Summary
            Starts ADC1.

        @Description
            This routine generates the start an ADC1 conversion.

        @Preconditions
            None

        @Param
            None

        @Returns
            Unsigned Integer value of number of steps.

        @Example 
            <code>    
                steps = ADC1_Get_Measurement();
            </code>
    */
    extern uint16_t ADC1_Get_Measurement(void);
    
    /**
        @Summary
            Starts ADC1 conversion

        @Description
            This routine generates the start of an ADC1 conversion but does not wait for it to complete.

        @Preconditions
            None

        @Param
            None

        @Returns
            None

        @Example 
            <code>    
                ADC1_Begin_Converting();
            </code>
    */
    extern void ADC1_Begin_Converting(void);

    #ifdef __cplusplus  // Provide C++ Compatibility
}
    #endif
#endif // __ADC1_H__
