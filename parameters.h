/*
* @file        at.h
* @author      Aqib D. Ace 
* @date        March 2025
* @version     0.0.0
*/
#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <xc.h> 
#include "stdbool.h"
#include "string.h"
#include "stdio.h"
#include "can1.h"
void disableEcho(void);
void enableEcho(void);
bool getEchoStatus(void);
void disableLF(void);
void enableLF(void);
bool getLFStatus(void);
void readADCvolts(char* adcVolts, char* precession);
void saveCallibratedValues(void);
uint8_t readSavedCalibrationStatus(void);
uint8_t getProtocol(void);
uint8_t loadDefaultConfigurationOBD(void);
void SetCanPin(uint8_t protocol);

#define ELM327_VERSION_ID                       "ELM327 v1.4b"
#define OBD_VERSION_ID                          "STN2120 v5.6.5"
#define OBD_DEFAULT_SERIAL_NUMBER               "212010154303"


///////////////////
enum OBD_STATE
{
    OBD_STATE_LED_TEST = 0x00,
    OBD_STATE_SEND_STARTUP,
    OBD_STATE_SEND_PROMPT,
    OBD_STATE_IDLE,
    OBD_STATE_CMD_RUN
};

enum OBD_SUBSTATE
{
    OBD_SUBSTATE_IDLE = 0x00,
    OBD_SUBSTATE_STSBR_SEND_OK,
    OBD_SUBSTATE_STSBR_WAIT,
    OBD_SUBSTATE_STSBR_CHANGE_BAUDRATE,
    OBD_SUBSTATE_STBR_WAIT_BEFORE_STI,
    OBD_SUBSTATE_STBR_SEND_STI,
    OBD_SUBSTATE_STBR_WAIT4RESPONSE,
};

enum OBD_SLEEP_STATUS
{
    OBD_SLEEP_STATUS_IDLE = 0x00,
    OBD_SLEEP_STATUS_ACTIVE,
};

enum OBD_SLEEP_REASON{
    OBD_SLEEP_REASON_NONE,
    OBD_SLEEP_REASON_CMD,
    OBD_SLEEP_REASON_UART,
    OBD_SLEEP_REASON_EXT,
    OBD_SLEEP_REASON_VL // voltage level
};

enum OBD_SLEEP_WAKE{
    OBD_WAKE_REASON_NONE,
    OBD_WAKE_REASON_UART,
    OBD_WAKE_REASON_EXT,
    OBD_WAKE_REASON_VL, // voltage level
    OBD_WAKE_REASON_VCHG // voltage change
};




enum OBD_SLEEP_CONFIG_FLAGS
{
    OBD_SLEEP_CONFIG_FLAGS_UART_INACTIVITY_BIT = 0x01,
    OBD_SLEEP_CONFIG_FLAGS_UART_WAKE_BIT = 0x02,
};

enum OBD_AUTOFILTERING_STATE
{
    OBD_AUTOFILETRING_STATE_DISABLED = 0x00,
    OBD_AUTOFILETRING_STATE_ENABLED,
};

enum OBD_LONG_MESSAGE_STATE
{
    OBD_LONG_MESSAGE_STATE_DISABLED = 0x00,
    OBD_LONG_MESSAGE_STATE_ENABLED,
};

enum OBD_ATAT_STATE
{
    OBD_ATAT_STATE_ADAPTIVE_TIMING_OFF = 0x00,
    OBD_ATAT_STATE_ADAPTIVE_TIMING_NORMAL,
    OBD_ATAT_STATE_ADAPTIVE_TIMING_AGGRESSIV,
};

enum OBD_RESPONSE_STATE
{
    OBD_RESPONSE_STATE_ENABLED = 0x00,
    OBD_RESPONSE_STATE_DISABLED,
};

