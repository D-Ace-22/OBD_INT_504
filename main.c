/**
* @file        main.h
* @author      Aqib D. Ace 
* @date        24 April 2025 7:40 PM
* @version     0.4.15
*/

#include "mcc_generated_files/system.h"
#include "parameters.h"
char receivedCmd[60]={0}; //buffer for commands
//CommunicationInterface_t * GetCanInterface();

void delay_ms(unsigned int ms) 
{
    while (ms--) 
    {
        __delay32(20000000UL / 1000);
    }
}

int main(void)
{
    
    // initialize the device
    SYSTEM_Initialize();
    initNVM();
//    g_obdInfo.obdTransmissionTimeout = 50;
//    g_obdInfo.obdRequestPeriod = 0;
//    //setBaudrateCAN(CAN_BAUDRATE_HIGH_SPEED_500K);
//    setBaudrateCAN(500000);
//    configureFilterCAN(CAN_MODE_STANDARD_DATA_FRAME_8_DLC, g_obdInfo.canSidFilter, g_obdInfo.canEidFilter, g_obdInfo.canSidFilterMask, g_obdInfo.canEidFilterMask);
    UART1_Write('\r');
    UART1_Write_String(g_obdConfig.atiId); ///ATI string received on connection
    UART1_Write('\r');
    //restoreDefaultPParameters();
    //writeToNVM("STS@1","SCANTOOL.NET LLC");
    UART1_Write('>');
        
    while (1)
    {
        // wait for the command and process it
        UART1_ReadString(receivedCmd, 60);
        if (getLFStatus())
        {
            UART1_Write('\n'); 
        }
        processCommand(receivedCmd);
    }
    return 1;
}
