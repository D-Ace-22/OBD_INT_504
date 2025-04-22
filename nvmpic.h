/**
* @file        nvmpic.h
* @author      Aqib D. Ace 
* @date        March 2025
* @version     0.0.0
*/
#ifndef FLASH_H
#define FLASH_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include "parameters.h"
#include <stdio.h>
#include <string.h>

#define FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS 1024U

#define FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS  (FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS*2U)

#define FLASH_UNLOCK_KEY 0x00AA0055

#define FLASH_ERASE_PAGE_MASK (~((FLASH_ERASE_PAGE_SIZE_IN_INSTRUCTIONS*2UL) - 1U)) 

enum
{
    PROGRAMMABLE_PARAMETERS_TYPE_P = 0x00,
    PROGRAMMABLE_PARAMETERS_TYPE_R,
    PROGRAMMABLE_PARAMETERS_TYPE_D,
    PROGRAMMABLE_PARAMETERS_TYPE_I,
} PROGRAMMABLE_PARAMETERS_TYPE;
const uint8_t c_def_programmable_parameters[48][3] = 
{
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0x19,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0x01,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_I},
    {0xF1,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0x09,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_I},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_I},
    {0x00,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0x0A,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
    {0x23,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_P},
    {0x0D,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
    {0x5A,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
    {0x0D,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_I},
    {0x00,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0x00,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
    {0xF4,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_I},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
    {0x0A,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
    {0x92,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0x00,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_I},
    {0x28,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_I},
    {0x0A,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0x0A,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0x00,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0x00,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0x00,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0xff,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0xFF,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0x04,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_D},
    {0x02,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
    {0xE0,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
    {0x04,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
    {0x80,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
    {0x0A,0xFF,PROGRAMMABLE_PARAMETERS_TYPE_R},
};

void     FLASH_Unlock(uint32_t  key);
void     FLASH_Lock(void);

bool     FLASH_ErasePage(uint32_t address);    

uint16_t FLASH_ReadWord16(uint32_t address);
uint32_t FLASH_ReadWord24(uint32_t address);

bool     FLASH_WriteDoubleWord16(uint32_t flashAddress, uint16_t Data0, uint16_t Data1);
bool     FLASH_WriteDoubleWord24(uint32_t address, uint32_t Data0, uint32_t Data1  );


uint16_t FLASH_GetErasePageOffset(uint32_t address);
uint32_t FLASH_GetErasePageAddress(uint32_t address);
//void WordWriteExample(int x);
void printPParameters(void); //this function will print the programmable parameters saved in current memory.
void restoreDefaultPParameters(void); //this function will save the default programmable parameters in memory.
//uint32_t returnread(int i);
void getAddress(void);
void printAllSaved(void);
void printSaved(char* command);
void configurePPs(char* argument);
void enablePP(uint8_t serial);
void disablePP(uint8_t serial);
void changePP(uint8_t xx, uint8_t yy);
uint8_t hexDigitToDecimal(char hex);
uint8_t hexDigitsToDecimal(char x, char y);
uint8_t updateCustomConfigurationOBD(void);
uint8_t loadCustomConfigurationOBD(uint8_t reason);
uint8_t restoreCustomConfigurationOBD(void);
uint8_t loadOtpConfigurationOBD(void);
uint8_t restoreOtpConfigurationOBD(void);
uint8_t updateOtpConfigurationOBD(void);
#endif	/* FLASH_H */

