/**
* @file        nvmpic.c
* @author      Aqib D. Ace 
* @date        March 2025
* @version     0.0.0
*/

#include "nvmpic.h"
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/uart1.h"


void delay2_ms(uint16_t ms) {
    for (uint16_t i = 0; i < ms; i++) {
        for (uint16_t j = 0; j < 180; j++) {
            asm("nop"); // No-operation instruction
        }
    }
}
uint8_t read_PPdata[250];  //buffer holds the data after raed operation.
uint32_t flash_storage_address; //variable for storing NVM starting address
uint32_t flash_storage_address2; //variable for storing NVM starting address
bool result;
char testbuf[50];
uint32_t write_data[100]; //buffer that holde the data in 24bit word form before writing to NVM memory
const uint8_t c_obdConfigPreamble[9] = {0x49, 0x52, 0x51, 0x64, 0x65, 0x73, 0x69, 0x67, 0x6E};
// Allocate and reserve a page of flash for this test to use.  The compiler/linker will reserve this for data and not place any code here.
static __prog__  uint8_t flashTestPage[FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS] __attribute__((space(prog),aligned(FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS)));
static __prog__  uint8_t flashTestPage2[FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS] __attribute__((space(prog),aligned(FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS)));
// We have detected a flash hardware error of some type.
static void FlashError()
{
    while (1) 
    { }
}

