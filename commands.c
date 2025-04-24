/**
* @file        commands.c
* @author      Aqib D. Ace
* @date        March 2025
* @version     0.0.0
*/


#include "xc.h"
#include "mcc_generated_files/system.h"
#include <stdbool.h>
#include "commands.h"
#include "PM.h"
#include "parameters.h"
#include "mcc_generated_files/uart1.h"
#include "mcc_generated_files/gpios.h"
#include "mcc_generated_files/adc1.h"
#include "delay.h"

extern char ELM327_VERSION[32];
char tempCommand[60];
unsigned long UART1_Timeout;
uint8_t UART1FlowCtrl = 1;
unsigned long baudrate = 9600;
unsigned long oldbaudrate = 9600;
char TXbuffer[60];
uint32_t BRError = 0;
char receivedChar;
char stixStr[32]   = "STN2120 v5.6.5 [2020.10.14]\0";
uint8_t stsatiId[32];
//bool argErrFlag = false;
bool statusLEDs = true;
uint32_t CANbaudrate = 0x00000000;
uint8_t CANprotocol;
void processCommand(char *command)
{
    int j = 0;
    
        // Remove spaces from command
        for (int i = 0; command[i] != '\0' && j < sizeof(tempCommand) - 1; i++) {
        if (command[i] != ' ') {
            tempCommand[j++] = command[i];
        }
        }
        tempCommand[j] = '\0'; // Null-terminate the modified command
        
        //capitalize the Alphabets
        for (int i = 0; tempCommand[i] != '\0'; i++)
        {
            if((tempCommand[i] >= 'a') && (tempCommand[i] <= 'z'))
            {
                tempCommand[i] = (tempCommand[i] - 'a') + 'A';
            }
        }
        
        if (tempCommand[0] == 'S' && tempCommand[1] == 'T')
        {
            processSTCommand(tempCommand);
        }
        else if (tempCommand[0] == 'A' && tempCommand[1] == 'T')
        {
            processATCommand(tempCommand);
        }
        else if (tempCommand[0] == '0')
        {
            processOBDRequest(tempCommand);
        }
        else
        {
            UART1_Write('?');   //wrong command error
        }
        UART1_Write('\r'); //return carriage return to host
        //check Line Feed controlled by ATL
        if (getLFStatus())
        {
            UART1_Write('\n');  //if line feed is on send line feed
            UART1_Write('\n');
        }
        UART1_Write('>'); //send prompt for next command
}

//function for checking numerical arguments
bool argErrCheck(char* command)  
{
    for (int i = 0; command[i] != '\0'; i++)
            {
                if((command[i] < '0') || (command[i] > '9'))
                {
                    return 1;
                    break;
                }
            }
    return 0;
}

//function for checking ASCII arguments
bool argAsciiErrCheck(char* command)
{
    for (int i = 0; command[i] != '\0'; i++)
            {
                if((command[i] <0x020) || (command[i] > 0x07E))
                {
                    return 1;
                    break;
                }
            }
    return 0;
}

