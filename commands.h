/**
* @file        commands.h
* @author      Aqib D. Ace 
* @date        March 2025
* @version     0.0.0
*/

#ifndef COMMANDS_H
#define	COMMANDS_H

#include <xc.h> // include processor files - each processor file is guarded.  
void processCommand(char *command); //verify and filters the command
void processSTCommand(char *command); //process ST command
void processATCommand(char *command); //process AT command
void processOBDRequest(char *command);//process OBD request
void loadDefaultOnWarmReset(void); //load default values on warm reset
void resetAllNVMToDefault(void); //reset all value to default on NVM
#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