static void MiscompareError()
{
    while (1) 
    { }
}
void getAddress(void)
{
    flash_storage_address = FLASH_GetErasePageAddress((uint32_t)&flashTestPage[0]);
    flash_storage_address2 = FLASH_GetErasePageAddress((uint32_t)&flashTestPage2[0]);
}
void restoreDefaultPParameters(void)
{
    // Get flash page aligned address of flash reserved above for this test.
    getAddress();
   
    FLASH_Unlock(FLASH_UNLOCK_KEY);

    result = FLASH_ErasePage(flash_storage_address);
    if (result == false)
    {
        FlashError();
    }
    
    // Fill first 4 flash words with data
    // For this product we must write two adjacent words at a one time.
   
    // For this product we must write two adjacent words at a one time.
    for (int i = 0 ; i < 16; i++)
    {
        write_data[i*2]     = (((uint32_t)c_def_programmable_parameters[(i*3)+0][0]<<16) | ((uint32_t)c_def_programmable_parameters[(i*3)+0][1]<<8) | (c_def_programmable_parameters[(i*3)+1][0])); //0x00102030;
        write_data[(i*2)+1] = (((uint32_t)c_def_programmable_parameters[(i*3)+1][1]<<16) | ((uint32_t)c_def_programmable_parameters[(i*3)+2][0]<<8) | (c_def_programmable_parameters[(i*3)+2][1])); //0x00405060;
    
        //result  = 
        FLASH_WriteDoubleWord24(flash_storage_address + (i*4),  write_data[i*2], write_data[(i*2)+1]);
    }
   //if (result == false)
    //{
   //     FlashError();
    //}

    // Clear Key for NVM Commands so accidental call to flash routines will not corrupt flash
    FLASH_Lock();
}
void writeToNVM(char* command, char* argument)
{
    // Get flash page aligned address of flash reserved above for this test.
    getAddress();
   
    FLASH_Unlock(FLASH_UNLOCK_KEY);

    if (strcmp(command, "STWBR") == 0)
    {
        for (int i = 0 ; i < 2; i++)//4 words
        {
            write_data[i*2]     = (((uint32_t)argument[(i*6)+0]<<16) | ((uint32_t)argument[(i*6)+1]<<8) | (argument[(i*6)+2])); //0x00102030;
            write_data[(i*2)+1] = (((uint32_t)argument[(i*6)+3]<<16) | ((uint32_t)argument[(i*6)+4]<<8) | (argument[(i*6)+5])); //0x00405060; 
            
            FLASH_WriteDoubleWord24(flash_storage_address + (i*4) + 68,  write_data[i*2], write_data[(i*2)+1]);
        }
    }
    if (strcmp(command, "STSATI") == 0)
    {
        for (int i = 0 ; i < 6; i++)//12 words
        {
            write_data[i*2]     = (((uint32_t)argument[(i*6)+0]<<16) | ((uint32_t)argument[(i*6)+1]<<8) | (argument[(i*6)+2])); //0x00102030;
            write_data[(i*2)+1] = (((uint32_t)argument[(i*6)+3]<<16) | ((uint32_t)argument[(i*6)+4]<<8) | (argument[(i*6)+5])); //0x00405060; 
            
            FLASH_WriteDoubleWord24(flash_storage_address + (i*4) + 76,  write_data[i*2], write_data[(i*2)+1]);
        }
    }
    if (strcmp(command, "STSDI") == 0)
    {
        for (int i = 0 ; i < 8; i++)//16 words
        {
            write_data[i*2]     = (((uint32_t)argument[(i*6)+0]<<16) | ((uint32_t)argument[(i*6)+1]<<8) | (argument[(i*6)+2])); //0x00102030;
            write_data[(i*2)+1] = (((uint32_t)argument[(i*6)+3]<<16) | ((uint32_t)argument[(i*6)+4]<<8) | (argument[(i*6)+5])); //0x00405060; 
            
            FLASH_WriteDoubleWord24(flash_storage_address + (i*4) + 100,  write_data[i*2], write_data[(i*2)+1]);
        }
    }
    if (strcmp(command, "STS@1") == 0)
    {
        for (int i = 0 ; i < 8; i++)//16 words
        {
            write_data[i*2]     = (((uint32_t)argument[(i*6)+0]<<16) | ((uint32_t)argument[(i*6)+1]<<8) | (argument[(i*6)+2])); //0x00102030;
            write_data[(i*2)+1] = (((uint32_t)argument[(i*6)+3]<<16) | ((uint32_t)argument[(i*6)+4]<<8) | (argument[(i*6)+5])); //0x00405060; 
            
            FLASH_WriteDoubleWord24(flash_storage_address + (i*4) + 132,  write_data[i*2], write_data[(i*2)+1]);
        }
    }
    if (strcmp(command, "AT@3") == 0)
    {
        for (int i = 0 ; i < 2; i++)//12 bytes 4 words
        {
            write_data[i*2]     = (((uint32_t)argument[(i*6)+0]<<16) | ((uint32_t)argument[(i*6)+1]<<8) | (argument[(i*6)+2])); //0x00102030;
            write_data[(i*2)+1] = (((uint32_t)argument[(i*6)+3]<<16) | ((uint32_t)argument[(i*6)+4]<<8) | (argument[(i*6)+5])); //0x00405060; 
            
            FLASH_WriteDoubleWord24(flash_storage_address + (i*4) + 164,  write_data[i*2], write_data[(i*2)+1]);
        }
    }
    
    FLASH_Lock();
}
void printPParameters(void)
{
    char TXbuf[30];
    for (int i = 0 ; i < OBD_ATPP_CONFIG_SIZE ;i++)
    {
        sprintf(TXbuf,"%02X:%02X %c " ,i, g_obdConfig.atppConfig[i][0],(g_obdConfig.atppConfig[i][1] == 0 ? 'N' : 'F')) ;
        delay2_ms(10);
        UART1_Write_String(TXbuf);   
        if ((i+1)%4==0 && i!=47)
        {
            UART1_Write('\r');
            if(getLFStatus())
            {
                UART1_Write('\n');
            }
        
        }
    }
    
    // Stop if the read data does not match the write data;
//    if ( (write_data[0] != read_data[0]) ||
//         (write_data[1] != read_data[1]) ||
//         (write_data[2] != read_data[2]) ||
//         (write_data[3] != read_data[3]) )
//    {
//        MiscompareError();    
//    }
}