//function for processing ST commands
void processSTCommand(char *command) 
{
    if  (strncmp(command, "STBRT", 5) == 0)
    {
        char *stbrtStr = command + 5;
        uint8_t stbrtLen = strlen(stbrtStr);
        if (argErrCheck(stbrtStr))
        {
            UART1_Write('?');
        }
        else if (stbrtLen != 0)
        {
            uint32_t stbrt = strtoul(stbrtStr, NULL, 10);
            if (stbrt > 0 && stbrt < 65536) 
            {
                g_obdInfo.uartTimeout = stbrt;
                UART1_Write_String("OK");
            }
            else
            {
                UART1_Write('?');
            }
        }
        else 
        {
            UART1_Write('?');
        }
    }
    else if (strncmp(command, "STBR", 4) == 0 ) 
    {
        char *stbrStr = command + 4; 
        if (argErrCheck(stbrStr))
        {
            UART1_Write('?');
        }
        else
        {
            g_obdInfo.uartBaudrateTmp = strtoul(stbrStr, NULL, 10);
            if (g_obdInfo.uartBaudrateTmp > 38 && g_obdInfo.uartBaudrateTmp <= 10000000) 
            {
                BRError = UART1_GetBRdiff(g_obdInfo.uartBaudrateTmp);
                if (BRError <= 3)
                {
                    UART1_Write_String("OK");
                    delayMs(10);
                    UART1_Initialize(g_obdInfo.uartBaudrateTmp);
                    
                    delayMs(75);
                    
                    UART1_Write_String(OBD_VERSION_ID);
                    
                    if(UART1_Read() == '\r')
                    {
                        g_obdInfo.uartBaudrate = g_obdInfo.uartBaudrateTmp;
                        UART1_Write_String("OK");
                    }
                    else
                    {
                        UART1_Initialize(g_obdInfo.uartBaudrate);
                    }
                  
                }
                else
                {
                   UART1_Write('?');
                }
            }
            else
            {
                UART1_Write('?');
            }
        }
    }
    else if (strncmp(command, "STSBR", 5) == 0) 
    {
        char *stsbrStr = command + 5; 
        if (argErrCheck(stsbrStr))
        {
            UART1_Write('?');
        }
        else
        {
            g_obdInfo.uartBaudrateTmp = strtoul(stsbrStr, NULL, 10);
            if (g_obdInfo.uartBaudrateTmp >= 38 && g_obdInfo.uartBaudrateTmp <= 10000000) 
            {
                BRError = UART1_GetBRdiff(g_obdInfo.uartBaudrateTmp);
                if (BRError <= 3)
                {
                    UART1_Write_String("OK");
                    delayMs(g_obdInfo.uartTimeout);
                    UART1_Initialize(g_obdInfo.uartBaudrateTmp);
                    g_obdInfo.uartBaudrate = g_obdInfo.uartBaudrateTmp;
                }
                else
                {
                    UART1_Write('?');
                }       
            }
            else
            {
                UART1_Write('?');
            }
        }
    }
    else if (strncmp(command, "STUFC", 5) == 0) 
    {
        char *stufcStr = command + 5;
        uint8_t stufcLen = strlen(stufcStr);
        if (argErrCheck(stufcStr))
        {
            UART1_Write('?');
        }
        else if (stufcLen == 1)
        {
            uint32_t stufc = strtoul(stufcStr, NULL, 10);
            if (stufc == 0) 
            {
                g_obdInfo.uartFlowControlFlag = 0;
                UART1_Write_String("OK");
            }
            else if (stufc == 1)
            {
                g_obdInfo.uartFlowControlFlag = 1;
                UART1_Write_String("OK");
            }
            else
            {
                UART1_Write('?');
            }
        }
        else
        {
            UART1_Write('?');
        }
    }   
    else if (strcmp(command, "STWBR") == 0)
    {   
        g_obdConfig.usartBaudrate = g_obdInfo.uartBaudrate;
        g_obdConfig.atppConfig[0x0C][1] = 0xff;
        updateCustomConfigurationOBD();            
        UART1_Write_String("OK");     
    }
    else if(strcmp(command, "STSLCS") == 0){
        UART1_Write('O');
        UART1_Write('K');
        UART1_Write('\n');
        PM_LCS();
    }
    else if(strcmp(command, "STSLLT") == 0){
        UART1_Write('O');
        UART1_Write('K');
        UART1_Write('\n');
        PM_STSLLT();
    }
    else if(strncmp(command, "STSLEEP", 7) == 0){
        UART1_Write('O');
        UART1_Write('K');
        UART1_Write('\n');
        char val1[20];
        uint16_t index = 7, num;
        for(index; command[index] != '\0' && command[index] != ','; index++){
            val1[index-7] = command[index];
        }
        num = strtoul(val1, NULL, 10);
        PM_Set_Sleep("CMD", 65536>num>=0?num:0);
    }
    else if(strncmp(command, "STSLXP", 6) == 0){
        if(command[6] == '0'||command[6] == '1'){
            UART1_Write('O');
            UART1_Write('K');
            UART1_Write('\n');
            PM_STSLXP(command[6]=='1'?true:false);
        }else{
            UART1_Write("?");
        }
    }
    else if(strncmp(command, "STSLPCP", 7) == 0){
        if(command[7]=='0'||command[7]=='1'){
            UART1_Write('O');
            UART1_Write('K');
            UART1_Write('\n');
            PM_STSLPCP(command[7]=='1'?true:false);
        }else{
            UART1_Write('?');
            UART1_Write('\n');
        }
    }
    else if(strncmp(command, "STSLUIT", 7) == 0){
        char val1[10]="";
        uint16_t index = 7;
        for(index; command[index] != '\0'; index++){
            val1[index-7] = command[index];
        }
        uint16_t num = strtoul(val1, NULL, 10);
        if(num>=0 || num<65536){
            UART1_Write('O');
            UART1_Write('K');
            UART1_Write('\n');
            PM_STSLUIT(num);
        }else{
            UART1_Write('?');
            UART1_Write('\n');
        }
    }
    else if(strncmp(command, "STSLUWP", 7) == 0){
        char val1[10]="", val2[10]="";
        uint16_t index = 7;
        for(index; command[index] != '\0' && command[index] != ','; index++){
            val1[index-7] = command[index];
        }
        for(uint16_t index2 = index + 1; command[index2] != '\0'; index2++){
            val2[index2-index-1] = command[index2];
        }
        uint16_t num1 = strtoul(val1, NULL, 10), num2 = strtoul(val2, NULL, 10);
        if((num1 >= 0 && num1 < 65536)&&(num2 >= 0 && num2 < 65536)){
            UART1_Write('O');
            UART1_Write('K');
            UART1_Write('\n');
            PM_STSLUWP(num1, num2);
        }
        else{
            UART1_Write('?');
            UART1_Write('\n');
        }
    }
    else if(strncmp(command, "STSLVGW", 7) == 0){
        char val1[10]="", val2[10]="", symbol;
        uint16_t index = 7;
        switch(command[index]){
            case '+':
            case '-':
                symbol = command[8];
                index++;
                break;
            default:
                symbol = '=';
        }
        if(command[index+1]=='X'){
            VCHG_CONVERT = false;
            index+=2;
        }
        else{
            VCHG_CONVERT = true;
        }
        for(index; command[index] != '\0' && command[index] != ','; index++){
            val1[index-7] = command[index];
        }
        for(uint16_t index2 = index + 1; command[index2] != '\0'; index2++){
            val2[index2-index-1] = command[index2];
        }
        int steps = strtoul(val1, NULL, 16);
        if(VCHG_CONVERT = true){
            VCHG_WAKE_VOLT = strtof(val1, NULL);
            steps *= ADC_Step;
        }
        uint16_t num2 = strtoul(val2, NULL, 10);
        if((steps >= 0 && steps < 4095)&&(num2 >= 0 && num2 < 65536)){
            UART1_Write('O');
            UART1_Write('K');
            UART1_Write('\n');
            PM_STSLVGW(symbol, steps, num2);
        }
        else{
            UART1_Write('?');
            UART1_Write('\n');
        }
    }
    else if(strncmp(command, "STSLVG", 6) == 0){
        char val1[10]="";
        uint16_t index = 6;
        for(index; command[index] != '\0'; index++){
            val1[index-6] = command[index];
        }
        if(strncmp(val1, "ON", 2)==0||strncmp(val1, "OFF", 3)==0){
            UART1_Write('O');
            UART1_Write('K');
            UART1_Write('\n');
            PM_STSLVG(strncmp(val1, "ON", 2)==0?true:false);
        }
        else{
            UART1_Write('?');
            UART1_Write('\n');
        }
    }
    else if(strncmp(command, "STSLVLS", 7) == 0){
        char val1[10]="", val2[10]="";
        uint16_t index = 8;
        if(command[index+1]=='X'){
            VL_SLEEP_CONVERT = false;
            index+=2;
        }
        else{
            VL_SLEEP_CONVERT = true;
        }
        for(index; command[index] != '\0' && command[index] != ','; index++){
            val1[index-8] = command[index];
        }
        for(uint16_t index2 = index + 1; command[index2] != '\0'; index2++){
            val2[index2-index-1] = command[index2];
        }
        int steps = strtoul(val1, NULL, 16);
        if(VL_SLEEP_CONVERT = true){
            VL_SLEEP_VOLT = strtof(val1, NULL);
            steps *= ADC_Step;
        }
        uint16_t time = strtoul(val2, NULL, 10);
        if((steps >= 0 && steps < 4095)&&(time > 0 && time < 65536)&&(command[7] == '>' || command[7] == '<')){
            UART1_Write('O');
            UART1_Write('K');
            UART1_Write('\n');
            PM_STSLVLS(command[7], steps, time);
        }
        else{
            UART1_Write('?');
            UART1_Write('\n');
        }
    }
    else if(strncmp(command, "STSLVLW", 7) == 0){
        char val1[10]="", val2[10]="";
        uint16_t index = 8;
        if(command[index+1]=='X'){
            VL_WAKE_CONVERT = false;
            index+=2;
        }
        else{
            VL_WAKE_CONVERT = true;
        }
        for(index; command[index] != '\0' && command[index] != ','; index++){
            val1[index-8] = command[index];
        }
        for(uint16_t index2 = index + 1; command[index2] != '\0'; index2++){
            val2[index2-index-1] = command[index2];
        }
        int steps = strtoul(val1, NULL, 16);
        if(VL_WAKE_CONVERT = true){
            VL_WAKE_VOLT = strtof(val1, NULL);
            steps *= ADC_Step;
        }
        uint16_t time = strtoul(val2, NULL, 10);
        if((steps >= 0 && steps <= 4095)&&(time > 0 && time < 65536)&&(command[7] == '>' || command[7] == '<')){
            UART1_Write('O');
            UART1_Write('K');
            UART1_Write('\n');
            PM_STSLVLW(command[7], steps, time);
        }
        else{
            UART1_Write('?');
            UART1_Write('\n');
        }
    }
    else if(strncmp(command, "STSLVL", 6) == 0){
        char val1[10]="", val2[10]="";
        uint16_t index = 6;
        for(index; command[index] != '\0' && command[index] != ','; index++){
            val1[index-6] = command[index];
        }
        for(uint16_t index2 = index + 1; command[index2] != '\0'; index2++){
            val2[index2-index-1] = command[index2];
        }
        if((strncmp(val1, "ON", 2)==0||strncmp(val1, "OFF", 3)==0)&&(strncmp(val2, "ON", 2)==0||strncmp(val2, "OFF", 3)==0)){
            UART1_Write('O');
            UART1_Write('K');
            UART1_Write('\n');
            PM_STSLVL(strncmp(val1, "ON", 2)==0?true:false, strncmp(val2, "ON", 2)==0?true:false);
        }
        else{
            UART1_Write('?');
            UART1_Write('\n');
        }
    }
    else if(strncmp(command, "STSLU", 5) == 0){
        char val1[10]="", val2[10]="";
        uint16_t index = 5;
        for(index; command[index] != '\0' && command[index] != ','; index++){
            val1[index-5] = command[index];
        }
        for(uint16_t index2 = index + 1; command[index2] != '\0'; index2++){
            val2[index2-index-1] = command[index2];
        }
        if((strncmp(val1, "ON", 2)==0||strncmp(val1, "OFF", 3)==0)&&(strncmp(val2, "ON", 2)==0||strncmp(val2, "OFF", 3)==0)){
            UART1_Write('O');
            UART1_Write('K');
            UART1_Write('\n');
            PM_STSLU(strncmp(val1, "ON", 2)==0?true:false, strncmp(val2, "ON", 2)==0?true:false);
        }else{
            UART1_Write('?');
            UART1_Write('\n');
        }
    }
    
    else if (strcmp(command, "STDI") == 0)
    {
        //Print device hardware ID string (e.g., ?OBDLink r1.7?)
        if (g_obdOtpConfig.hardwareIdFlag == 0)
        {
            UART1_Write_String(g_obdOtpConfig.hardwareId);
        }
        else
        {
            UART1_Write('?');
        }
    }
    else if (strcmp(command, "STDICPO") == 0)
    {
        //Print POR (Power on Reset) count
        sprintf(TXbuffer,"%lu",g_obdOtpConfig.powerOnResetCount);
        UART1_Write_String(TXbuffer);
    }
    else if (strcmp(command, "STI") == 0)
    {
        UART1_Write_String(OBD_VERSION_ID);
    }
    else if (strncmp(command, "STSATI", 6) == 0) 
    {
        char *stsatiStr = command + 6;
        uint8_t atiLen = strlen(stsatiStr);
        if (argAsciiErrCheck(stsatiStr))
        {
            UART1_Write('?');
        }
        else if(atiLen > 0 && atiLen < 32)
        {
            strcpy(stsatiId, stsatiStr);
            UART1_Write_String("OK");
            g_obdInfo.atiIdFlag = 1;
        }
        else
        {
            UART1_Write('?');
        }
    }
    else if (strncmp(command, "STSDI", 5) == 0) 
    {
        char *stsdiStr = command + 5;
        uint8_t stsdiLen = strlen(stsdiStr); 
        if (argAsciiErrCheck(stsdiStr))
        {
            UART1_Write('?');
        }
        else if (stsdiLen != 0)
        {
            if (g_obdOtpConfig.hardwareIdFlag != 0x00) 
            {
                g_obdOtpConfig.hardwareIdFlag = 0x00;
                strcpy(g_obdOtpConfig.hardwareId,stsdiStr);
                updateOtpConfigurationOBD();
                UART1_Write_String("OK");
            }
            else
            {
                UART1_Write('?');
            }
        }
        else
        {
            UART1_Write('?');
        }
    }
    else if (strcmp(command, "STSN") == 0)  
    {
        //Print device serial number
        UART1_Write_String(OBD_DEFAULT_SERIAL_NUMBER);
        
    }
    
    else if (strncmp(command, "STS@1", 5) == 0) 
    {
        //use to store the device description string returned by AT@1
        char *stsat1Str = command + 5; 
        uint8_t stsat1Len =strlen(stsat1Str);
        if (argAsciiErrCheck(stsat1Str))
        {
            UART1_Write('?');
        }
        else if (stsat1Len > 0)
        {
          strcpy(g_obdConfig.descriptionString,stsat1Str);
          //UART1_Write_String(stsat1Str);
          UART1_Write_String("OK");
          updateCustomConfigurationOBD();
        }     
        else
        {
            UART1_Write('?');
        }
    }
    else if (strncmp(command, "STVCAL",6) == 0) 
    {
        /*Calibrate voltage measurement. The voltage 
        returned by ATRV and STVR commands can be 
        calibrated using this command. Takes current voltage 
        with a maximum value of 65.534*/ 
        char *voltCalStr = command + 6;
        uint8_t voltCalLen = strlen(voltCalStr);
        uint32_t voltage_calibration = 12345;
        uint32_t voltage_offset = 670;
        if (argAsciiErrCheck(voltCalStr))
        {
            UART1_Write('?');
        }
        else if (voltCalLen > 0)
        {
           uint16_t value[2];
           uint16_t k;
           uint16_t p = 0;
           ParsedData voltCal = parseString(voltCalStr,',');
//         UART1_Write_String("\r\nVal1:");
//         UART1_Write_String(voltCal.values[0]);
//         UART1_Write_String("\r\nVal2:");
//         UART1_Write_String(voltCal.values[1]);
//         UART1_Write_String("\r\n");
//         sprintf(TXbuffer,"count:%u",voltCal.count);
//         UART1_Write_String(TXbuffer);
//         UART1_Write('\n');
           if(voltCal.count < 3)
           {
                    ParsedData voltCalArg1 = parseString(voltCal.values[0],'.');
                    uint32_t Arg1Dec = strtoul(voltCalArg1.values[0], NULL, 10);
                    Arg1Dec = Arg1Dec * 1000;
                    voltage_calibration = Arg1Dec;
                    if(voltCalArg1.count == 2)
                    {   
                        uint32_t Arg1Frac = strtoul(voltCalArg1.values[1], NULL, 10);
                        uint32_t Arg1FracLen = strlen(voltCalArg1.values[1]);
                        if(Arg1Frac < 10 && Arg1FracLen == 1 )
                        {
                            Arg1Frac *= 100;
                        }
                        else if(Arg1Frac < 100 && Arg1FracLen == 2)
                        {
                            Arg1Frac *= 10;
                        }
                        voltage_calibration = voltage_calibration + Arg1Frac;
                    }
                if(voltCal.count == 2)
                {
                    ParsedData voltCalArg2 = parseString(voltCal.values[1],'.');
                    uint32_t Arg2Dec = strtoul(voltCalArg2.values[0], NULL, 10);
                    Arg2Dec = Arg2Dec * 100;
                    voltage_offset = Arg2Dec;
                    if(voltCalArg2.count == 2)
                    {   
                        uint32_t Arg2Frac = strtoul(voltCalArg2.values[1], NULL, 10);
                        uint32_t Arg2FracLen = strlen(voltCalArg2.values[1]);
                        if(Arg2Frac < 10 && Arg2FracLen == 1)
                        {
                            Arg2Frac *= 10;
                        }
                        voltage_offset = voltage_offset + Arg2Frac;
                    } 
                }
            } 
            else
            {
                UART1_Write('?');
            }
            if(voltage_calibration > 0 && voltage_calibration < 65535) 
            {
                g_obdInfo.calibrationFlag = 0x00;
                g_obdInfo.voltageCalibration = voltage_calibration;
                g_obdInfo.voltageOffset = voltage_offset;
                g_obdInfo.adcCalibration = ADC1_Get_Measurement();       
                UART1_Write_String("OK"); 
            }
            else
            {
                UART1_Write('?');
            }
        }
        else if (voltCalLen == 0)
        {
            g_obdInfo.calibrationFlag = 0x00;
            g_obdInfo.voltageCalibration = voltage_calibration;
            g_obdInfo.voltageOffset = voltage_offset;
            g_obdInfo.adcCalibration = ADC1_Get_Measurement();       
            UART1_Write_String("OK");
        }
        
    }
    else if (strcmp(command, "STVRX") == 0) 
    {
        /*Read voltage in ADC steps. Returns the voltage on
         ANALOG_IN pin in ADC counts. The range is 0x000 
         (AVSS) to 0xFFF (AVDD)*/  
        uint16_t adc = ADC1_Get_Measurement();
        if (adc >= 0X000 && adc <= 0xFFF)
        {
            sprintf(TXbuffer,"0x%03X",adc);
            UART1_Write_String(TXbuffer);
        }
        else
        {
            UART1_Write('?');
        }
    }
    else if (strncmp(command, "STVR", 4) == 0) 
    {
         /*Read voltage in volts. Returns calibrated voltage 
        measured by the ANALOG_IN pin.*/
        char* stvrStr = command + 4;
        uint8_t stvrLen = strlen(stvrStr);
        if (argErrCheck(stvrStr))
        {
            UART1_Write('?');
        }
        else if(stvrLen == 1)
        {
            uint16_t adc = ADC1_Get_Measurement();
            uint16_t voltage = ((uint32_t)adc * g_obdInfo.voltageCalibration)/g_obdInfo.adcCalibration;
            uint16_t residue = voltage % 1000;
            uint8_t precession = strtoul(stvrStr, NULL, 10);
            if(precession == 0)
            {
                precession = 0;
            }
            else if(precession == 1)
            {
                precession = 1;
                residue = residue / 100;
               
            }
            else if(precession == 2)
            {
                precession = 2;
                residue = residue / 10;
            }
            else if(precession == 3)
            {
                precession = 3;
            }
            else
            {
                residue = residue / 10;
            } 
            
            if(voltage > 65534)
            {
                switch(precession)
                {
                    case(0):
                    {
                        UART1_Write_String("--.");
                        break;
                    }
                    case(1):
                    {
                        UART1_Write_String("--.-");
                        break;
                    }
                    case(2):
                    {
                        UART1_Write_String("--.--");
                        break;
                    }
                    case(3):
                    {
                        UART1_Write_String("--.---");
                        break;
                    }
                }
            }
            else
            {
                voltage = voltage / 1000;
                if(precession == 0)
                {
                    sprintf(TXbuffer,"%d",voltage);
                }
                else if(precession == 1)
                {
                    sprintf(TXbuffer,"%d.%01d",voltage,residue);
                }
                else if(precession == 2)
                {
                    sprintf(TXbuffer,"%d.%02d",voltage,residue);
                }
                else if(precession == 3)
                {
                    sprintf(TXbuffer,"%d.%03d",voltage,residue);
                }
                else
                {
                    strcpy(TXbuffer,"?");
                }
                UART1_Write_String(TXbuffer);
            }   
        }
        else if (stvrLen == 0)
        {
            uint16_t adc = ADC1_Get_Measurement();
            uint16_t voltage = ((uint32_t)adc * g_obdInfo.voltageCalibration)/g_obdInfo.adcCalibration;
            uint16_t residue = voltage % 1000;
            residue = residue / 10;
            
            if(voltage > 65534)
            {
                UART1_Write_String("--.--");
            }
            else
            {
                voltage = voltage / 1000;
                sprintf(TXbuffer,"%d.%02d",voltage,residue);
                UART1_Write_String(TXbuffer);
            }
        }
        else
        {
            UART1_Write('?');
        }
    }
    ////////////////Table 15//////////////////////
    else if (strcmp(command, "STPBRR") == 0) 
    {
        /*Report actual OBD protocol baud rate. Returns 
         *current protocol bit rate in bps rounded to the nearest 
         *integer value. Returns ??? if the protocol is AUTO. The 
         *actual baud rate will be reported whether the baud rate 
         *is variable or not.
         */
         CANbaudrate = 0x00000000;
         CANprotocol = getProtocol();
         switch(CANprotocol)
         {
            case(11):
            case(12):
            {
                break;
            }
            case(21):
            case(22):
            case(23):
            case(24):
            case(25):
            {
                break;
            }
            case(31):
            case(32):
            case(33):
            case(34):
            case(35):
            case(36):
            {
                //baudrate =  getBaudrateCAN();
                break;
            }
            case(51):
            case(52):
            case(53):
            case(54):
            {
                break;
            }
            case(61):
            case(62):
            case(63):
            case(64):
            {
                break;
            }
            default:
            {
                break;
            }
        }
                    
    }
    else if (strncmp(command, "STPBR",5) == 0) 
    {
        /* Set current OBD protocol baud rate. Takes bit rate i n bps as a decimal number. The 
         * command will round the specified baud rate to the closest value that can be generated. 
         * When values outside the minimum/maximum possible baud rates are specified, they will 
         * be set to the corresponding minimum/maximum value.
         */
        char *stpbrStr = command + 5; 
        if (argErrCheck(stpbrStr))
        {
            UART1_Write('?');
        }
        else
        {
            CANbaudrate = strtoul(stpbrStr, NULL, 10);
            CANprotocol = getProtocol();
            switch(CANprotocol)
            {
               case(11):
               case(12):
               {
                   break;
               }
               case(21):
               case(22):
               case(23):
               case(24):
               case(25):
               {
                   break;
               }
               case(31):
               case(32):
               case(33):
               case(34):
               case(35):
               case(36):
               {
                   //setBaudrateCAN(CANbaudrate);
                   break;
               }
               case(51):
               case(52):
               case(53):
               case(54):
               {
                   break;
               }
               case(61):
               case(62):
               case(63):
               case(64):
               {
                   break;
               }
               default:
               {
                   break;
               }
             }
        }
        
    }
    else if (strncmp(command, "STPCB",5) == 0) 
    {
        /*Turn automatic check byte calculation and checking off/on. When this setting is off, 
         * OBDLink will not automatically append checksum byte for transmitted messages or verify 
         * checksum for received messages.This command does not apply to CAN protocols.
         * Default is 1.
         */      
           UART1_Write_String("OK");
    }
    else if (strcmp(command, "STPC") == 0) 
    {
        /*Close current protocol.*/      
    }
    else if (strcmp(command, "STPRS") == 0) 
    {
        /*Report current protocol string*/
        CANprotocol = getProtocol();
                switch(CANprotocol)
                {
                        case(11):
                        {
                            UART1_Write_String("SAE J1850 PWM\r");
                            break;
                        }
                        case(12):
                        {
                            UART1_Write_String("SAE J1850 VPW\r");
                            break;
                        }
                        case(21):
                        {
                            UART1_Write_String("ISO 9141\r");
                            break;
                        }
                        case(22):
                        {
                            UART1_Write_String("ISO 9141 (5 BAUD)\r");
                            break;
                        }
                        case(23):
                        {
                            UART1_Write_String("ISO 14230\r");
                            break;
                        }
                        case(24):
                        {
                            UART1_Write_String("ISO 14230 (5 BAUD)\r");
                            break;
                        }
                        case(25):
                        {
                            UART1_Write_String("ISO 14230 (FAST)\r");
                            break;
                        }
                        case(31):
                        {
                            UART1_Write_String("HS CAN (ISO 11898, 500K/11B)\r");
                            break;
                        }
                        case(32):
                        {
                            UART1_Write_String("HS CAN (ISO 11898, 500K/29B)\r");
                            break;
                        }
                        case(33):
                        {
                            UART1_Write_String("HS CAN (ISO 15765, 500K/11B)\r");
                            break;
                        }
                        case(34):
                        {
                            UART1_Write_String("HS CAN (ISO 15765, 500K/29B)\r");
                            break;
                        }
                        case(35):
                        {
                            UART1_Write_String("HS CAN (ISO 15765, 250K/11B)\r");
                            break;
                        }
                        case(36):
                        {
                            UART1_Write_String("HS CAN (ISO 15765, 250K/29B)\r");
                            break;
                        }
                        case(51):
                        {
                            UART1_Write_String("MS CAN (ISO 11898, 125K/11B)\r");
                            break;
                        }
                        case(52):
                        {
                            UART1_Write_String("MS CAN (ISO 11898, 125K/29B)\r");
                            break;
                        }
                        case(53):
                        {
                            UART1_Write_String("MS CAN (ISO 15765, 125K/11B)\r");
                            break;
                        }
                        case(54):
                        {
                            UART1_Write_String("MS CAN (ISO 15765, 125K/29B)\r");
                            break;
                        }
                        case(61):
                        {
                            UART1_Write_String("SW CAN (ISO 11898, 33K/11B)\r");
                            break;
                        }
                        case(62):
                        {
                            UART1_Write_String("SW CAN (ISO 11898, 33K/29B)\r");
                            break;
                        }
                        case(63):
                        {
                            UART1_Write_String("SW CAN (ISO 15765, 33K/11B)\r");
                            break;
                        }
                        case(64):
                        {
                            UART1_Write_String("SW CAN (ISO 15765, 33K/29B)\r");
                            break;
                        }
                        default:
                        {

                        }
                    }
    }
    else if (strcmp(command, "STPR") == 0) 
    {
        /*Report current protocol number*/ 
        CANprotocol = getProtocol();
        sprintf(TXbuffer, "%u", CANprotocol);
        UART1_Write_String(TXbuffer);
    }
    else if (strncmp(command, "STPTOT",6) == 0) 
    {
        /*Set the message transmission timeout. This is the time the device will
         *  wait for bus access, before printing ?BUS BUSY?. Takes a decimal 
         * parameter in milliseconds (1 to 65535). The default values for each 
         * protocol are:
         * Default SAE J1850 300ms
         * ISO 9141/14230    300ms
         * ISO 15765-4 (CAN) 50ms
         */      
        char *stptotStr = command + 6; 
        if (argErrCheck(stptotStr))
        {
            UART1_Write('?');
        }
        else
        {
            uint32_t stptotms = strtoul(stptotStr, NULL, 10);
            if(stptotms > 0 && stptotms < 65536)
            {
               g_obdInfo.obdTransmissionTimeout = stptotms;
               UART1_Write_String("OK");
            }
            else
            {
                UART1_Write_String("?");
            }
          }
    }
    else if (strncmp(command, "STPTRQ",6) == 0) 
    {
        /* Set the minimum time between the last response and the next request. 
         * Takes a decimal parameter in milliseconds (1 to 65535). For ISO 9141-2 
         * and ISO 14230-4 protocols, this is the P3 timing.
         * The default for ISO 9141-2 and ISO 14230-4 is 56. 
         * For all others, it is 0.
         */    
        char *stptrqStr = command + 6; 
        if (argErrCheck(stptrqStr))
        {
            UART1_Write('?');
        }
        else
        {
            uint32_t stptrqms = strtoul(stptrqStr, NULL, 10);
            if(stptrqms > 0 && stptrqms < 65536)
            {
               g_obdInfo.obdRequestPeriod = stptrqms;
               UART1_Write_String("OK");
            }
            else
            {
                UART1_Write_String("?");
            }
        }
    }
    else if (strncmp(command, "STPTO",5) == 0) 
    {
        /*Set OBD request timeout. Takes a decimal parameter in milliseconds (1 to 65535). 
         * 0: timeout is infinite.The default setting is controlled by PP 03.
         * Default is 102 ms.
         */
        char *stptoStr = command + 5; 
        if (argErrCheck(stptoStr))
        {
            UART1_Write('?');
        }
        else
        {
            uint32_t stptoms = strtoul(stptoStr, NULL, 10);
            if(stptoms > 0 && stptoms < 65536)
            {
               g_obdInfo.obdRequestTimeout = stptoms;
               UART1_Write_String("OK");
            }
            else
            {
                UART1_Write('?');
            }
        }
    }
    else if (strncmp(command, "STPX",5) == 0) 
    {
        /*Transmit arbitrary message on OBD bus. Takes a variable list of parameters, 
         * separated by commas. Each parameter is prefixed with a single-character 
         * parameter identifier, followed by parameter value, delimited by a colon 
         * character. This command will turn on segmentation but will revert segmentation 
         * back to its previous state (on or off) after sending the message.
         * h/d/I/t/r/x/f
         * prereq cmds h:ATSH/t:STPTO,ATAT/r:ATR/x:ATCEA
         */      
    }
    else if (strncmp(command, "STP",5) == 0) 
    {
        /*Set current protocol preset
         * 31-36 High Speed CAN
         * 51-54 Medium Speed CAN
         * Mode, timeout, request period, baudrate, sid filter and mask, Eid filter and mask
         */      
    }
    
    /////////// Table 17 /////////////////////
    else if (strncmp(command, "STCAF",5) == 0) 
    {
        /*Set CAN addressing format STCAF format [, tt]
         * Format 0/1/2 tt(parameter specifies target address extension and is 
         * required for formats 1 and 2
         * linked commands: STCFCPA
         */    
        char *stcafStr = command + 5; 
        if (argAsciiErrCheck(stcafStr))
        {
            UART1_Write('?');
        }
        else
        {
            ParsedData stcaf = parseString(stcafStr,',');
            if (stcaf.count == 1)
            {
                uint32_t stcafArg0 = strtoul(stcaf.values[0], NULL, 10);
                if(stcafArg0 == 0)
                {
                    g_obdInfo.obdCanAddressingFormat = 0;
                    UART1_Write_String("OK");
                }
                else
                {
                    UART1_Write('?');
                }
            }
            else if (stcaf.count == 2)
            {
                uint32_t stcafValue0 = strtoul(stcaf.values[0], NULL, 10);
                uint32_t stcafValue1 = strtoul(stcaf.values[1], NULL, 16);
                if(stcafValue0 == 1 && stcafValue0 == 2)
                {
                    g_obdInfo.obdCanAddressingFormat = stcafValue0;
                    g_obdInfo.obdCanTargetAddressExtension = stcafValue1;
                    UART1_Write_String("OK");
                }
                else
                {
                    UART1_Write('?');
                }
            }
            else
            {
                UART1_Write('?');
            }
            freeParsedData(&stcaf);
        }
        
    }
    else if (strncmp(command, "STCFCPA",7) == 0) 
    {
        /*Add a flow control CAN address pair. STCFCPA txadd[ext], rxadd[ext]
         * Takes two three-digit or eight-digit parameters: txid is transmitter ID 
         * (i.e. ID transmitted by the OBDLink), and rxid is receiver ID (i.e. ID transmitted by the ECU).
         * Optionally takes two five-digit or ten-digit parameters.
         * Example STCFCPA 7E0, 7E8
         */   
        char *stcfcpaStr = command + 7; 
        if (argAsciiErrCheck(stcfcpaStr))
        {
            UART1_Write('?');
        }
        else
        {
            ParsedData stcfcpa = parseString(stcfcpaStr,',');
            if (stcfcpa.count == 2 )
            {
                uint8_t lenValue0 = strlen(stcfcpa.values[0]);
                uint8_t lenValue1 = strlen(stcfcpa.values[1]);
                if (lenValue0 != lenValue1)
                {
                    UART1_Write('?');
                }
                else if (lenValue0 == 3 || lenValue0 == 8)
                {
                    uint32_t stcfcpaValue0 = strtoul(stcfcpa.values[0], NULL, 16);
                    uint32_t stcfcpaValue1 = strtoul(stcfcpa.values[1], NULL, 16);
                    if(g_obdInfo.obdFlowcontrolPairIdx < OBD_FLOWCONTROL_PAIR_CNT)
                    {
                        g_obdInfo.obdFlowcontrolPairs[g_obdInfo.obdFlowcontrolPairIdx][0] = stcfcpaValue0;     // [Address1,Ext1,Address2,Ext2]
                        g_obdInfo.obdFlowcontrolPairs[g_obdInfo.obdFlowcontrolPairIdx][1] = 0;                 
                        g_obdInfo.obdFlowcontrolPairs[g_obdInfo.obdFlowcontrolPairIdx][2] = stcfcpaValue1;     
                        g_obdInfo.obdFlowcontrolPairs[g_obdInfo.obdFlowcontrolPairIdx][3] = 0;                 
                        g_obdInfo.obdFlowcontrolPairIdx++;
                        UART1_Write_String("OK");
                    }
                    else
                    {
                        UART1_Write_String("OUT OF MEMORY");
                    }
                }
                else if (lenValue0 == 5 || lenValue0 == 10)
                {
                    uint32_t stcfcpaValue2 = strtoul(stcfcpa.values[0], NULL, 16);
                    uint32_t stcfcpaValue4 = strtoul(stcfcpa.values[1], NULL, 16);
      
                    stcfcpa.values[0][lenValue0 - 2] = '\0';
                    stcfcpa.values[1][lenValue1 - 2] = '\0';
                    
                    uint32_t stcfcpaValue1 = strtoul(stcfcpa.values[0], NULL, 16);
                    uint32_t stcfcpaValue3 = strtoul(stcfcpa.values[1], NULL, 16);
                    
                    if(g_obdInfo.obdFlowcontrolPairIdx < OBD_FLOWCONTROL_PAIR_CNT)
                    {
                        stcfcpaValue2 = stcfcpaValue2 & 0xff;
                        stcfcpaValue4 = stcfcpaValue4 & 0xff;
                        g_obdInfo.obdFlowcontrolPairs[g_obdInfo.obdFlowcontrolPairIdx][0] = stcfcpaValue1;   // [Address1,Ext1,Address2,Ext2]
                        g_obdInfo.obdFlowcontrolPairs[g_obdInfo.obdFlowcontrolPairIdx][1] = stcfcpaValue2;                 
                        g_obdInfo.obdFlowcontrolPairs[g_obdInfo.obdFlowcontrolPairIdx][2] = stcfcpaValue3;                 
                        g_obdInfo.obdFlowcontrolPairs[g_obdInfo.obdFlowcontrolPairIdx][3] = stcfcpaValue4;                 
                        g_obdInfo.obdFlowcontrolPairIdx++;
                        UART1_Write_String("OK");
                    }
                    else
                    {
                        UART1_Write_String("OUT OF MEMORY");
                    }
                }
                else
                {
                    UART1_Write('?');
                }
            }
            else
            {
                UART1_Write('?');
            }
        }
    }
    else if (strcmp(command, "STCFCPC") == 0) 
    {
        /*Clear all flow control address pairs.*/ 
        g_obdInfo.obdFlowcontrolPairIdx = 0;
        UART1_Write_String("OK");
    }
    else if (strncmp(command, "STCMM",5) == 0) 
    {
        /*Set CAN monitoring mode. STCMM mode
         * Modes: 0/1/2
         * The default setting is controlled by PP 21. The factory default is 0 
         * (silent monitoring, no ACKs)
         */ 
        char *stcmmStr = command + 5; 
        if (argErrCheck(stcmmStr))
        {
            UART1_Write('?');
        }
        else
        {
            uint32_t stcmm = strtoul(stcmmStr, NULL, 10);
            if(stcmm >=0 && stcmm <= 2)
            {
                g_obdInfo.obdCanMonitoringMode = stcmm;
                UART1_Write_String("OK");
            }
            else
            {
                UART1_Write_String("?");
            }
        }

    }
    else if (strncmp(command, "STCSEGR",7) == 0) 
    {
        /*Disable or enable CAN segmentation for received multi-frame messages. 
         * Automatically removes multiframe PCI bytes and assembles the data into a single message.
         * Arg: 0/1 Default is 0 (CAN segmentation disabled).
         */    
        char *segrStr = command + 7; 
        if (argErrCheck(segrStr))
        {
            UART1_Write('?');
        }
        else
        {
            uint32_t segr = strtoul(segrStr, NULL, 10);
            if(segr == 0 || segr == 1)
            {
                g_obdInfo.obdCanRxSegmentationState = segr;
                UART1_Write_String("OK");
            }
            else
            {
                UART1_Write_String('?');
            }
        }
    }
    else if (strncmp(command, "STCSEGT",7) == 0) 
    {
        /*Disable or enable CAN segmentation for transmitted multi-frame messages. 
         * Automatically splits the message into frames and adds multi-frame PCI bytes.
         * Arg: 0/1  Default is 0 (CAN segmentation disabled)
         */    
        char *segtStr = command + 7; 
        if (argErrCheck(segtStr))
        {
            UART1_Write('?');
        }
        else
        {
            uint32_t segt = strtoul(segtStr, NULL, 10);
            if(segt == 0 || segt == 1)
            {
                g_obdInfo.obdCanTxSegmentationState = segt;
                UART1_Write_String("OK");
            }
            else
            {
                UART1_Write('?');
            }
        }
    }
    else if (strncmp(command, "STCSTM",6) == 0) 
    {
        /*Set additional time for ISO 15765-2 minimum Separation Time (STmin) during 
         * multi-frame message transmission.
         * Timeout can be set in submilliseconds, with at most 3 decimal places. (0.075)
         */   
        char *stcstmStr = command + 6; 
        if (argAsciiErrCheck(stcstmStr))
        {
            UART1_Write('?');
        }
        else
        {
            ParsedData stcstm = parseString(stcstmStr,'.');
            if (stcstm.count == 1 )
            {
                uint32_t stcstmValue = strtoul(stcstm.values[0], NULL, 10);
                g_obdInfo.obdStminTime = stcstmValue;
                UART1_Write_String("OK");
            }
            else if (stcstm.count == 2)
            {
                uint32_t decimal;
                uint32_t fractional;
                uint8_t decimalPlaces;
                float value = 0;
                decimalPlaces = strlen(stcstm.values[1]);
                uint32_t divideFactor = 1;
                uint8_t k = 0;
                for( k = 0; k < decimalPlaces; k++)
                {
                    divideFactor *= 10;
                }
                uint32_t stcstmValue0 = strtoul(stcstm.values[0], NULL, 10);
                uint32_t stcstmValue1 = strtoul(stcstm.values[1], NULL, 10);
                
                
                decimal    = stcstmValue0;
                fractional = stcstmValue1;
                
                value = fractional;
                value = (value / divideFactor);
                
                value = value + decimal;
                
                g_obdInfo.obdStminTime = value;
                UART1_Write_String("OK");
            }
            else
            {
                UART1_Write('?');
            }
            
        }
    }
    else if (strncmp(command, "STCTOR",6) == 0) 
    {
        /*Set receive timeout for ISO 15765-2 Flow Control (FC) and Consecutive (CF) frames.
         * STCTOR fcTimeout, cfTimeout
         * Timeouts are specified in milliseconds. Defaults: fcTimeout = 75 ms cfTimeout = 150 ms
         */    
        char *stctorStr = command + 6; 
        if (argAsciiErrCheck(stctorStr))
        {
            UART1_Write('?');
        }
        else
        {
            ParsedData stctor = parseString(stctorStr,',');
            if (stctor.count == 2)
            {
                uint32_t stctorValue0 = strtoul(stctor.values[0], NULL, 10);
                uint32_t stctorValue1 = strtoul(stctor.values[1], NULL, 10);
                
                g_obdInfo.obdStctroFcTimeout = stctorValue0;
                g_obdInfo.obdStctroCfTimeout = stctorValue1;
                
                UART1_Write_String("OK");
            }
            else
            {
                UART1_Write('?');
            }
        }  
    }
    else if (strcmp(command, "STCTRR") == 0) 
    {
        /*Print the CAN timing configuration registers (set bySTCTR hhhhhh).
         * linked command STCTR
         */ 
        uint32_t stctrrValue = getCANTimConReg();
        sprintf(TXbuffer,"%06X",stctrrValue);
        UART1_Write_String(TXbuffer);
        
    }
    else if (strncmp(command, "STCTR",5) == 0) 
    {
        /*Set the CAN timing configuration registers. The input is the 6-digit hex value 
         * that will be written to the registers.
         */
        char *stctrStr = command + 5; 
        uint8_t stctrlen = strlen(stctrStr);
        if (argAsciiErrCheck(stctrStr))
        {
            UART1_Write('?');
        }
        else
        {
            if(stctrlen == 6)
            {
                uint32_t stctr = strtoul(stctrStr, NULL, 16);
                if(stctr >=0 && stctr <= 0xFFFFFF )
                {
                    setCANTimConReg(stctr);
                    UART1_Write_String("OK");
                }
                else
                {
                    UART1_Write('?');
                }
            }
            else
            {
                UART1_Write('?');
            }
        }
    }
    
    ///////////////Table18 - Monitoring ST Commands ///////////////////
    else if (strncmp(command, "STM",5) == 0) 
    {
        /*Monitor OBD bus using current filters.
         */    
    }
    else if (strncmp(command, "STMA",5) == 0) 
    {
        /*Monitor all messages on OBD bus. For CAN protocols, all messages will 
         * be treated as ISO 15765. To monitor raw CAN messages, use the STM command.
         */    
    }
    
    ///////////////Table 19 - Filtering ST commands ////////////////////
    else if (strcmp(command, "STFAC") == 0) 
    {
        /*Clear all filters.*/ 
        g_obdInfo.obdBlockFilterIdx = 0;
        g_obdInfo.obdPassFilterIdx = 0;
        g_obdInfo.obdFlowcontrolFilterIdx = 0;
    }
    else if (strcmp(command, "STFA") == 0) 
    {
        /*Enable automatic filtering.*/  
        g_obdInfo.obdAfState = OBD_AUTOFILETRING_STATE_ENABLED;
    }
    else if (strncmp(command, "STFBA",5) == 0) 
    {
        /*Add block filter. Same syntax as STFPA. STFBA [pattern], [mask]
         */    
        char *stfbaStr = command + 5; 
        if (argAsciiErrCheck(stfbaStr))
        {
            UART1_Write('?');
        }
        else
        {
            ParsedData stfba = parseString(stfbaStr,',');
            if (stfba.count == 2 )
            {
                uint8_t stfbalen0 = strlen(stfba.values[0]);
                uint8_t stfbalen1 = strlen(stfba.values[1]);
                if (stfbalen0 != stfbalen1)
                {
                    UART1_Write('?');
                }
                else if (stfbalen0%2 != 0 && stfbalen0 <= 10)
                {
                    char* tmp;
                    tmp[0] = '0';
                    for (int i = 0 ; i < stfbalen0 ; i++)
                    {
                        tmp[i+1] = stfba.values[0][i];
                    }
                    stfbalen0++;
                    tmp[stfbalen0]='\0';
                    
                    strcpy(stfba.values[0],tmp);
                    
                    tmp[0]='0';
                    for (int i = 0 ; i < stfbalen1 ; i++)
                    {
                        tmp[i+1] = stfba.values[1][i];
                    }
                    stfbalen1++;
                    tmp[stfbalen1]='\0';
                    
                    strcpy(stfba.values[1],tmp);
                    
                    uint32_t stfbaValue0 = strtoul(stfba.values[0], NULL, 16);
                    uint32_t stfbaValue1 = strtoul(stfba.values[1], NULL, 16);
                    
                    if(g_obdInfo.obdBlockFilterIdx < OBD_BLOCK_FILTER_CNT)
                    {
                        g_obdInfo.obdBlockFilter[g_obdInfo.obdBlockFilterIdx][0] = stfbaValue0;
                        g_obdInfo.obdBlockFilter[g_obdInfo.obdBlockFilterIdx][1] = stfbaValue1;
                        g_obdInfo.obdBlockFilterIdx++;
                        UART1_Write_String("OK");
                    }
                    else
                    {
                        UART1_Write_String("OUT OF MEMORY");
                    }
                }
                else if (stfbalen0 <= 10)
                {
                    uint32_t stfbaValue0 = strtoul(stfba.values[0], NULL, 16);
                    uint32_t stfbaValue1 = strtoul(stfba.values[1], NULL, 16);
                    
                    if(g_obdInfo.obdBlockFilterIdx < OBD_BLOCK_FILTER_CNT)
                    {
                        g_obdInfo.obdBlockFilter[g_obdInfo.obdBlockFilterIdx][0] = stfbaValue0;
                        g_obdInfo.obdBlockFilter[g_obdInfo.obdBlockFilterIdx][1] = stfbaValue1;
                        g_obdInfo.obdBlockFilterIdx++;
                        UART1_Write_String("OK");
                    }
                    else
                    {
                        UART1_Write_String("OUT OF MEMORY");
                    }
                }
                else
                {
                    UART1_Write('?');
                }
            }
            else
            {
                UART1_Write('?');
            }
        }
    }
    else if (strcmp(command, "STFBC") == 0) 
    {
        /*Clear all block filters*/ 
        g_obdInfo.obdBlockFilterIdx = 0;
    }
    else if (strncmp(command, "STFFCA",6) == 0) 
    {
        /*Add flow control filter. Same syntax as STFPA./ STFFCA [pattern], [mask]
         */
        char *stffcaStr = command + 6; 
        if (argAsciiErrCheck(stffcaStr))
        {
            UART1_Write('?');
        }
        else
        {
            ParsedData stffca = parseString(stffcaStr,',');
            if (stffca.count == 2 )
            {
                uint8_t stffcalen0 = strlen(stffca.values[0]);
                uint8_t stffcalen1 = strlen(stffca.values[1]);
                if (stffcalen0 != stffcalen1)
                {
                    UART1_Write('?');
                }
                else if (stffcalen0%2 != 0 && stffcalen0 <= 10)
                {
                    char* tmp;
                    tmp[0] = '0';
                    for (int i = 0 ; i < stffcalen0 ; i++)
                    {
                        tmp[i+1] = stffca.values[0][i];
                    }
                    stffcalen0++;
                    tmp[stffcalen0]='\0';
                    
                    strcpy(stffca.values[0],tmp);
                    
                    tmp[0]='0';
                    for (int i = 0 ; i < stffcalen1 ; i++)
                    {
                        tmp[i+1] = stffca.values[1][i];
                    }
                    stffcalen1++;
                    tmp[stffcalen1]='\0';
                    
                    strcpy(stffca.values[1],tmp);
                    
                    uint32_t stffcaValue0 = strtoul(stffca.values[0], NULL, 16);
                    uint32_t stffcaValue1 = strtoul(stffca.values[1], NULL, 16);
                    
                    if(g_obdInfo.obdFlowcontrolFilterIdx < OBD_BLOCK_FILTER_CNT)
                    {
                        g_obdInfo.obdFlowcontrolFilter[g_obdInfo.obdFlowcontrolFilterIdx][0] = stffcaValue0;
                        g_obdInfo.obdFlowcontrolFilter[g_obdInfo.obdFlowcontrolFilterIdx][1] = stffcaValue1;
                        g_obdInfo.obdFlowcontrolFilterIdx++;
                        UART1_Write_String("OK");
                    }
                    else
                    {
                        UART1_Write_String("OUT OF MEMORY");
                    }
                }
                else if (stffcalen0 <= 10)
                {
                    uint32_t stffcaValue0 = strtoul(stffca.values[0], NULL, 16);
                    uint32_t stffcaValue1 = strtoul(stffca.values[1], NULL, 16);
                    
                    if(g_obdInfo.obdFlowcontrolFilterIdx < OBD_BLOCK_FILTER_CNT)
                    {
                        g_obdInfo.obdFlowcontrolFilter[g_obdInfo.obdFlowcontrolFilterIdx][0] = stffcaValue0;
                        g_obdInfo.obdFlowcontrolFilter[g_obdInfo.obdFlowcontrolFilterIdx][1] = stffcaValue1;
                        g_obdInfo.obdFlowcontrolFilterIdx++;
                        UART1_Write_String("OK");
                    }
                    else
                    {
                        UART1_Write_String("OUT OF MEMORY");
                    }
                }
                else
                {
                    UART1_Write('?');
                }
            }
            else
            {
                UART1_Write('?');
            }
        }
    }
    else if (strncmp(command, "STFFCC",5) == 0) 
    {
        /*Clear all flow control filters.
         */    
        g_obdInfo.obdFlowcontrolFilterIdx = 0;
    }
    else if (strncmp(command, "STFPA",5) == 0) 
    {
        /*Add a pass filter. STFPA [pattern], [mask]
         * Takes two parameters: pattern and mask. Pattern and mask can be any length 
         * from 0 to 5 bytes (0 to 10 ASCII characters), but both have to be the same length.
         * If an odd number of ASCII characters is specified, a leading 0 will be added to the first byte. In other words,
         */    
        char *stfpaStr = command + 5; 
        if (argAsciiErrCheck(stfpaStr))
        {
            UART1_Write('?');
        }
        else
        {
            ParsedData stfpa = parseString(stfpaStr,',');
            if (stfpa.count == 2 )
            {
                uint8_t stfpalen0 = strlen(stfpa.values[0]);
                uint8_t stfpalen1 = strlen(stfpa.values[1]);
                if (stfpalen0 != stfpalen1)
                {
                    UART1_Write('?');
                }
                else if (stfpalen0%2 != 0 && stfpalen0 <= 10)
                {
                    char* tmp;
                    tmp[0] = '0';
                    for (int i = 0 ; i < stfpalen0 ; i++)
                    {
                        tmp[i+1] = stfpa.values[0][i];
                    }
                    stfpalen0++;
                    tmp[stfpalen0]='\0';
                    
                    strcpy(stfpa.values[0],tmp);
                    
                    tmp[0]='0';
                    for (int i = 0 ; i < stfpalen1 ; i++)
                    {
                        tmp[i+1] = stfpa.values[1][i];
                    }
                    stfpalen1++;
                    tmp[stfpalen1]='\0';
                    
                    strcpy(stfpa.values[1],tmp);
                    
                    uint32_t stfpaValue0 = strtoul(stfpa.values[0], NULL, 16);
                    uint32_t stfpaValue1 = strtoul(stfpa.values[1], NULL, 16);
                    
                    if(g_obdInfo.obdPassFilterIdx < OBD_BLOCK_FILTER_CNT)
                    {
                        g_obdInfo.obdPassFilter[g_obdInfo.obdPassFilterIdx][0] = stfpaValue0;
                        g_obdInfo.obdPassFilter[g_obdInfo.obdPassFilterIdx][1] = stfpaValue1;
                        g_obdInfo.obdPassFilterIdx++;
                        UART1_Write_String("OK");
                    }
                    else
                    {
                        UART1_Write_String("OUT OF MEMORY");
                    }
                }
                else if (stfpalen0 <= 10)
                {
                    uint32_t stfpaValue0 = strtoul(stfpa.values[0], NULL, 16);
                    uint32_t stfpaValue1 = strtoul(stfpa.values[1], NULL, 16);
                    
                    if(g_obdInfo.obdPassFilterIdx < OBD_BLOCK_FILTER_CNT)
                    {
                        g_obdInfo.obdPassFilter[g_obdInfo.obdPassFilterIdx][0] = stfpaValue0;
                        g_obdInfo.obdPassFilter[g_obdInfo.obdPassFilterIdx][1] = stfpaValue1;
                        g_obdInfo.obdPassFilterIdx++;
                        UART1_Write_String("OK");
                    }
                    else
                    {
                        UART1_Write_String("OUT OF MEMORY");
                    }
                }
                else
                {
                    UART1_Write('?');
                }
            }
            else
            {
                UART1_Write('?');
            }
        }
        
    }
    else if (strcmp(command, "STFPC") == 0) 
    {
        /*Clear all pass filters
         */    
        g_obdInfo.obdPassFilterIdx = 0;
    }
    
    
    
    ///////
    else if (strcmp(command, "STCALSTAT") == 0) 
    {
        //Read voltage calibration status  
        if((g_obdInfo.calibrationFlag != 0xff) && (g_obdOtpConfig.calibrationFlag != 0xff))
        {
            UART1_Write_String("ANALOG IN: SAVED");
        }
        if((g_obdInfo.calibrationFlag != 0xff) && (g_obdOtpConfig.calibrationFlag == 0xff))
        {
            UART1_Write_String("ANALOG IN: READY");
        }
        else if((g_obdInfo.calibrationFlag == 0xff))
        {
            UART1_Write_String("ANALOG IN: NOT READY");
        }
        else
        {
            UART1_Write('?');
        }  
    }
    else if (strcmp(command, "STRSTNVM") == 0) 
    {
        //Reset NVM to factory defaults 
        restoreCustomConfigurationOBD();
        UART1_Write_String("OK");
    }
    else if (strcmp(command, "STSAVCAL") == 0) 
    {
        //Save all calibration values
        if (g_obdInfo.calibrationFlag == 0x00 && g_obdOtpConfig.calibrationFlag != 0x00)
        {
            
            g_obdOtpConfig.calibrationFlag    = 0x00;
            g_obdOtpConfig.voltageCalibration = g_obdInfo.voltageCalibration;
            g_obdOtpConfig.voltageOffset      = g_obdInfo.voltageOffset;
            g_obdOtpConfig.adcCalibration     = g_obdInfo.adcCalibration;
            UART1_Write_String("OK");
            updateOtpConfigurationOBD();
        }
        else
        {
            UART1_Write('?');
        }
    }
    else if (strncmp(command, "STUIL",5) == 0) 
    {
        //Disable/Enable LEDs
        //Default Enable
        char *stuilStr = command + 5; 
        uint8_t stuilLen = strlen(stuilStr);
        if (argErrCheck(stuilStr))
        {
            UART1_Write('?');
        }
        else if (stuilLen == 1)
        {
            uint32_t stuil = strtoul(stuilStr, NULL, 10);
            if (stuil == 0) 
            {
                setLED(LED_ID_STATUS, LED_STATE_OFF);
                setLED(LED_ID_OBD, LED_STATE_OFF);
                setLED(LED_ID_HOST, LED_STATE_OFF);
                setStatusLED(LED_STATUS_OFF);
                UART1_Write_String("OK");
            }
            else if (stuil == 1)
            {
                setStatusLED(LED_STATUS_ON);
                setLED(LED_ID_STATUS, LED_STATE_ON);
                UART1_Write_String("OK");
            }
            else
            {
                UART1_Write('?');
            }
        }
        else
        {
            UART1_Write('?');
        }
        
    }
    else if (strncmp(command, "STGPC",5) == 0) 
    {
        //Configure I/O pins
        char *stgpcStr = command + 5; 
        if (argAsciiErrCheck(stgpcStr))
        {
            UART1_Write('?');
            return;
        }
        else
        {
            GPIO_CONFIG config[32];
            uint16_t idx = 0;
            uint16_t k = 0;
            uint32_t stgpaPar;
            ParsedData stgpa = parseString(stgpcStr,',');
            for(k = 0; k < stgpa.count ; k++)
            {
                ParsedData stgpaArg = parseString(stgpa.values[k],':');
                if(stgpaArg.count == 2){
                    config[idx].ioTypeConfig = '\0';
                }
                if (2>stgpaArg.count || stgpaArg.count>3)
                {
                    UART1_Write('?');
                    return;
                }
                else
                {
                    uint16_t p = 0;
                    for( p = 0; p < stgpaArg.count; p++)
                    {
                        if(p==0)
                        {
                            stgpaPar = strtoul(stgpaArg.values[0], NULL, 10);
                            config[idx].id = stgpaPar;
                        }
                        else if (p == 1)
                        {
                            if ( strcmp(stgpaArg.values[1],"O")==0)
                            {
                                config[idx].ioConfig = GPIO_CONFIG_OUTPUT;
                            }
                            else if (strcmp(stgpaArg.values[1], "I")==0)
                            {
                                config[idx].ioConfig = GPIO_CONFIG_INPUT;
                            }
                            else
                            {
                                UART1_Write('?');
                                return;
                            }
                        }
                        else if ( p == 2)
                        {
                            if (strcmp(stgpaArg.values[2], "N0") == 0)
                            {
                                config[idx].ioTypeConfig = GPIO_CONFIG_OPEN_DRAIN_DISABLE;
                            }
                            else if (strcmp(stgpaArg.values[2], "N1") == 0)
                            {
                                config[idx].ioTypeConfig = GPIO_CONFIG_OPEN_DRAIN_ENABLE;
                            }
                            else if (strcmp(stgpaArg.values[2], "U0") == 0)
                            {
                                config[idx].ioTypeConfig = GPIO_CONFIG_PULL_UP_DISABLE;
                            }
                            else if (strcmp(stgpaArg.values[2], "U1") == 0)
                            {
                                config[idx].ioTypeConfig = GPIO_CONFIG_PULL_UP_ENABLE;
                            }
                            else if (strcmp(stgpaArg.values[2], "D0") == 0)
                            {
                                config[idx].ioTypeConfig = GPIO_CONFIG_PULL_DOWN_DISABLE;
                            }
                            else if (strcmp(stgpaArg.values[2], "D1") == 0)
                            {
                                config[idx].ioTypeConfig = GPIO_CONFIG_PULL_DOWN_ENABLE;
                            }
                            else
                            {
                                UART1_Write('?');
                                return;
                            }
                        }
                        else
                        {
                            UART1_Write('?');
                            return;
                        }
                    }
                }
                idx++;
            }
            for( k = 0; k < idx; k++ )
            { 
                if(assertGPIO(config[k]) == GPIO_STATUS_ID_INVALID)
                {
                    sprintf(TXbuffer, "%u is Invalid \r\n", config[k].id);
                    UART1_Write_String(TXbuffer); 
                    return;
                }
                else if(configGPIO(config[k]) == GPIO_STATUS_ID_INVALID)
                {
                    sprintf(TXbuffer, "%u is Invalid \r\n", config[k].id);
                    UART1_Write_String(TXbuffer);
                    return;
                }
            }
            
        }
        UART1_Write_String("\nOK");
    }
    else if (strncmp(command, "STGPIRH",7) == 0) 
    {
        //Read inputs, report value as hex
        char *stgpirhStr = command + 7; 
        if (argAsciiErrCheck(stgpirhStr))
        {
            UART1_Write('?');
        }
        else
        {
            GPIO_CONFIG config[32];
            uint16_t idx = 0;
            uint16_t k = 0;
            ParsedData stgpirh = parseString(stgpirhStr,',');
            for(k = 0; k < stgpirh.count ; k++)
            {
                uint32_t stgpirhArg = strtoul(stgpirh.values[k], NULL, 10);
                config[idx].id = stgpirhArg;
                idx++;
            }
            uint8_t stgpirhPinStatus = 0x00;
            for(k = 0; k < idx ; k++)
            {
                stgpirhPinStatus = stgpirhPinStatus << 1;
                if(readInputGPIO(config[k]) != 0x00)
                {
                    stgpirhPinStatus |= 0x01;
                } 
            }
            sprintf(TXbuffer, "%x", stgpirhPinStatus);
            UART1_Write_String(TXbuffer);
        }
    }
    else if (strncmp(command, "STGPIR",6) == 0) 
    {
        //Read inputs
        char *stgpirStr = command + 6; 
        if (argAsciiErrCheck(stgpirStr))
        {
            UART1_Write('?');
        }
        else
        {
            GPIO_CONFIG config[32];
            uint16_t idx = 0;
            uint16_t k = 0;
            ParsedData stgpir = parseString(stgpirStr,',');
            for(k = 0; k < stgpir.count ; k++)
            {
                uint32_t stgpirArg = strtoul(stgpir.values[k], NULL, 10);
                config[idx].id = stgpirArg;
                idx++;
            }
            uint8_t stgpirPinStatus;
            for(k = 0; k < idx ; k++)
            {
                if(assertGPIO(config[k]) == GPIO_STATUS_ID_INVALID)
                {
                    sprintf(TXbuffer, "%u is Invalid \r\n", config[k].id);
                    UART1_Write_String(TXbuffer);
                }
                else
                {
                    stgpirPinStatus = readInputGPIO(config[k]);              
                    if (k == 0)
                    {
                        sprintf(TXbuffer, "%u", stgpirPinStatus);
                    }
                    else
                    {
                        sprintf(TXbuffer, ", %u", stgpirPinStatus);
                    }
                    UART1_Write_String(TXbuffer);
                }
                
            }
            
        }
    }
    else if (strncmp(command, "STGPOR",6) == 0) 
    {
        //Read Output latches
        char *stgporStr = command + 6; 
        if (argAsciiErrCheck(stgporStr))
        {
            UART1_Write('?');
            return;
        }
        else
        {
            GPIO_CONFIG config[32];
            uint16_t idx = 0;
            uint16_t k = 0;
            ParsedData stgpor = parseString(stgporStr,',');
            for(k = 0; k < stgpor.count ; k++)
            {
                uint32_t stgporArg = strtoul(stgpor.values[k], NULL, 10);
                config[idx].id = stgporArg;
                idx++;
            }
            uint8_t stgporPinStatus = 0x00;
            for(k = 0; k < idx ; k++)
            {
                if(assertGPIO(config[k]) == GPIO_STATUS_ID_INVALID)
                {
                    sprintf(TXbuffer, "%u is Invalid \r\n", config[k].id);
                    UART1_Write_String(TXbuffer);
                }
                else
                {
                    stgporPinStatus = getStateGPIO(config[k]);              
                    if (k == 0)
                    {
                        sprintf(TXbuffer, "%u", stgporPinStatus);
                    }
                    else
                    {
                        sprintf(TXbuffer, ", %u", stgporPinStatus);
                    }
                    UART1_Write_String(TXbuffer);
                } 
            }
            
        }
    }
    else if(strcmp(command, "STCANTX") == 0)
    {
        canTransmit();
        canReceive();
        
    }
    else if(strcmp(command, "STCANRX") == 0)
    {
        canReceive();
        canTransmit();
    }
    else if (strncmp(command, "STGPOW",6) == 0) 
    {
        //Write Output latches
        char *stgpowStr = command + 6; 
        if (argAsciiErrCheck(stgpowStr))
        {
            UART1_Write('?');
            return;
        }
        else
        {
            GPIO_CONFIG config[32];
            uint16_t idx = 0;
            uint16_t k = 0;
            uint32_t stgpowPar;
            ParsedData stgpow = parseString(stgpowStr,',');
            for(k = 0; k < stgpow.count ; k++)
            {
                ParsedData stgpowArg = parseString(stgpow.values[k],':');
                if (stgpowArg.count!=2)
                {
                    UART1_Write('?');
                    return;
                }
                else
                {
                    uint16_t p = 0;
                    for( p = 0; p < stgpowArg.count; p++)
                    {
                        if(p==0)
                        {
                            stgpowPar = strtoul(stgpowArg.values[0], NULL, 10);
                            config[idx].id = stgpowPar;
                        }
                        else if (p == 1)
                        {
                            if ( strcmp(stgpowArg.values[1], "0") == 0)
                            {
                                config[idx].state = 0;
                            }
                            else if (strcmp(stgpowArg.values[1], "1") == 0)
                            {
                                config[idx].state = 1;
                            }
                            else
                            {
                                UART1_Write('?');
                                return;
                            }
                        }
                        else
                        {
                            UART1_Write('?');
                            return;
                        }
                    }
                }
                idx++;
            }
            for( k = 0; k < idx; k++ )
            { 
                if(assertGPIO(config[k]) == GPIO_STATUS_ID_INVALID)
                {
                    sprintf(TXbuffer, "%u is Invalid \r\n", config[k].id);
                    UART1_Write_String(TXbuffer); 
                    return;
                }
                else if(setStateGPIO(config[k]) == GPIO_STATUS_ID_INVALID)
                {
                    sprintf(TXbuffer, "%u is Invalid \r\n", config[k].id);
                    UART1_Write_String(TXbuffer);
                    return;
                }
            }    
        }
        UART1_Write_String("\nOK");
    }
    else 
    {
        UART1_Write('?');
        return;
    }
    
}