enum OBD_CAN_SEGMENATION_STATE
{
    OBD_CAN_SEGMENATION_STATE_DISABLED = 0x00,
    OBD_CAN_SEGMENATION_STATE_ENABLED,
};
enum CAN_SPEED_MODE
{
    CAN_SPEED_MODE_HIGH_500K = 0x00,
    CAN_SPEED_MODE_HIGH_250K,
    CAN_SPEED_MODE_MEDIUM_125K,
};

enum OBD_CAN_FLOWCONTROL_MODE
{
    OBD_CAN_FLOWCONTROL_MODE_AUTOMATIC = 0x00,
    OBD_CAN_FLOWCONTROL_MODE_USER_DEFINED_ID_DATA = 0x01,
    OBD_CAN_FLOWCONTROL_MODE_AUTOMATIC_ID_USER_DATA = 0x01,
};
enum OBD_CAN_MONITORING_MODE_STATE
{
    OBD_CAN_MONITORING_MODE_STATE_DISABLED = 0x00,
    OBD_CAN_MONITORING_MODE_STATE_ENABLED,
};

#define DEFAULT_UART_BAUDRATE 9600

#define OBD_ATPP_CONFIG_SIZE                    48

#define OBD_BLOCK_FILTER_CNT                    8
#define OBD_PASS_FILTER_CNT                     8
#define OBD_FLOWCONTROL_FILTER_CNT              8

#define OBD_FLOWCONTROL_PAIR_CNT                60//8
#define OBD_FLOWCONTROL_DATA_CNT                5

#define OBD_TIMEOUT_BUFFER_CNT                  4


#define IS_FIRST_FRAME                          1
#define IS_CONSECUTIVE_FRAME                    0


///////////////////////sssssssssssss//////////
typedef struct {
        uint8_t ath; // header
        uint8_t ats; // spaces
        uint8_t atd; // Data length code
        uint8_t PCIRequest; //Data length code for requests
        uint8_t PCIResponse;
        uint8_t paddingResponse;
        uint8_t RTRFrames;
        uint8_t printFC;
        uint8_t printFrameNumber; // serial number for iso-tp
        uint8_t atcafFormatting;
    } MessageFormatingProperties_t;
    
typedef struct
{
    uint8_t isActive;
    char sign;                   // sign like +/i/</<
    uint16_t triggerTime;        // time how long the level must be hold until condition is met
    //uint32_t conditionValidTime; // cumulated time to measure how long the level occured
    float triggerLevel;
} voltageLevelTrigger_t;