void initNVM(void)
{
    loadDefaultConfigurationOBD();
    loadCustomConfigurationOBD(PROGRAMMABLE_PARAMETERS_TYPE_P);
    loadOtpConfigurationOBD(); //TODO:

    g_obdInfo.calibrationFlag = 0xff;
    if (g_obdOtpConfig.calibrationFlag != 0xff) 
    {
        g_obdInfo.voltageCalibration = g_obdOtpConfig.voltageCalibration;
        g_obdInfo.voltageOffset      = g_obdOtpConfig.voltageOffset;
        g_obdInfo.adcCalibration     = g_obdOtpConfig.adcCalibration;
    } 
    else 
    {
        g_obdInfo.voltageCalibration = 12345;
        g_obdInfo.voltageOffset = 670;
        g_obdInfo.adcCalibration = 2115;
    }
    //setObdProtocol(&g_obdInfo, &g_obdConfig, g_obdConfig.protocol);
}
//configute the PPs seperatelly using ATPP
void configurePPs(char* argument)
{
    uint8_t srNo,xx,yy;
    if ( strstr(argument, "ON") != NULL )
    {
        srNo = hexDigitsToDecimal(argument[0],argument[1]);
        if (srNo == -1)
        {
           UART1_Write('?');
        }
        else
        {
           g_obdConfig.atppConfig[srNo][1] = 0x00;
           UART1_Write_String("OK");
           updateCustomConfigurationOBD(); 
        }   
    }
    else if(strstr(argument, "OFF") != NULL)
    {
        srNo = hexDigitsToDecimal(argument[0],argument[1]);
        if (srNo == -1)
        {
            UART1_Write('?');
        }
        else
        {
           g_obdConfig.atppConfig[srNo][1] = 0xFF;
           UART1_Write_String("OK");
           updateCustomConfigurationOBD(); 
        }
    }
    else if(strstr(argument, "SV") != NULL)
    {
        xx = hexDigitsToDecimal(argument[0],argument[1]);
        yy = hexDigitsToDecimal(argument[4],argument[5]);
        if (xx == -1 || yy == -1)
        {
            UART1_Write('?');
        }
        else
        {
            uint8_t value = yy & 0xff;
            switch (xx) 
            {
                    // 0,ff
                case(0x00):
                case(0x01):
                case(0x02):
                case(0x09):
                case(0x11):
                case(0x12):
                case(0x16):
                case(0x21):
                case(0x24):
                case(0x25):
                case(0x29):
                {
                    if ((value == 0xff) || (value == 0x00)) {
                        g_obdConfig.atppConfig[xx][0] = value;
                        UART1_Write_String("OK");
                    } else {
                        UART1_Write('?');
                    }
                    break;
                }
                case(0x03):
                case(0x06):
                case(0x0A):
                case(0x0D):
                case(0x0E):
                case(0x10):
                case(0x13):
                case(0x15):
                case(0x17):
                case(0x18):
                case(0x19):
                case(0x1A):
                case(0x1B):
                case(0x26):
                case(0x2A):
                case(0x2C):
                case(0x2E):
                {
                    g_obdConfig.atppConfig[xx][0] = value;
                    UART1_Write_String("OK");
                    break;
                }
                case(0x07):
                {
                    if ((value >= 1) && (value <= 0x0C)) 
                    {
                        g_obdConfig.atppConfig[xx][0] = value;
                        UART1_Write_String("OK");
                    } 
                    else 
                    {
                        UART1_Write('?');
                    }
                    break;
                }
                case(0x04):
                {
                    if ((value >= 0) && (value <= 2)) {
                        g_obdConfig.atppConfig[xx][0] = value;
                        UART1_Write_String("OK");
                    } else {
                        UART1_Write('?');
                    }
                    break;
                }
                case(0x0C):
                {
                    if ((value >= 0x06) || (value == 0x02) || (value == 0x04)) {
                        g_obdConfig.atppConfig[xx][0] = value;
                        UART1_Write_String("OK");
                    } else {
                        UART1_Write('?');
                    }
                    break;

                }
                case(0x2B):
                case(0x2D):
                case(0x2F):
                {
                    if ((value >= 1) && (value <= 20)) {
                        g_obdConfig.atppConfig[xx][0] = value;
                        UART1_Write_String("OK");
                    } else {
                        UART1_Write('?');
                    }
                    break;
                }
                default:
                {
                    UART1_Write('?');
                }
            }
            
            updateCustomConfigurationOBD();
        }  
    }
    else
        {
            UART1_Write('?');
        }
}