//function for processing AT commands
void processATCommand(char *command)
{
    if  (strncmp(command, "ATD",3) == 0)
        
    {
        char *atdStr = command + 3;
        uint8_t atdLen = strlen(atdStr);
        if (argErrCheck(atdStr))
        {
            UART1_Write('?');
        }
        else if (atdLen == 1)
        {
          /*Turn printing of CAN DLC on or off. The DLC will 
            be printed between the CAN ID and data bytes, but 
            only if the headers are on (ATH1). By default, DLC 
            printing is off (ATD0). The default setting is controlled 
            by programmable parameter PP 29.*/ 
          uint32_t atdStatus = strtoul(atdStr, NULL, 10);
          if (atdStatus == 0 ||  atdStatus == 1)
          {
            if (atdStatus == 0) 
            {
                g_obdInfo.formatProperties.atd = 0;
            }
            else if (atdStatus == 1)
            {
                g_obdInfo.formatProperties.atd = 1;
            }
            UART1_Write_String("OK");
          }
          else
          {
              UART1_Write('?');
          }
        }
        else
        {
            /*Set all settings to defaults
           * Tester address
           * Last saved protocol
           * Protocol baud rate
           * Message headers
           * Message filter
           * Timeouts*/
        setAllSettoDefault();
        UART1_Write_String("OK");
        }
        
    }
    else if  (strncmp(command, "ATE", 3) == 0)
    {
        //Turn echo on/off 1 means on 0 means off
        //default ON
        
        char *echoStr = command + 3;
        uint8_t echoLen = strlen(echoStr);
        if (argErrCheck(echoStr))
        {
            UART1_Write('?');
        }
        else if (echoLen==1)
        {
          uint32_t echoStatus = strtoul(echoStr, NULL, 10);
          if (echoStatus == 0 ||  echoStatus == 1)
          {
            if (echoStatus == 0) 
            {
                disableEcho();
            }
            else if (echoStatus == 1)
            {
                enableEcho();
            }
            UART1_Write_String("\rOK");
          }
          else
          {
              UART1_Write('?');
          }
        }
        else
        {
            UART1_Write('?');
        }
    }
    else if  (strcmp(command, "ATI") == 0)
    {
        //print ELM327 version ID string
        UART1_Write_String(g_obdConfig.atiId);
     
    }
    else if  (strncmp(command, "ATL", 3) == 0)
    {
        //Line feeds on/off 
        //Default OFF
        char *lineFeedStr = command + 3; // Skip "STBR "
        uint8_t LFLen = strlen(lineFeedStr);
        if (argErrCheck(lineFeedStr))
        {
            UART1_Write('?');
        }
        else if (LFLen==1)
        {
          uint32_t lineFeedStatus = strtoul(lineFeedStr, NULL, 10);
          if (lineFeedStatus == 0 ||  lineFeedStatus == 1)
          {
            if (lineFeedStatus == 0) 
            {
                disableLF();
            }
            else if (lineFeedStatus == 1)
            {
                enableLF();
            }
            UART1_Write_String("\rOK");
          }
          else
          {
              UART1_Write('?');
          }
        }
        else
        {
            UART1_Write('?');
        }
    }
    else if  (strcmp(command, "ATWS") == 0)
    {
        /*Warm start. This command reboots OBDLink, but 
        unlike ATZ, skips the LED test and keeps the user 
        selected baud rate (selected using ATBRD, STBR, or 
        STSBR).*/
        loadDefaultOnWarmReset();
        UART1_Write_String("\rOK");
    }
    else if  (strcmp(command, "ATZ") == 0)
    {
        //Reset device
        if (g_obdInfo.atiIdFlag == 1)
        {
            strcpy(g_obdConfig.atiId, stsatiId);
            updateCustomConfigurationOBD();
        }
        
        delay_ms(100);
        asm("reset");
    }
    else if  (strcmp(command, "AT@1") == 0)
    {
        //Display device description saved by STS@1
        strcpy(TXbuffer,g_obdConfig.descriptionString);
        UART1_Write_String(TXbuffer);
    }
    else if  (strcmp(command, "AT@2") == 0)
    {
        //Display device identifier
        if (g_obdOtpConfig.deviceIdentifierFlag == 0xff) 
        {
            UART1_Write('?');
        } 
        else 
        {
            UART1_Write_String(g_obdOtpConfig.deviceIdentifier);
        }
    }
    else if  (strncmp(command, "AT@3", 4) == 0)
    {
        //store device identifier  
        char *DeviceIDStr = command + 4;
        uint8_t DeviceIDLen =strlen(DeviceIDStr);
        if (argAsciiErrCheck(DeviceIDStr))
        {
           UART1_Write('?');
        }
        else if(DeviceIDLen == 12)
        {
            if(g_obdOtpConfig.deviceIdentifierFlag != 0x00)
            {
                g_obdOtpConfig.deviceIdentifierFlag = 0x00;
                strcpy((char *)g_obdOtpConfig.deviceIdentifier,DeviceIDStr);
                updateOtpConfigurationOBD();
                UART1_Write_String("OK");
            }
            else
            {
                UART1_Write('?');
            }
        } 
        else
        {
            UART1_Write('?');
        }
    }
    else if  (strncmp(command, "ATPPS", 5) == 0)
    {
        //Print programmable parameter summary.
        printPParameters();
    }
    else if (strncmp(command, "ATPPR", 5) == 0)
    {
        restoreDefaultPParameters();
        UART1_Write_String("\rOK");
    }
    else if (strncmp(command, "ATPPTS", 6) == 0)
    {
       //printSaved("ALL");
       UART1_Write_String("\rOKATPPTS");
    }
    else if (strncmp(command, "ATPPT", 5) == 0)
    {
       saveOnFlash();
       UART1_Write_String("\rOKATPPT");
    }
    
    else if  (strncmp(command, "ATPP", 4) == 0)
    {
       //Turn off/ON programmable parameter xx.
       //Set the value of programmable parameter xx to yy.
       char *atppStr = command + 4; // Skip "STBR "
       if (argAsciiErrCheck(atppStr))
        {
           UART1_Write('?');
        }
        else
        {
          configurePPs(atppStr);
          UART1_Write_String("\rOK");
        }
    }
    
    else if (strncmp(command, "ATPOC", 5) == 0)
    {
       //initNVM();
       //printAllSaved();
    }
    
    /////////// Table 5 - OBD AT Commands //////////////////
    else if (strcmp(command, "ATAL") == 0)
    {
       /*Allow long messages. 
        * The ATAL command removes the limit, allowing OBDLink to accept OBD requests 
        * and replies longer than 7 bytes (up to the maximum supported by the currently selected OBD protocol).
        * The default is ATNL (normal length, ATAL off).
        */
        g_obdInfo.obdLongMessageState = OBD_LONG_MESSAGE_STATE_ENABLED;
        UART1_Write_String("OK");
    }
    else if (strncmp(command, "ATAT", 4) == 0)
    {
       /*Set adaptive timing mode. ATAT mode
        * Modes: 0/1/2 
        * First frame will follow STPTO/ATST timeout
        */
        char *atatStr = command + 4; // Skip "STBR "
        if (argErrCheck(atatStr))
        {
            UART1_Write('?');
        }
        else
        {
          uint32_t atat = strtoul(atatStr, NULL, 10);
          if (atat == 0 ||  atat == 1 || atat == 2)
          {
              g_obdInfo.obdAtatModeState = atat;
              UART1_Write_String("OK");
          }
          else
          {
              UART1_Write('?');
          }
        }
    }
    else if (strncmp(command, "ATH", 3) == 0)
    {
       /*Turn display of headers on or off. 
        * By default, headers are off (ATH0)
        * (ATH1) to display the headers, check byte, and CAN PCI byte.
        */
        char *athStr = command + 3;
        uint8_t athLen = strlen(athStr);
        if (argErrCheck(athStr))
        {
            UART1_Write('?');
        }
        else if (athLen == 1)
        {
          uint32_t athValue = strtoul(athStr, NULL, 10);
          if (athValue == 0 ||  athValue == 1)
          {
              g_obdInfo.formatProperties.ath = athValue; 
              UART1_Write_String("OK");
          }
          else
          {
              UART1_Write('?');
          }
        }
        else
        {
            UART1_Write('?');
        }
        
    }
    else if (strcmp(command, "ATNL") == 0)
    {
       /*Enforce normal message length. 
        * linked Command ATAL
        */
        g_obdInfo.obdLongMessageState = OBD_LONG_MESSAGE_STATE_DISABLED;
        UART1_Write_String("OK");
    }
    else if (strcmp(command,"ATRTR") == 0)
    {
       /*Send an RTR (Remote Transmission Request) CAN frame.
        * By default, OBDLink ignores (doesn?t print) RTR frames. To enable 
        * printing of RTR frames, turn on the headers (ATH 1) or turn CAN formatting 
        * off (ATCAF 0).*/
        txRemoteCAN(g_obdInfo.canSid, g_obdInfo.canEid, g_obdInfo.canMode, g_obdInfo.obdTransmissionTimeout);
    }
    else if (strncmp(command, "ATR", 3) == 0)
    {
       /*Turn responses on or off.
        * By Default Responses are On
        * Arg 0/1
        */
        char *atrStr = command + 3;
        uint8_t atrLen = strlen(atrStr);
        if (argErrCheck(atrStr))
        {
            UART1_Write('?');
        }
        else if(atrLen == 1)
        {
          uint32_t atrValue = strtoul(atrStr, NULL, 10);
          if (atrValue == 0)  
          {
              g_obdInfo.obdResponseState = OBD_RESPONSE_STATE_DISABLED;
              UART1_Write_String("OK");     
          }
          else if (atrValue == 1)
          {
              g_obdInfo.obdResponseState = OBD_RESPONSE_STATE_ENABLED;
              UART1_Write_String("OK");
          }
          else
          {
              UART1_Write('?');
          }
        }
        else
        {
            UART1_Write('?');
        }
        
    }
    else if (strncmp(command, "ATSH", 4) == 0)
    {
       /*Set the header of transmitted OBD messages to header. ATSH header
        * Depends on the current selected Protocol
        * linked Commands ATCP 18///use ATCP before ATSH
        */
        char *atshStr = command + 4; 
        uint8_t atshLen = strlen(atshStr);
        if (argAsciiErrCheck(atshStr))
        {
            UART1_Write('?');
        }
        else if (atshLen > 0)
        {
            uint32_t sid;
            sid = strtoul(atshStr, NULL, 16);
            if (atshLen == 3)
            {
                g_obdInfo.canSid = sid;
                UART1_Write_String("OK");
            }
            else if (atshLen == 6)
            {
               g_obdInfo.canEid  = g_obdInfo.canEid | (sid & 0xFFFFFF);
               UART1_Write_String("OK");
            }
            else 
            {
                UART1_Write('?');
            }
        }
        else
        {
            UART1_Write('?');
        }
            //printDEBUG(DSYS, "SID[%xw]\n", sid);
            //sendStringATP("OK");
    }
    else if (strncmp(command, "ATS", 3) == 0)
    {
       /*Turn printing of spaces in OBD responses on or off. 
        * By default, spaces are on (ATS 1)
        * for performance, turn spaces off (ATS 0).
        */
        char *atsStr = command + 3;
        uint8_t atsLen = strlen(atsStr);
        if (argErrCheck(atsStr))
        {
            UART1_Write('?');
        }
        else if (atsLen == 1)
        {
          uint32_t atsValue = strtoul(atsStr, NULL, 10);
          if (atsValue == 0 ||  atsValue == 1)
          {
              g_obdInfo.formatProperties.ats = atsValue;
              UART1_Write_String("OK");     
          }
          else
          {
              UART1_Write('?');
          }
        }
        else
        {
            UART1_Write('?');
        }
    }
    else if (strncmp(command, "ATTA", 4) == 0)
    {
       /*Set tester address to hh/ ATTA hh
        * linked command: STPC/STPO     
        */
        char *attaStr = command + 4;
        uint8_t attaLen = strlen(attaStr);
        if (argAsciiErrCheck(attaStr))
        {
            UART1_Write('?');
        }
        else if (attaLen == 2)
        {
          uint32_t attaValue = strtoul(attaStr, NULL, 16);
          if (attaValue >= 0 &&  attaValue <= 0xFF)
          {
              g_obdConfig.testerAddress = attaValue;
              UART1_Write_String("OK");     
          }
          else
          {
              UART1_Write('?');
          }
        }
        else
        {
            UART1_Write('?');
        }
    }
    
    
    /////////// Table 8 - CAN Specific AT Commands(protocol 6 to C) //////////////////
    else if (strncmp(command, "ATCAF", 5) == 0)
    {
       /*Turn CAN Auto Formatting on or off.
        * Arg 0/1
        * Linked command: ATV    */
        char *atcafStr = command + 5;
        uint8_t atcafLen = strlen(atcafStr);
        if (argAsciiErrCheck(atcafStr))
        {
            UART1_Write('?');
        }
        else if (atcafLen == 1)
        {
            uint32_t atcafValue = strtoul(atcafStr, NULL, 16);
            if (atcafValue == 0)
            {
                g_obdInfo.formatProperties.atcafFormatting = 0;
                g_obdInfo.formatProperties.PCIRequest = 0;
                g_obdInfo.formatProperties.PCIResponse = 1;
                g_obdInfo.formatProperties.paddingResponse = 1;
                g_obdInfo.formatProperties.RTRFrames = 1;
                g_obdInfo.formatProperties.printFrameNumber = 0;
                g_obdInfo.formatProperties.printFC = 0;
                UART1_Write_String("OK");
                
            }
            else if (atcafValue == 1)
            {
                g_obdInfo.formatProperties.atcafFormatting = 1;
                g_obdInfo.formatProperties.PCIRequest = 1;
                g_obdInfo.formatProperties.PCIResponse = 0;
                g_obdInfo.formatProperties.paddingResponse = 0;
                g_obdInfo.formatProperties.RTRFrames = 0;
                g_obdInfo.formatProperties.printFrameNumber = 1;
                g_obdInfo.formatProperties.printFC = 1;
                UART1_Write_String("OK");
            }
            else
            {
                UART1_Write('?');
            }
        }
        else
        {
            UART1_Write('?');
        }
    }
    else if (strncmp(command, "ATCFC", 5) == 0)
    {
       /*Turn automatic CAN flow control on or off.
        * Default: ON 
        * Note that OBDLink never sends flow control frames while monitoring.
        */
        char *atcfcStr = command + 5;
        uint8_t atcfcLen = strlen(atcfcStr);
        if (argErrCheck(atcfcStr))
        {
            UART1_Write('?');
        }
        else if (atcfcLen == 1)
        {
            uint32_t atcfcValue = strtoul(atcfcStr, NULL, 10);
            if (atcfcValue == 0)
            {
                g_obdInfo.obdFlowcontrolActive = 0;
                UART1_Write_String("OK");
            }
            else if (atcfcValue == 1)
            {
                g_obdInfo.obdFlowcontrolActive = 1;
                UART1_Write_String("OK");
            }
            else
            {
                UART1_Write('?');
            }
        }
        else
        {
            UART1_Write('?');
        }
    }
    else if (strncmp(command, "ATCF", 4) == 0)
    {
       /*Set the CAN hardware filter pattern. ATCF pattern
        * This command accepts both 11-bit and 29-bit CAN IDs.
        * Example: ATCF 7E0/ATCF 18 DB 00 00
        */
        char *atcfStr = command + 4; // Skip "STBR "
        if (argAsciiErrCheck(atcfStr))
        {
            UART1_Write('?');
        }
        else
        {
            uint8_t atcfArgLen = strlen (atcfStr);
            uint32_t atcfValue = strtoul(atcfStr, NULL, 16);
            if (atcfArgLen == 3) 
            {
                uint32_t sid;
                g_obdInfo.canSidFilter = atcfValue & 0x7FF;
                configureFilterCAN(g_obdInfo.canMode, g_obdInfo.canSidFilter, g_obdInfo.canEidFilter, g_obdInfo.canSidFilterMask, g_obdInfo.canEidFilterMask);
                UART1_Write_String("OK"); 
            } 
            else if (atcfArgLen == 8) 
            {
                g_obdInfo.canEidFilter = atcfValue;
                configureFilterCAN(g_obdInfo.canMode, g_obdInfo.canSidFilter, g_obdInfo.canEidFilter, g_obdInfo.canSidFilterMask, g_obdInfo.canEidFilterMask);
                UART1_Write_String("OK");
            }
            else
            {
                UART1_Write('?');
            }
        }
    }
    else if (strncmp(command, "ATCM", 4) == 0)
    {
       /*Set the CAN hardware filter mask. ATCM mask
        * This command accepts both 11-bit and 29-bit CAN IDs.
        * Example: ATCM FF0/ATCM FF FE 00 00
        */
        char *atcmStr = command + 4; // Skip "STBR "
        if (argAsciiErrCheck(atcmStr))
        {
            UART1_Write('?');
        }
        else
        {
            uint8_t atcmArgLen = strlen (atcmStr);
            uint32_t atcmValue = strtoul(atcmStr, NULL, 16);
            if (atcmArgLen == 3) 
            {
                uint32_t sid;
                g_obdInfo.canSidFilterMask = atcmValue & 0x7FF;
                configureFilterCAN(g_obdInfo.canMode, g_obdInfo.canSidFilter, g_obdInfo.canEidFilter, g_obdInfo.canSidFilterMask, g_obdInfo.canEidFilterMask);
                UART1_Write_String("OK"); 
            } 
            else if (atcmArgLen == 8) 
            {
                g_obdInfo.canEidFilterMask = atcmValue;
                configureFilterCAN(g_obdInfo.canMode, g_obdInfo.canSidFilter, g_obdInfo.canEidFilter, g_obdInfo.canSidFilterMask, g_obdInfo.canEidFilterMask);
                UART1_Write_String("OK");
            }
            else
            {
                UART1_Write('?');
            }
        }    
    }
    else if (strncmp(command, "ATCP", 4) == 0)
    {
       /*Set CAN Priority bits of a 29-bit CAN ID. ATCP hh 
        * This command sets the five most significant bits of transmitted frames Extended ID. 
        * linked command: use before ATSH
        * The three most significant bits of the parameter are ignored.
        */
        char *atcpStr = command + 4;
        uint8_t atcpLen = strlen(atcpStr);
        if (argAsciiErrCheck(atcpStr))
        {
            UART1_Write('?');
        }
        else if (atcpLen == 2)
        {
          uint32_t atcpValue = strtoul(atcpStr, NULL, 16);
          if (atcpValue >= 0 && atcpValue <= 0x1F)
          {
              g_obdInfo.canEid &= ~(0xFF000000);
              g_obdInfo.canEid |= (atcpValue << 24);
              UART1_Write_String("OK");     
          }
          else
          {
              UART1_Write('?');
          }
        }
        else
        {
            UART1_Write('?');
        }
    }
    else if (strncmp(command, "ATCRA", 5) == 0)
    {
       /*This command sets the CAN hardware filter pattern to pattern. ATCRA pattern
        * If the parameter pattern contains any non-hex characters, those hex values 
        * will be treated as don?t cares. 
        * Send ATCRA (without any parameters) to reset the CAN hardware filter to its default state.
        * Examples: ATCRA 7E9/ATCRA 18 DA F1 10/ATCRA 7EX this X mean dont care accept 0-F
        */
        char *atcraStr = command + 5;
        uint8_t atcraArgLen = strlen(atcraStr);
        while(*atcraStr != '\0')
        {
            if ( (*atcraStr >= '0' && *atcraStr <= '9') || (*atcraStr >= 'A' && *atcraStr <= 'F') )
            {
            }
            else
            {
                *atcraStr = '0';
            }
            atcraStr++;
        }
        atcraStr = atcraStr - atcraArgLen;
        
        if (argAsciiErrCheck(atcraStr))
        {
            UART1_Write('?');
        }
        else
        {
            uint32_t atcraValue = strtoul(atcraStr, NULL, 16);
            if (atcraArgLen == 3)
            {
                g_obdInfo.canSidFilter     = atcraValue;
                g_obdInfo.canSidFilterMask = atcraValue;
                configureFilterCAN(g_obdInfo.canMode, g_obdInfo.canSidFilter, g_obdInfo.canEidFilter, g_obdInfo.canSidFilterMask, g_obdInfo.canEidFilterMask);
                UART1_Write_String("OK");
            }
            else if (atcraArgLen == 8)
            {
                g_obdInfo.canEidFilter     = atcraValue;
                g_obdInfo.canEidFilterMask = atcraValue;
                configureFilterCAN(g_obdInfo.canMode, g_obdInfo.canSidFilter, g_obdInfo.canEidFilter, g_obdInfo.canSidFilterMask, g_obdInfo.canEidFilterMask);
                UART1_Write_String("OK");
            }
            else
            {
                g_obdInfo.canSidFilter = DEFAULT_SID_CAN_FILTER;
                g_obdInfo.canEidFilter = DEFAULT_EID_CAN_FILTER;
                g_obdInfo.canSidFilterMask = DEFAULT_SID_CAN_FILTER_MASK;
                g_obdInfo.canEidFilterMask = DEFAULT_EID_CAN_FILTER_MASK;
                configureFilterCAN(g_obdInfo.canMode, g_obdInfo.canSidFilter, g_obdInfo.canEidFilter, g_obdInfo.canSidFilterMask, g_obdInfo.canEidFilterMask);
                UART1_Write_String("OK");
            }
        }
    }
    else if (strncmp(command, "ATCS", 5) == 0)
    {
       /*Print CAN status counts. 
        * This command displays the number of transmit and receive error counts, 
        * as a hexadecimal number.
        * Linked command STPC
        */
        uint8_t rxCnt;
        uint8_t txCnt;
        getErrorCountCAN(&rxCnt, &txCnt);
        sprintf(TXbuffer, "T:%02X R:%02X\r", txCnt, rxCnt);
        UART1_Write_String(TXbuffer);
    }
    else if (strncmp(command, "ATFCSD", 6) == 0)
    {
       /*Set flow control data. ATFCSD data_bytes
        * Only relevant when flow control mode 1 or 2 has been enabled
        * Linked command ATFCSM
        * Example below specifies a block size of 2, and a separation time (STmin) of 16 ms.
        * ATFCSD 30 02 10
        */
        char *atfcsdStr = command + 6; 
        if (argAsciiErrCheck(atfcsdStr))
        {
            UART1_Write('?');
        }
        else
        {
                uint8_t atfcsdlen = strlen(atfcsdStr);
                if (atfcsdlen % 2 != 0 || atfcsdlen > 10)
                {
                    UART1_Write('?');
                }
                else
                {
                    uint8_t tmp[3];
                    uint32_t stfcsd;
                    uint8_t data[5]={0};
                    uint8_t n=0;
                    int k;
                    for(k = 0; k < atfcsdlen/2;k++)
                    {
                        tmp[0] = atfcsdStr[n++];
                        tmp[1] = atfcsdStr[n++];
                        tmp[2] = '\0';
                        stfcsd = strtoul(tmp, NULL, 16);
                        data[k] = stfcsd;
                    }
                    for( n = 0; n < OBD_FLOWCONTROL_DATA_CNT; n++)
                    {
                        if( n < k )
                        {
                            g_obdInfo.obdFlowcontrolData[n] = data[n];
                        }
                        else
                        {
                            g_obdInfo.obdFlowcontrolData[n] = 0x00;
                        }
                    }
                    g_obdInfo.obdFlowcontrolDataFlag = 1;
                    UART1_Write_String("OK");
                }        
        }
    }
    else if (strncmp(command, "ATFCSH", 6) == 0)
    {
       /*Set flow control header (CAN ID). ATFCSH fc_header
        * Only relevent in flow control mode 1
        * Linked command ATFCSM
        * 
        */
        char *atfcshStr = command + 6; 
        if (argAsciiErrCheck(atfcshStr))
        {
            UART1_Write('?');
        }
        else
        {
            uint8_t atfcshlen = strlen(atfcshStr);
            if (atfcshlen == 3 || atfcshlen == 8)
            {
                uint32_t atfcshValue = strtoul(atfcshStr, NULL, 16);
                g_obdInfo.obdFlowcontrolHeader = atfcshValue;
                g_obdInfo.obdFlowcontrolHeaderFlag = 1;
                UART1_Write_String("OK");
            }
            else
            {
                UART1_Write('?');
            }
        }
    }
    else if (strncmp(command, "ATFCSM", 6) == 0)
    {
       /*Set flow control mode. ATFCSM mode
        * Arguments 0/1/2 Default 0
        * pre req ATFCSD/ATFCSH
        */
        char *atfcsmStr = command + 6;
        uint8_t atfcsmLen = strlen(atfcsmStr);
        if (argErrCheck(atfcsmStr))
        {
            UART1_Write('?');
        }
        else if (atfcsmLen == 1)
        {
          uint32_t atfcsmValue = strtoul(atfcsmStr, NULL, 16);
          if (atfcsmValue == 0 )
          {
              g_obdInfo.obdFlowcontrolMode = OBD_CAN_FLOWCONTROL_MODE_AUTOMATIC;
              UART1_Write_String("OK");     
          }
          else if (atfcsmValue == 1 && g_obdInfo.obdFlowcontrolDataFlag == 1 && g_obdInfo.obdFlowcontrolHeaderFlag == 1)
          {
              g_obdInfo.obdFlowcontrolMode = OBD_CAN_FLOWCONTROL_MODE_USER_DEFINED_ID_DATA;
              UART1_Write_String("OK");
          }
          else if (atfcsmValue == 2 && g_obdInfo.obdFlowcontrolDataFlag == 1 && g_obdInfo.obdFlowcontrolHeaderFlag == 1)
          {
              g_obdInfo.obdFlowcontrolMode = OBD_CAN_FLOWCONTROL_MODE_AUTOMATIC_ID_USER_DATA;
              UART1_Write_String("OK");
          }
          else
          {
              UART1_Write('?');
          }
        }
        else
        {
            UART1_Write('?');
        }
    }
    else if (strncmp(command, "ATPB", 4) == 0)
    {
       /*Set Protocol B parameters. ATPB xx yy
        * Configures Protocol B (USER1) options and baud rate.
        * The xx parameter corresponds to the options set by PP 2C, 
        * while yy corresponds to PP 2D.
        */
        char *atpbStr = command + 4; 
        if (argAsciiErrCheck(atpbStr))
        {
            UART1_Write('?');
        }
        else
        {
            uint8_t atpblen = strlen(atpbStr);
            if (atpblen == 4)
            {
                uint32_t atpbValue = strtoul(atpbStr, NULL, 16);
                uint8_t atpbValue1 =  atpbValue & 0xFF;
                uint8_t atpbValue2 = (atpbValue >> 8) & 0xFF;
                if(atpbValue1 > 0x20)
                {
                    UART1_Write('?');
                }
                else
                {
                    g_obdInfo.protocolbSettings = atpbValue2;
                    g_obdInfo.protocolbBaudrate = atpbValue1;
                    UART1_Write_String("OK");
                }
            }
            else
            {
                UART1_Write('?');
            }
        }
         
    }
    else if (strncmp(command, "ATV", 3) == 0)
    {
       /*Variable DLC on or off. When one of the CAN protocols is selected, this 
        * command controls whether variable or fixed (DLC = 8) Data Length Code is used. 
        * The default is fixed DLC (ATV 0)
        */
        char *atvStr = command + 3;
        uint8_t atvLen = strlen(atvStr);
        if (argErrCheck(atvStr))
        {
            UART1_Write('?');
        }
        else if (atvLen == 1)
        {
          uint32_t atvValue = strtoul(atvStr, NULL, 10);
          if (atvValue == 0 ||  atvValue == 1 )
          {
              g_obdInfo.obdCanVariableDlc = atvValue; 
              UART1_Write_String("OK");     
          }
          else
          {
              UART1_Write('?');
          }
        }
        else
        {
            UART1_Write('?');
        }
    }
    else 
    {
        UART1_Write('A');
    } 
}
//function for processing OBD requests
void processOBDRequest(char *command)
{
    
        if (argAsciiErrCheck(command))
        {
            UART1_Write('?');
        }
        else
        {
            obdRequestHandle(command);
        }
}

