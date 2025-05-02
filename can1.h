 
#ifndef __CAN_H
#define	__CAN_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "stdbool.h"
#define FCAN    20267500UL
#define BITRATE 250000UL  
#define NTQ     10  // 10 Time Quanta in a Bit Time
#define BRP_VAL ( (FCAN / (2 * NTQ * BITRATE)) - 1  )

#define CAN_BUFFER_CNT              16                  // must be power of 2 

#define OPTION_NOP                  0x0
#define OPTION_IS_FLOW_CONTROL      0x01

#define DEFAULT_SID_CAN_FILTER_MASK 0xfff
// TODO: check if this is the correct filter value 0x1fffffff for EID
#define DEFAULT_EID_CAN_FILTER_MASK 0x00000000
//#define SID_FLOW_CONTROL_EXT_EQUIP  0x7E0

#define DEFAULT_SID_CAN_FILTER 0x07E8
#define DEFAULT_EID_CAN_FILTER 0x18DAF111

#define CAN_BAUDRATE_HIGH_SPEED_500K    500000
#define CAN_BAUDRATE_HIGH_SPEED_250K    250000
#define CAN_BAUDRATE_MEDIUM_SPEED_125K  125000

#define COUNT_BYTES_8_DLC           8
#define MAX_CAN_FRAMES_SIZE         8
#define MAX_CAN_MESSAGE_DATA_LENGTH 16

#define CAN_FIXED_DLC                  0x80
#define CAN_EXTENDED_ID_FORMAT         0x01

#define CAN_FRAME_DATA_LEN 8
#define ISO_TP_MAX_MSG_LEN 4095

// PCI types
#define ISO_TP_FRAME_TYPE_SF 0x0     // Single Frame
#define ISO_TP_FRAME_TYPE_FF 0x1     // First Frame
#define ISO_TP_FRAME_TYPE_CF 0x2     // Consecutive Frame
#define ISO_TP_FRAME_TYPE_FC 0x3     // Flow Control

// Flow Control Types
#define ISO_TP_FLOW_STATUS_CTS 0x00  // Continue To Send
#define ISO_TP_FLOW_STATUS_WAIT 0x01
#define ISO_TP_FLOW_STATUS_OVERFLOW 0x02

typedef
enum {
    CAN_TP_SINGLE_FRAME = 0,
    CAN_TP_FIRST_FRAME = 1,
    CAN_TP_CONSECUTIVE_FRAME = 2,
    CAN_TP_FLOW_CONTROL_FRAME = 3,
} CanTPFrameType_t;

enum CAN_MODE {
    CAN_MODE_STANDARD_DATA_FRAME_VAR_DLC = 0x00,
    CAN_MODE_EXTENDED_DATA_FRAME_VAR_DLC = CAN_EXTENDED_ID_FORMAT,
    CAN_MODE_STANDARD_DATA_FRAME_8_DLC = CAN_FIXED_DLC ,
    CAN_MODE_EXTENDED_DATA_FRAME_8_DLC = CAN_FIXED_DLC | CAN_EXTENDED_ID_FORMAT,
};

//enum CAN_MODE {
//    CAN_MODE_STANDARD_DATA_FRAME_VAR_DLC = 0x00,
//    CAN_MODE_EXTENDED_DATA_FRAME_VAR_DLC = 0x01,
//    CAN_MODE_STANDARD_DATA_FRAME_8_DLC = 0x80,
//    CAN_MODE_EXTENDED_DATA_FRAME_8_DLC = 0x81,
//};

typedef struct {
    uint8_t data[CAN_BUFFER_CNT][16];
    uint8_t ridx;
    uint8_t widx;
} canRingBufferInfo_t;

extern volatile canRingBufferInfo_t g_canRingBufferInfo;


#define GET_CAN_SID_X____(DATA_ARR) \
    (((uint16_t)((((uint16_t)(DATA_ARR)[1]) << 2) | (((uint16_t)(DATA_ARR)[0] >> 6) & 0x03)) & 0xFFF) << 4) | (((uint16_t)(DATA_ARR)[0] >> 2) & 0x0F)