void printAllSaved(void)
{
    getAddress();
    // read the flash words to verify the data
    for (int i = 0; i < 52 ; i++)
    {
        read_PPdata[(i*3)+0] = FLASH_ReadWord24(flash_storage_address + (i*2) + 68)>>16 & 0xFF;
        read_PPdata[(i*3)+1] = FLASH_ReadWord24(flash_storage_address + (i*2) + 68)>>8  & 0xFF;
        read_PPdata[(i*3)+2] = FLASH_ReadWord24(flash_storage_address + (i*2) + 68)     & 0xFF;
    }
    char TXbuf[30];
    UART1_Write_String("\r\nUART1 baudrate: ");
    for (int i = 0 ; i<12 ;i++) //4 words 12 bytes
    {
        TXbuf[i]=read_PPdata[i];
    }
    UART1_Write_String(TXbuf); 
    UART1_Write_String("\r\nSaved ATI device ID String: ");
    for (int i = 12 ; i<48 ;i++)//12 words 36 bytes
    {
        TXbuf[i-12]=read_PPdata[i];
    }
    UART1_Write_String(TXbuf);
    UART1_Write_String("\r\nSaved Hardware ID device ID String: ");
    for (int i = 48 ; i<96 ;i++)//16 words 48 bytes
    {
        TXbuf[i-48]=read_PPdata[i];
    }
    UART1_Write_String(TXbuf);
    UART1_Write_String("\r\nSaved STS@1 description device ID String: ");
    for (int i = 96 ; i<144 ;i++)//16 words 48 bytes
    {
        TXbuf[i-96]=read_PPdata[i];
    }
    UART1_Write_String(TXbuf);
    UART1_Write_String("\r\nSaved AT@3 description device ID String: ");
    for (int i = 144 ; i<156 ;i++)//16 words 48 bytes
    {
        TXbuf[i-144]=read_PPdata[i];
    }
    UART1_Write_String(TXbuf);
    
}
void printSaved(char* command)
{
    getAddress();
    // read the flash words to verify the data
    for (int i = 0; i < 80 ; i++)
    {
        read_PPdata[(i*3)+0] = FLASH_ReadWord24(flash_storage_address + (i*2))>>16 & 0xFF;
        read_PPdata[(i*3)+1] = FLASH_ReadWord24(flash_storage_address + (i*2))>>8  & 0xFF;
        read_PPdata[(i*3)+2] = FLASH_ReadWord24(flash_storage_address + (i*2))     & 0xFF;
    }
    char TXbuf[50];
    if (1)
    {
        for (int k = 0; k < 240; k++) 
        {
            sprintf(TXbuf, "\r\nByte %d: %02X", k, read_PPdata[k]);
            UART1_Write_String(TXbuf);
        }
    }
    else if(strcmp(command,"baudrate") == 0)
    {
        for (int i = 0 ; i<12 ;i++) //4 words 12 bytes
        {
            TXbuf[i]=read_PPdata[i];
        }
        UART1_Write_String(TXbuf);
    }
    if(strcmp(command,"ATI") == 0)
    {
        for (int i = 12 ; i<48 ;i++)//12 words 36 bytes
        {
            TXbuf[i-12]=read_PPdata[i];
        }
        UART1_Write_String(TXbuf);
    }
    if(strcmp(command,"STSDI") == 0)
    {
        for (int i = 48 ; i<96 ;i++)//16 words 48 bytes
        {
            TXbuf[i-48]=read_PPdata[i];
        }
        UART1_Write_String(TXbuf);
    }
    if(strcmp(command,"STS@1") == 0)
    {
        for (int i = 96 ; i<144 ;i++)//16 words 48 bytes
        {
            TXbuf[i-96]=read_PPdata[i];
        }
        UART1_Write_String(TXbuf);
    }
    if(strcmp(command,"AT@3") == 0)
    {
        for (int i = 144 ; i<156 ;i++)//16 words 48 bytes
        {
            TXbuf[i-144]=read_PPdata[i];
        }
        UART1_Write_String(TXbuf);
    }
}
//uint32_t returnread(int i)
//{
    
//   return (uint32_t)read_data[i];
//}
    
static void PageWritexample()
{
    uint32_t flash_storage_address,flashOffset;
    bool result;
    uint32_t readData;

    // Get flash page aligned address of flash reserved above for this test.
    flash_storage_address = FLASH_GetErasePageAddress((uint32_t)&flashTestPage[0]);


    FLASH_Unlock(FLASH_UNLOCK_KEY);
    
    // ------------------------------------------
    // Fill a page of memory with data.  
    // ------------------------------------------
    
    // Erase the page of flash at this address
    result = FLASH_ErasePage(flash_storage_address);
    if (result == false)
    {
        FlashError();
    }
  
    // Program flash with a data pattern.  For the data pattern we will use the index 
    // into the flash as the data.
    for (flashOffset= 0U; flashOffset< FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS; flashOffset += 4U)
    {
        result = FLASH_WriteDoubleWord24(flash_storage_address+flashOffset, flashOffset, flashOffset+2U);
        if (result == false)
        {
            FlashError();
        }   
    }

    // Clear Key for NVM Commands so accidental call to flash routines will not corrupt flash
    FLASH_Lock();

    
    // Verify the flash data is correct.  If it's not branch to error loop.
    // The data in the flash is the offset into the flash page.
    for (flashOffset= 0U; flashOffset< FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS; flashOffset += 2U)
    {
        readData = FLASH_ReadWord24(flash_storage_address+flashOffset);
        if (readData != flashOffset )
        {
            MiscompareError();
        }   
    }

}


void FlashDemo()
{
    //WordWriteExample();
    //PageWritexample();
}