//function for setting values to default on ATWS call
void loadDefaultOnWarmReset(void)
{
    loadDefaultConfigurationOBD();
    //loadCustomConfigurationOBD(PROGRAMMABLE_PARAMETERS_TYPE_R);
    loadCustomConfigurationOBD(); //TODO:
    
}

void obdRequestHandle(char *command)
{
   // Length check (minimum 2 bytes for mode)
    uint8_t len = strlen(command);
    if (len < 2 || len % 2 != 0) 
    {
        UART1_Write_String("Invalid command length\n");
        return;
    }

    // Parse the command string into bytes
    uint8_t data[8] = {0}; // Max 8 bytes for CAN payload
    uint8_t receivedData[8] = {0};
    uint8_t receivedDatalen = 0;
    uint8_t data_len = len / 2;
    if (data_len > 8) 
    {
        UART1_Write_String("Command too long for CAN payload\n");
        return;
    }

    for (uint8_t i = 0; i < data_len; i++) 
    {
        char byteStr[3] = { command[i * 2], command[i * 2 + 1], '\0' };
        data[i] = strtoul(byteStr, NULL, 16);
    }

    // Print parsed data for debug
//    UART1_Write_String("Parsed OBD request:\n\r");
//    for (uint8_t i = 0; i < data_len; i++) 
//    {
//        sprintf(TXbuffer,"%x",data[i]);
//        UART1_Write_String(TXbuffer);
//        UART1_Write_String("\n\r");
//    }
//    UART1_Write_String("\n");
    if (g_obdInfo.obdLongMessageState == OBD_LONG_MESSAGE_STATE_ENABLED)
    {
        iso_tp_send(0x7DF, 0x18DB33F1, CAN_MODE_STANDARD_DATA_FRAME_8_DLC, data, data_len, 100);
        if(g_obdInfo.obdResponseState == OBD_RESPONSE_STATE_ENABLED)
        {
            iso_tp_receive(CAN_MODE_STANDARD_DATA_FRAME_8_DLC, receivedData, &receivedDatalen, 100,0x7DF, 0x18DB33F1);
        }
    }
    else if(g_obdInfo.obdLongMessageState == OBD_LONG_MESSAGE_STATE_DISABLED)
    {
       txMessageCAN(0x7DF, 0x18DB33F1, CAN_MODE_STANDARD_DATA_FRAME_8_DLC, data, data_len, 2000);
       //canTransmit(data,data_len);
       if(g_obdInfo.obdResponseState == OBD_RESPONSE_STATE_ENABLED)
       {
           canReceive(CAN_MODE_STANDARD_DATA_FRAME_8_DLC, receivedData, &receivedDatalen, 2000);
       }
    }
    if(g_obdInfo.obdResponseState == OBD_RESPONSE_STATE_ENABLED)
    {
//        sprintf(TXbuffer,"reLEN=%02X",receivedDatalen);
//        UART1_Write_String(TXbuffer);
        
        if (g_obdInfo.formatProperties.atcafFormatting == 1 && g_obdInfo.formatProperties.ath != 1)
        {
            sprintf(TXbuffer,"%03X",receivedDatalen);
            UART1_Write_String(TXbuffer);
            if(g_obdInfo.formatProperties.ats == 1)
            {
                UART1_Write(' ');
            }
            if (g_obdInfo.linefeedState == 1)
            {
                UART1_Write_String("\n\r");
            }
            
            UART1_Write_String("0: ");
            uint8_t dataLineCount = 1;
            for (int i = 1; i <= receivedDatalen; i++)
            {
                if (i % 7 == 0)
                {
                    if (g_obdInfo.linefeedState == 1)
                    {
                        UART1_Write('\n');
                    }
                    sprintf(TXbuffer,"\r%X:",dataLineCount);
                    UART1_Write_String(TXbuffer);
                    dataLineCount++;
                    if(g_obdInfo.formatProperties.ats == 1)
                    {
                        UART1_Write(' ');
                    }
                }
                sprintf(TXbuffer,"%02X",receivedData[i]);
                UART1_Write_String(TXbuffer);
                if(g_obdInfo.formatProperties.ats == 1)
                {
                    UART1_Write(' ');
                }
            }
        }
        else
        {
            if(g_obdInfo.formatProperties.atd == 1 && g_obdInfo.formatProperties.ath == 1)
            {
                sprintf(TXbuffer,"%02X",receivedData[0]);
                UART1_Write_String(TXbuffer);
                if(g_obdInfo.formatProperties.ats == 1)
                {
                    UART1_Write(' ');
                }
            }
            for (int i = 1;i <= receivedDatalen; i++)
            {
                sprintf(TXbuffer,"%02X",receivedData[i]);
                UART1_Write_String(TXbuffer);
                if(g_obdInfo.formatProperties.ats == 1)
                {
                            UART1_Write(' ');
                }
            }
        }
    }
//g_obdInfo.formatProperties.atcafFormatting = 1;
//                g_obdInfo.formatProperties.PCIRequest = 1;
//                g_obdInfo.formatProperties.PCIResponse = 0;
//                g_obdInfo.formatProperties.paddingResponse = 0;
//                g_obdInfo.formatProperties.RTRFrames = 0;
//                g_obdInfo.formatProperties.printFrameNumber = 1;
//                g_obdInfo.formatProperties.printFC = 1;
    // Now you can send this over CAN
    // e.g., canSend(OBD_REQUEST_ID, data, data_len);
}