typedef struct /*obd_module_t*/
{
    uint8_t state;
    uint8_t substate;
    //ObdCommandInfo_t cmd;
    void * obdParameter;
    uint32_t timer;
    uint32_t uartBaudrate;
    uint32_t uartBaudrateTmp;
    uint8_t uartFlowControlFlag;
    uint8_t echoState;
    uint8_t linefeedState;
    uint16_t uartTimeout;
    
    MessageFormatingProperties_t  formatProperties;

    uint8_t canMode;
    uint32_t canSid;
    uint32_t canSidFilter;
    uint32_t canSidFilterMask;
    uint32_t canEid;
    uint32_t canEidFilter;
    uint32_t canEidFilterMask;
    uint8_t canHeader;

    uint8_t sleepStatus;
    
    uint32_t sleepTimer;
//    uint32_t sleepPeriod;
//    uint16_t wakeUartMinPulse;
//    uint16_t uartWakeMaxPulse;
//    voltageLevelTrigger_t sleepVoltage;
//    voltageLevelTrigger_t wakeVoltage;
//    voltageLevelTrigger_t levelChangeVoltage;
    uint32_t sleepVoltageConditionValidTime;
    uint32_t wakeVoltageConditionValidTime;
    uint32_t levelChangeVoltageConditionValidTime;
    
    

    //uint8_t sleepUartFlags;
    uint8_t sleepReason:4;
    uint8_t wakeReason:4;

    uint8_t calibrationFlag;
    uint8_t atiIdFlag;
    uint32_t voltageCalibration;
    uint32_t voltageOffset;
    uint16_t adcCalibration;

    uint16_t obdRequestTimeout;
    uint16_t obdTransmissionTimeout;
    uint16_t obdRequestPeriod;
    uint32_t obdRequestTimer;

    uint8_t obdAfState;
    uint8_t obdBlockFilterIdx;
    uint8_t obdPassFilterIdx;
    uint8_t obdFlowcontrolFilterIdx;
    uint32_t obdBlockFilter[OBD_BLOCK_FILTER_CNT][2];                 // [Pattern,Mask]
    uint32_t obdPassFilter[OBD_PASS_FILTER_CNT][2];                   // [Pattern,Mask]
    uint32_t obdFlowcontrolFilter[OBD_FLOWCONTROL_FILTER_CNT][2];     // [Pattern,Mask]
    
    uint8_t obdLongMessageState;
    uint8_t obdAtatModeState;
    uint8_t obdResponseState;
    uint8_t obdCanRxSegmentationState;
    uint8_t obdCanTxSegmentationState;

    uint16_t obdStctroFcTimeout;
    uint16_t obdStctroCfTimeout;

    float obdStminTime;

    uint8_t obdCanAddressingFormat;
    uint8_t obdCanTargetAddressExtension;
    uint8_t obdFlowcontrolPairIdx;
    uint32_t obdFlowcontrolPairs[OBD_FLOWCONTROL_PAIR_CNT][4];                 // [Address1,Ext1,Address2,Ext2]
    uint8_t obdFlowcontrolData[OBD_FLOWCONTROL_DATA_CNT];
    uint32_t obdFlowcontrolHeader;
    uint8_t obdFlowcontrolMode;
    uint8_t obdFlowcontrolActive;
    uint8_t obdFlowcontrolDataFlag;
    uint8_t obdFlowcontrolHeaderFlag;
    
    uint8_t protocolbSettings;
    uint8_t protocolbBaudrate;

    uint8_t obdCanVariableDlc;
    uint8_t obdCanMonitoringMode;
    uint8_t obdCanDisplayDlc;

    uint32_t obdTimeoutBuffer[OBD_TIMEOUT_BUFFER_CNT];
    uint8_t obdTimeoutIdx;
    
    uint16_t LastReceivedAdressSid; // needed for transport protocoll
    uint32_t LastReceivedAdressEid;
    
    uint8_t powerControlMode[20];
} obdStateInfo_t;


typedef struct
{
    uint8_t preamble[9];
    uint32_t usartBaudrate;
    uint8_t atppConfig[OBD_ATPP_CONFIG_SIZE][2];
    uint8_t atiId[32];
    uint8_t descriptionString[48];
    uint8_t protocol;
    uint8_t testerAddress;

    uint8_t sleepUartFlags;
    uint32_t uartInactivityPeriod;
    uint16_t wakeUartMinPulse;
    uint16_t wakeUartMaxPulse;
    uint8_t extSleepPinPolarity;
    uint8_t extSleepSleep;
    uint8_t extSleepWake;
    uint16_t extSleepInactivityTime;
    uint16_t extSleepWakeTime;
    voltageLevelTrigger_t sleepVoltage;
    voltageLevelTrigger_t wakeVoltage;
    voltageLevelTrigger_t levelChangeVoltage;
    uint8_t pwrCtrlPinPolarity;
    uint8_t dummy[8];
} obdConfig_t;

typedef struct 
{
    uint8_t preamble[9];
    uint8_t hardwareIdFlag;
    uint8_t hardwareId[48];

    uint32_t powerOnResetCount;
    uint8_t serialNumberIdFlag;
    char serialNumberId[13];
    uint8_t deviceIdentifierFlag;
    uint8_t deviceIdentifier[13];

    uint8_t calibrationFlag;
    uint32_t voltageCalibration;
    uint32_t voltageOffset;
    uint16_t adcCalibration;
} obdOtpConfig_t;



extern volatile obdStateInfo_t g_obdInfo;
extern volatile obdConfig_t g_obdConfig;
extern volatile obdOtpConfig_t g_obdOtpConfig;
#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */



#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