uint8_t hexDigitToDecimal(char hex) 
{
    if (hex >= '0' && hex <= '9') {
        return hex - '0';  // Convert '0'-'9' to 0-9
    } else if (hex >= 'A' && hex <= 'F') {
        return hex - 'A' + 10;  // Convert 'A'-'F' to 10-15
    } else if (hex >= 'a' && hex <= 'f') {
        return hex - 'a' + 10;  // Convert 'a'-'f' to 10-15
    } else {
        return -1;  // Invalid input
    }
}

uint8_t hexDigitsToDecimal(char x, char y) 
{
    int high = hexDigitToDecimal(x);
    int low  = hexDigitToDecimal(y);

    if (high == -1 || low == -1) {
        return -1;  // Error: Invalid hex digit
    }

    return (high * 16) + low;  // Convert hex to decimal
}

uint8_t updateCustomConfigurationOBD(void) 
{
    getAddress();
   
    FLASH_Unlock(FLASH_UNLOCK_KEY);

    result = FLASH_ErasePage(flash_storage_address);
    if (result == false)
    {
        FlashError();
    }
    ////
    uint16_t k = 0;
    uint16_t n = 0;
    uint16_t size = sizeof(obdConfig_t);  // size = 234
//    sprintf(testbuf,"\r\n size of obdconfg: %x",size);
//    UART1_Write_String(testbuf);
    uint8_t *ptr = (uint8_t *)&g_obdConfig;
//    sprintf(testbuf, "\r\nAddress of g_obdConfig: %p", (void *)ptr);
//    UART1_Write_String(testbuf);
    uint8_t data[234] = {0x00};
    for (k = 0; k < size; k++) 
    {
//        if (((k % 3) == 0) && (k != 0)) {
//            data[n++] = 0x00;
//        }
        data[n++] = ptr[k];
    }
//    for (k = 0; k < size; k++) 
//    {
//        sprintf(testbuf, "\r\nByte %d: %02X", k, data[k]);
//        UART1_Write_String(testbuf);
//    }
//    sprintf(testbuf, "\r\nN: %u", n);
//    UART1_Write_String(testbuf);
    uint16_t residue = n % 16;
    if (residue) 
    {
        n = n + (16 - residue);
    }
//    sprintf(testbuf, "\r\nn: %u", n);
//   UART1_Write_String(testbuf);
    //printDEBUG(DSYS, "Write Cfg Size[%d]\n", size);
    for (int i = 0 ; i < n/6; i++)
    {
        write_data[i*2]     = (((uint32_t)data[(i*6)+0]<<16) | ((uint32_t)data[(i*6)+1]<<8) | (data[(i*6)+2])); //0x00102030;
        write_data[(i*2)+1] = (((uint32_t)data[(i*6)+3]<<16) | ((uint32_t)data[(i*6)+4]<<8) | (data[(i*6)+5])); //0x00405060;
        //result  = 
        FLASH_WriteDoubleWord24(flash_storage_address + (i*4),  write_data[i*2], write_data[(i*2)+1]);
    }

    FLASH_Lock();
    return 0;
}