#define GET_CAN_SID___V1(DATA_ARR) \
    (((uint16_t)((((uint16_t)(DATA_ARR)[1]) << 2)) & 0xFFF) << 4) | (((uint16_t)(DATA_ARR)[0] >> 2) & 0x7F)  

#define GET_CAN_SID(DATA_ARR) \
    (((uint16_t)((((uint16_t)(DATA_ARR)[1]) << 6)) & 0xFC0)) | (((uint16_t)(DATA_ARR)[0] >> 2) & 0x7F)  

#define GET_CAN_EID__OLD(myeid) ( \
    ((uint32_t)(myeid[1]) << 24) | \
    ((uint32_t)(myeid[0] & 0xFE) << 16) | \
    ((uint32_t) ((((uint32_t) myeid[3] >> 2) & 0xC0) |  (  (  ((uint32_t)myeid[2]) >> 2) & 0x3F)) << 8) | \
    ((uint32_t)(((myeid[5] >> 2) & 0x3F) | ((myeid[2] & 0x03) << 6))) \
)

#define GET_CAN_EID_31(myeid) ( \
    ((uint32_t)(myeid[1]) << 24) | \
    ((uint32_t)(myeid[0] & 0xFE) << 16) | \
    ((uint32_t) ((((uint32_t)myeid[3]>>2)&0xC0) |  (((uint32_t)myeid[2]>>2)&0xFf)) << 8) | \
    ((uint32_t)(((myeid[5] >> 2) & 0x3F) | ((myeid[2] & 0x03) << 6))) \
)
#define GET_CAN_EID(myeid) ( \
    ((uint32_t)(myeid[1]) << 24) | \
    ((uint32_t)(myeid[0] & 0xFE) << 16) | \
    (((uint32_t) ((((myeid[3])<<8)&0xff00) | ((myeid[2])&0xff)) << 6)&0xff00 ) | \
    ((uint32_t)(((myeid[5] >> 2) & 0x3F) | ((myeid[2] & 0x03) << 6))) \
)

// Request to switch the CAN controller to configuration mode and wait until it is done
#define SWITCH_TO_CAN_CONFIG_MODUS  if(C1CTRL1bits.OPMODE != 4) {C1CTRL1bits.REQOP = 4; while (C1CTRL1bits.OPMODE != 4);  }
// Request Normal Operation mode and Wait for CAN module to switch to Normal Operation mode
#define EXIT_CAN_CONFIG_MODUS  if(C1CTRL1bits.OPMODE != 0) {C1CTRL1bits.REQOP = 0; while (C1CTRL1bits.OPMODE != 0);  }

#define CAN_FUNCTIONS_INTERFACE

#ifdef CAN_FUNCTIONS_INTERFACE

void initCAN(void);
uint8_t txMessageCAN(uint32_t sid, uint32_t eid, uint8_t mode, uint8_t * data, uint8_t size, uint16_t timeout);
uint8_t txRemoteCAN(uint32_t sid, uint32_t eid, uint8_t mode, uint16_t timeout);
void setBaudrateCAN(uint32_t baudrate);
uint32_t getBaudrateCAN(void);
uint8_t getMessageCAN(uint8_t * data);
uint8_t dataAvailableCAN(void);
void flushBufferCAN(void);
void printMessageCAN(uint8_t buffer);
void configureFilterCAN(uint8_t mode, uint32_t sid, uint32_t eid, uint32_t sid_mask, uint32_t eid_mask);
void getErrorCountCAN(uint8_t * rx, uint8_t * tx);
void iso_tp_send(uint32_t sid, uint32_t eid, uint8_t mode, uint8_t data[], uint16_t size, uint16_t timeout);
bool iso_tp_receive(uint8_t mode, uint8_t* data, uint8_t* size, uint16_t timeout ,uint32_t sid, uint32_t eid);
bool canReceive(uint8_t mode, uint8_t *data, uint8_t *datalen, uint16_t timeout);
#endif

void setCANTimConReg(uint32_t timeValue);
uint32_t getCANTimConReg(void);


#endif	/* XC_HEADER_TEMPLATE_H */


  