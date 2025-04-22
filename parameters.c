/**
* @file        at.c
* @author      Aqib D. Ace 
* @date        March 2025
* @version     0.0.0
*/

#include "xc.h"
#include "parameters.h"


bool uartEcho = true;
bool uartLineFeed = false;
uint32_t voltage_calibration = 12345;
uint32_t voltage_offset = 670;
uint32_t analog_in_voltage;
uint32_t analog_in_steps;
bool stvcalrun = false;
bool stsavcalrun = false;

volatile obdStateInfo_t g_obdInfo;
volatile obdConfig_t g_obdConfig;
volatile obdOtpConfig_t g_obdOtpConfig;

char ELM327_VERSION[32];

uint8_t loadDefaultConfigurationOBD(void) {
    g_obdInfo.uartTimeout = 1000; // actually it should be 75ms according spec - we use 1000 ms
    g_obdInfo.echoState = 1;
    g_obdInfo.linefeedState = 0;
    g_obdInfo.formatProperties.ath = 0;
    g_obdInfo.formatProperties.ats = 1;
    g_obdInfo.formatProperties.atd = 0;
    g_obdInfo.formatProperties.atcafFormatting = 1;
    g_obdInfo.formatProperties.PCIRequest = 1;
    g_obdInfo.formatProperties.PCIResponse = 0;
    g_obdInfo.formatProperties.paddingResponse = 0;
    g_obdInfo.formatProperties.RTRFrames = 0;
    g_obdInfo.formatProperties.printFrameNumber = 1;
    g_obdInfo.formatProperties.printFC = 1;
    


    g_obdInfo.canMode = CAN_MODE_STANDARD_DATA_FRAME_8_DLC;
    g_obdInfo.canSid = 0x07DF;
    g_obdInfo.canEid = 0x18DB33F1;
    g_obdInfo.obdFlowcontrolHeader = 0x07DF;

    g_obdInfo.canSidFilter = DEFAULT_SID_CAN_FILTER;
    g_obdInfo.canEidFilter = DEFAULT_EID_CAN_FILTER;
    g_obdInfo.canSidFilterMask = DEFAULT_SID_CAN_FILTER_MASK;
    //g_OBD_INFO.can_eid_filter_mask = 0x1fffffff;// TODO : change back to this
    g_obdInfo.canEidFilterMask = DEFAULT_EID_CAN_FILTER_MASK;
    g_obdInfo.sleepStatus = OBD_SLEEP_STATUS_IDLE;
    g_obdConfig.sleepUartFlags = OBD_SLEEP_CONFIG_FLAGS_UART_WAKE_BIT;

    g_obdInfo.obdAfState = OBD_AUTOFILETRING_STATE_DISABLED;
    g_obdInfo.obdBlockFilterIdx = 0;
    g_obdInfo.obdPassFilterIdx = 0;
    g_obdInfo.obdFlowcontrolFilterIdx = 0;

    g_obdInfo.obdLongMessageState = OBD_LONG_MESSAGE_STATE_DISABLED;
    g_obdInfo.obdAtatModeState = OBD_ATAT_STATE_ADAPTIVE_TIMING_NORMAL;
    g_obdInfo.obdResponseState = OBD_RESPONSE_STATE_ENABLED;
    g_obdInfo.obdStminTime = 0;
    g_obdInfo.obdCanAddressingFormat = 0;
    g_obdInfo.obdCanTargetAddressExtension = 0;
    g_obdInfo.obdFlowcontrolMode = OBD_CAN_FLOWCONTROL_MODE_AUTOMATIC;
    g_obdInfo.obdFlowcontrolActive = 1;
    
    disableLF();
    enableEcho();
    return 0;
}

void initDefaultAT(void)
{
    strcpy(ELM327_VERSION, "ELM327 v1.4b");
}

void disableEcho(void)
{
    g_obdInfo.echoState = false;
}
void enableEcho(void)
{
    g_obdInfo.echoState = true;
}
bool getEchoStatus(void)
{
    return g_obdInfo.echoState;
}
void disableLF(void)
{
   g_obdInfo.linefeedState = false;
}
void enableLF(void)
{
    g_obdInfo.linefeedState = true;
}
bool getLFStatus(void)
{
    return g_obdInfo.linefeedState;
}
void calibrateVoltage(char* argument)
{
    stvcalrun = true;
    uint32_t volts;
    uint32_t offset;
    sscanf(argument, "%lf,%lf", &volts, &offset);
    voltage_calibration = volts;
    voltage_offset = offset;
}
void readADCvolts(char* adcVolts, char* precession)
{
    //read the ANALOG_IN pin here and return the actual  volts
    //analog_in_voltage = adc_read();
    //uint8_t precess = strtoul(precession, NULL, 10);
    //sprintf(adcVolts,"%u.",analog_in_voltage);
    
}
uint16_t readADCSteps(void)
{
    //read the ANALOG_IN pin here and return the ADC steps
}

void saveCallibratedValues(void)
{
    stsavcalrun = true;
}
uint8_t readSavedCalibrationStatus(void)
{
    if (stvcalrun == true && stsavcalrun == true)
    {
        return 1;
    }
    else if (stvcalrun == false)
    {
        return 2;
    }
    else if (stvcalrun == true && stsavcalrun == false)
    {
        return 3;
    }
    else
    {
        return 0;
    }
}
void setAllSettoDefault(void)
{
    loadDefaultConfigurationOBD();
    //loadCustomConfigurationOBD(PROGRAMMABLE_PARAMETERS_TYPE_D);  //TODO: add D type
    loadCustomConfigurationOBD();
}
uint8_t getProtocol(void)
{
    return g_obdConfig.protocol;
}