uint8_t loadCustomConfigurationOBD(uint8_t reason) {
    uint16_t k = 0;
    getAddress();
    if (reason == PROGRAMMABLE_PARAMETERS_TYPE_P) 
    {
        uint32_t baddr = flash_storage_address;
        uint32_t tmp = 0;
        uint16_t size = sizeof (obdConfig_t);
        //size += (size / 4);
        uint16_t residue = size % 16;
        if (residue) {
            size = size + (16 - residue);
        }
        uint8_t *ptr = (uint8_t *) &g_obdConfig;
        uint16_t n = 0;
        baddr = flash_storage_address;
        for (k = 0; k < size/3; k++) {
            tmp = FLASH_ReadWord24(baddr);
            ptr[(k*3)+0] = (tmp >> 16) & 0xff;

            ptr[(k*3)+1] = (tmp >> 8) & 0xff;

            ptr[(k*3)+2] = (tmp) & 0xff;

            baddr += 2;
        }
        //////*****testing code*****//////
//        uint8_t data[234] = {0x00};
//        for (k = 0; k < size; k++) 
//        {
//            data[k] = ptr[k];
//        }
//        for (k = 0; k < size; k++) 
//        {
//            sprintf(testbuf, "\r\nBytee %d: %02X", k, data[k]);
//            UART1_Write_String(testbuf);
//        }
        
        uint8_t flag = 0;

        //printDEBUG(0, "\n=>PREAMPLE \n");
        for (k = 0; k < 9; k++) 
        {
            //printDEBUG(0, "\n=>PREAMPLE %xb %xb\n", c_obdConfigPreamble[k], g_obdConfig.preamble[k]);
            if (c_obdConfigPreamble[k] != g_obdConfig.preamble[k]) 
            {
                flag = 1;
                break;
            }
        }
        
        
        if (flag != 0x00) {
            //printDEBUG(DOBD | DWARNING, "Restore custom configuration to default\n");
            restoreCustomConfigurationOBD();
        }
    }
    for (k = 0; k < OBD_ATPP_CONFIG_SIZE; k++) {
        switch (k) { // TODO implement
            case(0x00): // Perform ATMA after power up or reset
            {
                break;
            }
            case(0x01): // Printing of header bytes (ATH default setting)
            {
                g_obdInfo.formatProperties.ath = ((g_obdConfig.atppConfig[k][0] == 0xff) & (g_obdConfig.atppConfig[k][1] != 0)) ? 1 : 0;
                break;
            }
            case(0x02):
            {

                break;
            }
            case(0x03):
            {
                g_obdInfo.obdRequestTimeout = g_obdConfig.atppConfig[k][0] * 4.096;
                //printDEBUG(DSYS | DWARNING, "Set OBD request timeout [%d]\n", g_obdInfo.obdRequestTimeout);
                break;
            }
            case(0x04):
            {

                break;
            }
            case(0x06):
            {

                break;
            }
            case(0x07):
            {

                break;
            }
            case(0x08):
            {

                break;
            }
            case(0x09):
            {

                break;
            }
            case(0x0A):
            {

                break;
            }
            case(0x0B):
            {

                break;
            }
            case(0x0C):
            {
                if (reason <= c_def_programmable_parameters[k][2]) {
                    if (g_obdConfig.atppConfig[k][1] == 0x00) {
                        uint32_t baudrate = 4000000;
                        baudrate = baudrate / g_obdConfig.atppConfig[k][0];
                        g_obdInfo.uartBaudrate = baudrate;
                        UART1_Initialize(g_obdInfo.uartBaudrate);
                        //printDEBUG(DSYS | DWARNING, "Set baudrate to [%dw]\n", baudrate);
                    } else {
                        g_obdInfo.uartBaudrate = g_obdConfig.usartBaudrate;
                        UART1_Initialize(g_obdInfo.uartBaudrate);
                        //printDEBUG(DSYS | DWARNING, "Set baudrate to [%dw]\n", g_obdInfo.uartBaudrate);
                    }
                }
                break;
            }
            case(0x0D):
            {

                break;
            }
            case(0x0E):
            {

                break;
            }
            case(0x0F):
            {

                break;
            }
            case(0x10):
            {

                break;
            }
            case(0x11):
            {

                break;
            }
            case(0x12):
            {

                break;
            }
            case(0x13):
            {

                break;
            }
            case(0x14):
            {

                break;
            }
            case(0x15):
            {

                break;
            }
            case(0x16):
            {

                break;
            }
            case(0x17):
            {

                break;
            }
            case(0x18):
            {

                break;
            }
            case(0x19):
            {

                break;
            }
            case(0x1A):
            {

                break;
            }
            case(0x1B):
            {

                break;
            }
            case(0x1C):
            {

                break;
            }
            case(0x1D):
            {

                break;
            }
            case(0x1E):
            {

                break;
            }
            case(0x1F):
            {

                break;
            }
            case(0x20):
            {

                break;
            }
            case(0x21):
            {
                g_obdInfo.obdCanMonitoringMode = g_obdConfig.atppConfig[k][0];
                break;
            }
            case(0x22):
            {
                break;
            }
            case(0x23):
            {
                break;
            }
            case(0x24):
            {

                break;
            }
            case(0x25):
            {

                break;
            }
            case(0x26):
            {

                break;
            }
            case(0x27):
            {

                break;
            }
            case(0x28):
            {

                break;
            }
            case(0x29):
            {

                break;
            }
            case(0x2A):
            {

                break;
            }
            case(0x2B):
            {

                break;
            }
            case(0x2C):
            {
                g_obdInfo.protocolbSettings = g_obdConfig.atppConfig[k][0];
                break;
            }
            case(0x2D):
            {
                g_obdInfo.protocolbBaudrate = g_obdConfig.atppConfig[k][0];
                break;
            }
            case(0x2E):
            {

                break;
            }
            case(0x2F):
            {

                break;
            }
            default:
            {

            }

        }
    }
    return 0;
}

uint8_t restoreCustomConfigurationOBD(void) {
    uint8_t k;
    for (k = 0; k < 9; k++) {
       g_obdConfig.preamble[k] = c_obdConfigPreamble[k];
    }
    g_obdConfig.usartBaudrate = DEFAULT_UART_BAUDRATE;


    g_obdConfig.wakeUartMinPulse = 0; // in ms = 20 minutes (20*60*1000)
    g_obdConfig.wakeUartMaxPulse = 30000; // in ms = 20 minutes (20*60*1000)

    g_obdConfig.sleepUartFlags = OBD_SLEEP_CONFIG_FLAGS_UART_WAKE_BIT | OBD_SLEEP_CONFIG_FLAGS_UART_INACTIVITY_BIT;
    //g_obdConfig.sleepUartFlags = OBD_SLEEP_CONFIG_FLAGS_UART_WAKE_BIT ;
    //    g_obdConfig.sleepPeriod = 1200000; // in ms = 20 minutes (20*60*1000)
    g_obdConfig.sleepPeriod = 600l*1000l; // in ms = 20 minutes (20*60*1000)
    g_obdConfig.sleepVoltage.sign = '<';
    g_obdConfig.sleepVoltage.isActive = 0;
    g_obdConfig.sleepVoltage.triggerLevel = 13.0;
    g_obdConfig.sleepVoltage.triggerTime = 600;

    g_obdConfig.wakeVoltage.sign = '>';
    g_obdConfig.wakeVoltage.isActive = 1;
    g_obdConfig.wakeVoltage.triggerLevel = 13.0;
    g_obdConfig.wakeVoltage.triggerTime = 2;

    g_obdConfig.levelChangeVoltage.sign = '+';
    g_obdConfig.levelChangeVoltage.isActive = 1;
    g_obdConfig.levelChangeVoltage.triggerLevel = 0.75;
    g_obdConfig.levelChangeVoltage.triggerTime = 750;
    strcpy(g_obdConfig.descriptionString, "SCANTOOL.NET LLC");
    strcpy(g_obdConfig.atiId, ELM327_VERSION_ID);
    g_obdOtpConfig.hardwareIdFlag = 0xff;
    
    restoreDefaultProgrammableParametersOBD();
    updateCustomConfigurationOBD();
    return 0;
}
void restoreDefaultProgrammableParametersOBD(void) {
    uint16_t k = 0;
    for (k = 0; k < OBD_ATPP_CONFIG_SIZE; k++) {
        g_obdConfig.atppConfig[k][0] = c_def_programmable_parameters[k][0];
        g_obdConfig.atppConfig[k][1] = c_def_programmable_parameters[k][1];
    }
}
uint8_t loadOtpConfigurationOBD(void) {
    uint16_t k = 0;
    getAddress();
    uint32_t baddr = flash_storage_address2;
    uint32_t tmp   = 0;
    uint16_t size  = sizeof (obdOtpConfig_t);
    //size += (size / 4);
    uint16_t residue = size % 16;
    if (residue) {
        size = size + (16 - residue);
    }
    uint8_t * ptr = (uint8_t *) & g_obdOtpConfig;
    uint16_t n = 0;
    baddr =flash_storage_address2;
    for (k = 0; k < size/3; k++) {
            tmp = FLASH_ReadWord24(baddr);
            ptr[(k*3)+0] = (tmp >> 16) & 0xff;

            ptr[(k*3)+1] = (tmp >> 8) & 0xff;

            ptr[(k*3)+2] = (tmp) & 0xff;

            baddr += 2;
        }   

    uint8_t flag = 0;
    for (k = 0; k < 9; k++) {
        //printDEBUG(0, "%x <=> %x\n", c_obdConfigPreamble[k], g_obdOtpConfig.preamble[k]);
        if (c_obdConfigPreamble[k] != g_obdOtpConfig.preamble[k]) {
            flag = 1;
            break;
        }
    }
    if (flag != 0x00) {
        //printDEBUG(DOBD | DWARNING, "Restore custom OTP configuration to default\n");
        restoreOtpConfigurationOBD(); 
    }
    //printDEBUG(DOBD, " OTP data:\n");
    //printDEBUG(DAPPEND, "\t Hardware ID flag[%xb]\n", g_obdOtpConfig.hardwareIdFlag);
    if (g_obdOtpConfig.hardwareIdFlag == 0x00) {
        //printDEBUG(DAPPEND, "\t\t Hardware ID[%s]\n", g_obdOtpConfig.hardwareId);
        //printDEBUG(DAPPEND, "\t\t Power on cycle count[%dw]\n", g_obdOtpConfig.powerOnResetCount);
    }
    g_obdOtpConfig.powerOnResetCount++;
    updateOtpConfigurationOBD();

    return 0;
}
uint8_t restoreOtpConfigurationOBD(void) {
    uint8_t k;
    for (k = 0; k < 9; k++) {
        g_obdOtpConfig.preamble[k] = c_obdConfigPreamble[k];
    }

    g_obdOtpConfig.hardwareIdFlag = 0xff;
    g_obdOtpConfig.powerOnResetCount = 0x00;
    g_obdOtpConfig.deviceIdentifierFlag = 0xff;
    g_obdOtpConfig.serialNumberIdFlag = 0xff;
    g_obdOtpConfig.calibrationFlag = 0xff;
    strcpy((char*) g_obdOtpConfig.serialNumberId, OBD_DEFAULT_SERIAL_NUMBER);
    updateOtpConfigurationOBD();
    return 0;
}
uint8_t updateOtpConfigurationOBD(void) 
{
    getAddress();
    FLASH_Unlock(FLASH_UNLOCK_KEY);

    result = FLASH_ErasePage(flash_storage_address2);
    if (result == false)
    {
        FlashError();
    }
    uint16_t k = 0;
    uint16_t n = 0;
    uint16_t size = sizeof (obdOtpConfig_t);
    uint8_t * ptr = (uint8_t *) & g_obdOtpConfig;
    uint8_t data[128] = {0x00};
    for (k = 0; k < size; k++) 
    {
        data[n++] = ptr[k];
    }
    uint16_t residue = n % 16;
    if (residue) {
        n = n + (16 - residue);
    }
    //printDEBUG(DSYS, "Size[%d][%d]\n", size, n);
    for (int i = 0 ; i < n/6; i++)
    {
        write_data[i*2]     = (((uint32_t)data[(i*6)+0]<<16) | ((uint32_t)data[(i*6)+1]<<8) | (data[(i*6)+2])); //0x00102030;
        write_data[(i*2)+1] = (((uint32_t)data[(i*6)+3]<<16) | ((uint32_t)data[(i*6)+4]<<8) | (data[(i*6)+5])); //0x00405060;
        //result  = 
        FLASH_WriteDoubleWord24(flash_storage_address2 + (i*4),  write_data[i*2], write_data[(i*2)+1]);
    }
   //if (result == false)
    //{
   //     FlashError();
    //}

    // Clear Key for NVM Commands so accidental call to flash routines will not corrupt flash
    FLASH_Lock();
    return 0;
}

void saveOnFlash(void)
{
    getAddress();
   
    FLASH_Unlock(FLASH_UNLOCK_KEY);

    result = FLASH_ErasePage(flash_storage_address);
    if (result == false)
    {
        FlashError();
    }
    ////
    uint16_t k = 0;
    uint16_t n = 0;
    uint16_t size = sizeof(obdConfig_t);  // size = 234
//    sprintf(testbuf,"\r\n size of obdconfg: %x",size);
//    UART1_Write_String(testbuf);
    uint8_t *ptr = (uint8_t *)&g_obdConfig;
//    sprintf(testbuf, "\r\nAddress of g_obdConfig: %p", (void *)ptr);
//    UART1_Write_String(testbuf);
    uint8_t data[234] = {0x00};
    for (k = 0; k < size; k++) 
    {
//        if (((k % 3) == 0) && (k != 0)) {
//            data[n++] = 0x00;
//        }
        data[n++] = ptr[k];
    }
//    for (k = 0; k < size; k++) 
//    {
//        sprintf(testbuf, "\r\nByte %d: %02X", k, data[k]);
//        UART1_Write_String(testbuf);
//    }
    sprintf(testbuf, "\r\nN: %u", n);
    UART1_Write_String(testbuf);
    uint16_t residue = n % 16;
    if (residue) 
    {
        n = n + (16 - residue);
    }
    sprintf(testbuf, "\r\nn: %u", n);
    UART1_Write_String(testbuf);
    //printDEBUG(DSYS, "Write Cfg Size[%d]\n", size);
    for (int i = 0 ; i < n/6; i++)
    {
        write_data[i*2]     = (((uint32_t)data[(i*6)+0]<<16) | ((uint32_t)data[(i*6)+1]<<8) | (data[(i*6)+2])); //0x00102030;
        write_data[(i*2)+1] = (((uint32_t)data[(i*6)+3]<<16) | ((uint32_t)data[(i*6)+4]<<8) | (data[(i*6)+5])); //0x00405060;
        //result  = 
        FLASH_WriteDoubleWord24(flash_storage_address + (i*4),  write_data[i*2], write_data[(i*2)+1]);
    }

    FLASH_Lock();
    return 0;
}