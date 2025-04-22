#include "can1.h"
#include "delay.h"
#include "mcc_generated_files/uart1.h"
#include "parameters.h"
#define TEST1
#ifdef TEST

#ifdef CAN_FUNCTIONS_INTERFACE







/**
 * Initializes the CAN (Controller Area Network) module on the dsPIC33EP256GP504 microcontroller. 
 * This setup involves configuring the CAN receive and transmit pins, setting the module to operate at 250 kbps, 
 * configuring the phase segments, and setting up the DMA channels for transmission and reception.
 * 
 * The function also includes conditional configurations for CAN filters and masks, and sets the CAN module
 * to Normal Operation mode after the initialization is complete.
 * Look here for DMA config:
 * http://ww1.microchip.com/downloads/en/devicedoc/70348c.pdf
 * 
 */
void initCAN(void) {
    g_canRingBufferInfo.ridx = 0x00;
    g_canRingBufferInfo.widx = 0x00;

    // Configure the CAN receive and transmit pins using remappable pin functions.
    RPINR26 = 0; //clear register
    RPINR26bits.C1RXR = 0x2C; //set CAN1 RX to RPI96    (pin58)
    RPOR4bits.RP42R = 14; // set CAN1TX to RP57        (pin55)

    /* Set up the ECAN1 module to operate at 250 kbps. The ECAN module should be first placed
    in configuration mode. */
    C1CTRL1bits.REQOP = 4; // Request configuration mode.
    while (C1CTRL1bits.OPMODE != 4); // Wait until configuration mode is active.
    C1CTRL1bits.WIN = 1; // Enable window to access CAN configuration registers.
    /* Set up the CAN module for 250kbps speed with 10 Tq per bit. */
    C1CFG1bits.SJW = 0x1; // Synchronization Jump Width is set to 1xTQ
    /* Baud Rate Prescaler */
    C1CFG1bits.BRP = BRP_VAL; // Set Baud Rate Prescaler to achieve 250kbps.
    C1CFG2 = 0x2D2; // Configure Phase Segments and Propagation Time Segment
    C1FCTRL = 0xC01F; // No FIFO, 32 Buffers Set up FIFO, assign buffers as FIFO or as dedicated buffers.




    if (0) // maybe used for tests
    {
        C1FMSKSEL1bits.F0MSK = 0x0;
        C1RXM0SIDbits.SID = 0x07F8;
        C1RXF0SIDbits.SID = 0x07E8;

        C1RXM0SIDbits.MIDE = 0x1;
        C1RXF0SIDbits.EXIDE = 0x0;
        C1BUFPNT1bits.F0BP = 0xA;
        C1FEN1bits.FLTEN0 = 0x1;
        C1CTRL1bits.WIN = 0x0;
    } else if (1) {

        C1RXM1SID = 0xFFFF; // Sets the mask for standard and extended ID to accept all messages.
        C1RXM1EID = 0xFFFF; // Sets the extended ID mask to all 1s (accept all).
        /* C1RXF2SID = ((0x18DA << 2) | 0x03; */
        // sid << 5 | 0x08 | ((eid >> 16) & 0x03)
        /* C1RXF2SID = ((0x636) <<5) | (0x0A) ; */
        C1RXF2SID = ((0x636) << 5) | (0x0A); // Sets the standard ID for the filter with bit manipulation.
        C1RXF2EID = 0xF111; // Sets the extended ID for the filter.
        C1BUFPNT1bits.F2BP = 0xA; // Assigns this filter to a specific buffer.
        C1FEN1 = 0; // Turns off all filters.
        C1FEN1bits.FLTEN2 = 0x1; // Specifically enables Filter 2.
        C1FMSKSEL1bits.F2MSK = 0x1; // Selects the mask type for Filter 2.
        C1CTRL1bits.WIN = 0x0; // Closes the filter configuration window.
    }
    uint32_t sid = 0x636;
    uint32_t eid = 0x2F111;
    //printDEBUG(DSYS, "SID[%xw]R[%x]EID[%xw]\n", sid, C1RXF2SID, (eid >> 16) & 0x03);
    //printDEBUG(DSYS, "EID[%xw]R[%x]\n", (eid & 0xffff), C1RXF2EID);

    /* Configure Message Buffer 0 for Transmission and assign priority */
    C1TR01CONbits.TXEN0 = 0x1;
    C1TR01CONbits.TX0PRI = 0x3;


    /* Assign 32x8word Message Buffers for ECAN1 in device RAM. This example uses DMA0 for TX.
    Refer to 21.8.1 ?DMA Operation for Transmitting Data? for details on DMA channel
    configuration for ECAN transmit. */
    // CAN TX -Transimt
    DMA0CONbits.SIZE = 0x0;
    DMA0CONbits.DIR = 0x1;
    DMA0CONbits.AMODE = 0x2; // DMA Addressing Mode: Peripheral Indirect Addressing mode
    DMA0CONbits.MODE = 0x0;
    DMA0REQ = 70; // continuos mode / Ping-Pong modes disabled Set DMA0 Request Source to ECAN1 Transmit
    DMA0CNT = 7; // '7' DMA will perform 8 transfers
    DMA0PAD = (volatile unsigned int) &C1TXD; // ECAN1 Transmit Data Register (C1TXD)
    DMA0STAL = (unsigned int) &ecanTx1MsgBuf; // DMA Start Address Low Register
    DMA0STAH = (unsigned int) &ecanTx1MsgBuf; // DMA Start Address High Register
    DMA0CONbits.CHEN = 0x1; // enable this channel

    //CAN-Rx
    DMA1CONbits.SIZE = 0x0;
    DMA1CONbits.DIR = 0x0;
    DMA1CONbits.AMODE = 0x2; // Set Addressing Mode to Peripheral Indirect Addressing mode.
    DMA1CONbits.MODE = 0x0;
    DMA1REQ = 34; // Set DMA Request Source to the CAN1 receive event.
    DMA1CNT = 7; // Set the number of DMA transfers per DMA request to 8 (0 to 7)
    DMA1PAD = (volatile unsigned int) &C1RXD;
    //printDEBUG(DSYS, "Address [%x]", &ecanTx1MsgBuf);
    //printDEBUG(DSYS, "Address [%x]", &ecanRx2MsgBuf);
    DMA1STAL = (uint16_t) (&ecanRx2MsgBuf);
    DMA1STAH = (uint16_t) (&ecanRx2MsgBuf);
    //DMA1STAL = ((uint16_t) &ecan2MsgBuf) & 0xffff;
    //DMA1STAH = (((uint16_t) &ecan2MsgBuf) >> 16) & 0xffff;
    DMA1CONbits.CHEN = 0x1;


    IEC2bits.C1IE = 1; // Enable CAN1 interrupts.  
    C1INTE |= 0x02;
    /* At this point the ECAN1 module is ready to transmit a message. Place the ECAN module in
    Normal mode. */
    //    C1CTRL1bits.REQOP = 0; // Request Normal Operation mode.
    //    while (C1CTRL1bits.OPMODE != 0); // Wait for CAN module to switch to Normal Operation mode
    EXIT_CAN_CONFIG_MODUS;
}

/**
 * Configures the CAN filter and mask settings for the dsPIC33EP256GP504 microcontroller's CAN module. 
 * This function allows customization of the CAN filtering based on the mode and ID parameters provided. 
 * It includes configurations for both standard and extended data frames.
 *
 * @param mode The mode of the CAN filter, determining the type of data frames to filter (standard or extended).
 * @param sid Standard Identifier for the CAN message.
 * @param eid Extended Identifier for the CAN message.
 * @param sid_mask Mask for the Standard Identifier.
 * @param eid_mask Mask for the Extended Identifier.
 * 
 */
void configureFilterCAN(uint8_t mode, uint32_t sid, uint32_t eid, uint32_t sid_mask, uint32_t eid_mask) {

    //    C1CTRL1bits.REQOP = 4;
    //    while (C1CTRL1bits.OPMODE != 4); // Wait until the mode is active
    SWITCH_TO_CAN_CONFIG_MODUS;

    // Enable configuration window for the filters
    C1CTRL1bits.WIN = 0x1;

    // Decision based on the mode (Standard or Extended)
    switch (mode) {
            // For Standard Data Frames
        case(CAN_MODE_STANDARD_DATA_FRAME_VAR_DLC):
        case(CAN_MODE_STANDARD_DATA_FRAME_8_DLC):
        {
            // Select filter for Standard ID
            C1FMSKSEL1bits.F0MSK = 0x0;

            //printDEBUG(DWARNING, "Set normal format [%xw][%xw]\n", sid, sid_mask);
            // Configuration of mask and filter registers for Standard ID
            C1RXM0SIDbits.SID = sid_mask;
            C1RXF0SIDbits.SID = sid;

            // Configuration to accept only standard messages
            C1RXM0SIDbits.MIDE = 0x1; // Ensures only standard ID messages are accepted.
            C1RXF0SIDbits.EXIDE = 0x0; // Ignores extended IDs

            C1BUFPNT1bits.F0BP = 0xA; // Assigns this filter to a specific buffer

            C1FEN1bits.FLTEN0 = 0x1; // Enabling the filter

            // Closing the configuration window
            C1CTRL1bits.WIN = 0x0;
            break;
        }
            // For Extended Data Frames
        case(CAN_MODE_EXTENDED_DATA_FRAME_VAR_DLC):
        case(CAN_MODE_EXTENDED_DATA_FRAME_8_DLC):
        {
            C1RXF0SIDbits.EXIDE = 0x1; // accept extended IDs
            C1RXM0SIDbits.MIDE = 0x0; // dont accept standard ID messages 

            // Extracting Standard ID and Extended ID from the given EID
            sid = (eid >> 18);
            eid = (eid & 0x3ffff);
            // Extracting masks for SID and EID
            sid_mask = (eid_mask >> 18) & 0x7ff;
            eid_mask = (eid_mask & 0x3ffff);

            //printDEBUG(DWARNING, "Set extended format [%xw][%xw]\n", eid, eid_mask);
            // Configuration of mask and filter registers for Extended ID
            C1RXM1SID = (sid_mask << 5) | ((eid_mask >> 16) & 0x03);
            C1RXM1EID = eid_mask & 0xffff;
            C1RXF2SID = ((sid) << 5) | (0x0A) | ((eid >> 16) & 0x03);
            C1RXF2EID = eid & 0xffff;

            // Assigning the filter to a buffer
            C1BUFPNT1bits.F2BP = 0xA;
            // Deactivating all filters except the selected one
            C1FEN1 = 0;
            // Enabling the filter
            C1FEN1bits.FLTEN2 = 0x1;
            // Selecting the filter for Extended ID
            C1FMSKSEL1bits.F2MSK = 0x1;

            // Closing the configuration window
            C1CTRL1bits.WIN = 0x0;
            break;
        }
    }
    // Return to normal mode
    //    C1CTRL1bits.REQOP = 0;
    //    while (C1CTRL1bits.OPMODE != 0); // Wait until the normal mode is active
    EXIT_CAN_CONFIG_MODUS;
}

// Function to send a CAN message.
// Parameters:
// - sid: Standard Identifier for CAN message.
// - eid: Extended Identifier for CAN message.
// - mode: Type of CAN message, such as standard data frame or extended data frame.
// - data: Pointer to the data to be sent.
// - size: Size of the data in bytes.
// - timeout: Time to wait for the message to be sent before timing out.
uint8_t txMessageCAN(uint32_t sid, uint32_t eid, uint8_t mode, uint8_t * data, uint8_t size, uint16_t timeout) {
    
    //printDEBUG(0,"### txMessageCAN SID = %xw EID %xw \n", sid,eid);
    switch (mode) {
        case(CAN_MODE_STANDARD_DATA_FRAME_VAR_DLC):
        {
            ecanTx1MsgBuf[0][0] = (sid << 2) | (mode & 0x03);
            ecanTx1MsgBuf[0][1] = 0x0000;
            ecanTx1MsgBuf[0][2] = size & 0xf;
            break;
        }
        case(CAN_MODE_EXTENDED_DATA_FRAME_VAR_DLC):
        {
            /* g_OBD_INFO.can_eid = 0x18DB33F1; */
//            uint32_t sid = (g_OBD_INFO.can_eid >> 16) & 0xfffC;
//            uint32_t eid = g_OBD_INFO.can_eid & 0x3fffF;
            sid = (sid) & 0xffff;
            eid = eid & 0x3fffF;            
            ecanTx1MsgBuf[0][0] = (sid) | 0x01;
            ecanTx1MsgBuf[0][1] = eid >> 6;
            ecanTx1MsgBuf[0][2] = ((eid & 0x3f) << 10) | 0x08;
            break;
        }
        case(CAN_MODE_STANDARD_DATA_FRAME_8_DLC):
        {
            ecanTx1MsgBuf[0][0] = (sid << 2) | (mode & 0x03);
            ecanTx1MsgBuf[0][1] = 0x0000;
            ecanTx1MsgBuf[0][2] = size & 0xf;
            break;
        }
        case(CAN_MODE_EXTENDED_DATA_FRAME_8_DLC):
        {
            /* g_OBD_INFO.can_eid = 0x18DB33F1; */
//            uint32_t sid = (g_OBD_INFO.can_eid >> 16) & 0xfffC;
//            uint32_t eid = g_OBD_INFO.can_eid & 0x3fffF;
            sid = (sid) & 0xffff;
            eid = eid & 0x3fffF;            
            ecanTx1MsgBuf[0][0] = (sid) | 0x01;
            ecanTx1MsgBuf[0][1] = eid >> 6;
            ecanTx1MsgBuf[0][2] = ((eid & 0x3f) << 10) | 0x08;
            break;

        }


    }
    
    uint8_t k = 0;
    uint8_t n = 0;
    for (n = 0; n < 4; n++) {
        if (k < size) {
            ecanTx1MsgBuf[0][n + 3] = data[k + 1];
            ecanTx1MsgBuf[0][n + 3] = (ecanTx1MsgBuf[0][n + 3] << 8) | data[k];
        } else {
            ecanTx1MsgBuf[0][n + 3] = 0x0000;
        }
        k += 2;
    }

    // Request the CAN module to send the message.
    C1TR01CONbits.TXREQ0 = 0x1; // Set the TXREQ0 bit to 1 to start transmission of buffer 0.
    //printDEBUG(DSYS, "Send CAN message [%xw][%xw].... ", sid, eid);
    for (k = 0; k < 8; k++) {
        //printDEBUG(DAPPEND, "%x ", ecanTx1MsgBuf[0][k]);

    }
    //printDEBUG(DAPPEND, "\n");
    uint32_t timer = getSYSTIM();
    //while(C1TR01CONbits.TXREQ0 == 1) // While transmission request bit is set
    while ((C1TR01CONbits.TXREQ0 == 1) && (chk4TimeoutSYSTIM(timer, timeout) != SYSTIM_TIMEOUT)) // While transmission request bit is set  
    {
        if (chk4TimeoutSYSTIM(timer, timeout) == SYSTIM_TIMEOUT) {
            C1TR01CONbits.TXREQ0 = 0x0; // Clear the transmission request bit.
            return 1; // Return 1 to indicate a timeout occurred.
        }
    }
    /* Message was placed successfully on the bus */
    //printDEBUG(DAPPEND, " DONE\n");
    return 0;
}

/*
 * receives from CAN bus
 * 
 */
uint8_t getMessageCAN(uint8_t * data) {
    if (g_canRingBufferInfo.ridx == g_canRingBufferInfo.widx) {
        return 0;
    }
    uint8_t k = 0;
    //printMessageCAN(0xa);
    for (k = 0; k < 16; k++) {
        data[k] = g_canRingBufferInfo.data[g_canRingBufferInfo.ridx][k];
    }
    g_canRingBufferInfo.ridx = (g_canRingBufferInfo.ridx + 1) & (CAN_BUFFER_CNT - 1);
    return k;
}

uint8_t dataAvailableCAN(void) {
    if (g_canRingBufferInfo.ridx == g_canRingBufferInfo.widx) {
        return 0;
    } else {
        return 1;
    }

}

void flushBufferCAN(void) {
    g_canRingBufferInfo.widx = 0x00;
    g_canRingBufferInfo.ridx = 0x00;
}

void printMessageCAN(uint8_t buffer) {
    uint8_t k = 0;
    //printDEBUG(0, "\n========================\n");

    uint32_t eid = ecanRx2MsgBuf[buffer][1] << 6;
    eid |= ((ecanRx2MsgBuf[buffer][2]) >> 10);
    
    //printDEBUG(0, "EID TEST ==> %x -- %x\n", ecanRx2MsgBuf[buffer][1], ecanRx2MsgBuf[buffer][1] << 6 );
    //printDEBUG(0, "EID TEST ==> High %x  Low %x\n", ((ecanRx2MsgBuf[buffer][1]>>8)&0xff) , ecanRx2MsgBuf[buffer][1]&0xff );
    //printDEBUG(0, "EID ECAN ==> %xw\n", eid);

    //printDEBUG(DSYS, "Received data [%x][%x]:\n", DMA1STAL, DMA1STAH);
    uint8_t n = 0;
    //printDEBUG(DAPPEND, "[%d] ", n);
    for (k = 0; k < 8; k++) {
        //printDEBUG(DAPPEND, "%x ", ecanRx2MsgBuf[buffer][k]);
    }
    //printDEBUG(DAPPEND, "\n");
    //printDEBUG(0, "\n========================\n");
}

void getErrorCountCAN(uint8_t * rx, uint8_t * tx) {
    *rx = C1EC & 0xff;
    *tx = (C1EC >> 8) & 0xff;
    return;
}

#endif

/**
 * irq receive
 * TODO: unfortunatly, suppose we have to change it, because of fixed size (8) in the loop  
 */
void __attribute__((__interrupt__, no_auto_psv)) _C1Interrupt(void) {
    // Clear the interrupt flags for receive and transmit buffer.
    C1INTFbits.RBIF = 0; // Reset the receive buffer interrupt flag.
    C1INTFbits.TBIF = 0; // Reset the transmit buffer interrupt flag.
    uint8_t k = 0;
    uint8_t n = 0;
    for (k = 0; k < 8; k++) {
        g_canRingBufferInfo.data[g_canRingBufferInfo.widx][n++] = (ecanRx2MsgBuf[0x0A][k]) & 0xff; // LSB
        g_canRingBufferInfo.data[g_canRingBufferInfo.widx][n++] = (ecanRx2MsgBuf[0x0A][k] >> 8) & 0xff;
    }

    g_canRingBufferInfo.widx = (g_canRingBufferInfo.widx + 1) & (CAN_BUFFER_CNT - 1); // wrapping around
    // Clear the full receive buffer flag to indicate the buffer is read.
    C1RXFUL1bits.RXFUL10 = 0;

    // Clear the general ECAN1 interrupt flag to complete the ISR.
    IFS2bits.C1IF = 0;
}

#endif
#ifdef TEST1

volatile canRingBufferInfo_t g_canRingBufferInfo;

void delay_mscan(unsigned int ms) 
{
    while (ms--) 
    {
        __delay32(20000000UL / 1000);
    }
}
#define NUM_OF_ECAN_BUFFERS 32
 /* This is the ECAN message buffer declaration. Note the buffer alignment. */
        unsigned int ecan1MsgBuf[NUM_OF_ECAN_BUFFERS][8] __attribute__((aligned(NUM_OF_ECAN_BUFFERS * 16)));

        
char canbuff[30];  

//void initCAN(void)
//{
//    
//    
//    /* Set up the ECAN1 module to operate at 250 kbps. The ECAN module should be first placed
//       in configuration mode. */
//    C1CTRL1bits.REQOP = 4;
//    
//    while(C1CTRL1bits.OPMODE != 4);
//    
//    C1CTRL1bits.WIN = 1;
//    
//    /* Set up the CAN module for 250kbps speed with 10 Tq per bit. */
//    C1CTRL1bits.CANCKS = 0x0;
//    uint16_t baud = ( (FCAN / (2 * NTQ * BITRATE)) - 1  );
//    C1CFG1bits.BRP = baud;    // BRP = 3 SJW = 1 Tq
//    C1CFG2 = 0x2D2;
//    C1FCTRL = 0xC01F; // No FIFO, 32 Buffers
//    C1CTRL1bits.WIN = 0;
//}

void initCAN(void) {
    g_canRingBufferInfo.ridx = 0x00;
    g_canRingBufferInfo.widx = 0x00;

    /* Set up the ECAN1 module to operate at 250 kbps. The ECAN module should be first placed
    in configuration mode. */
    C1CTRL1bits.REQOP = 4; // Request configuration mode.
    while (C1CTRL1bits.OPMODE != 4); // Wait until configuration mode is active.
    
    C1CTRL1bits.WIN = 1; // Enable window to access CAN configuration registers.
    /* Set up the CAN module for 250kbps speed with 10 Tq per bit. */
    C1CFG1bits.SJW = 0x0; // Synchronization Jump Width is set to 1xTQ
    /* Baud Rate Prescaler */
    C1CFG1bits.BRP = BRP_VAL; // Set Baud Rate Prescaler to achieve 250kbps.
    C1CFG2 = 0x2D2; // Configure Phase Segments and Propagation Time Segment
    C1FCTRL = 0xC01F; // No FIFO, 32 Buffers Set up FIFO, assign buffers as FIFO or as dedicated buffers.




//    if (0) // maybe used for tests
//    {
        C1FMSKSEL1bits.F0MSK = 0x0;
        C1RXM0SIDbits.SID = 0x07F8;
        C1RXF0SIDbits.SID = 0x07E8;

        C1RXM0SIDbits.MIDE = 0x1;
        C1RXF0SIDbits.EXIDE = 0x0;
        C1BUFPNT1bits.F0BP = 0xA;
        
//    } 
//    else if (1) 
//    {
//
        C1RXM1SID = 0xFFFF; // Sets the mask for standard and extended ID to accept all messages.
        C1RXM1EID = 0xFFFF; // Sets the extended ID mask to all 1s (accept all).
        /* C1RXF2SID = ((0x18DA << 2) | 0x03; */
        // sid << 5 | 0x08 | ((eid >> 16) & 0x03)
        /* C1RXF2SID = ((0x636) <<5) | (0x0A) ; */
        //Set 29-bit ID to 0x18DAF111    00 0110 0011 0110 10 1111 0001 0001 0001
        C1RXF2SID = ((0x636) << 5) | (0x08) | (0x02); // Sets the standard ID for the filter with bit manipulation.
        C1RXF2EID = 0xF111; // Sets the extended ID for the filter.
        C1BUFPNT1bits.F2BP = 0xA; // Assigns this filter to a specific buffer.
        C1FEN1 = 0; // Turns off all filters.
        C1FEN1bits.FLTEN2 = 0x1; // Specifically enables Filter 2.
        C1FEN1bits.FLTEN0 = 0x1;
        C1FMSKSEL1bits.F2MSK = 0x1; // Selects the mask type for Filter 2.
        C1CTRL1bits.WIN = 0x0; // Closes the filter configuration window.
//    }
//    uint32_t sid = 0x636;
//    uint32_t eid = 0x2F111;
    //printDEBUG(DSYS, "SID[%xw]R[%x]EID[%xw]\n", sid, C1RXF2SID, (eid >> 16) & 0x03);
    //printDEBUG(DSYS, "EID[%xw]R[%x]\n", (eid & 0xffff), C1RXF2EID);

    /* Configure Message Buffer 0 for Transmission and assign priority */
    C1TR01CONbits.TXEN0 = 0x1;
    C1TR01CONbits.TX0PRI = 0x3;


    /* Assign 32x8word Message Buffers for ECAN1 in device RAM. This example uses DMA0 for TX.
    Refer to 21.8.1 ?DMA Operation for Transmitting Data? for details on DMA channel
    configuration for ECAN transmit. */
    // CAN TX -Transimt
    DMA0CONbits.SIZE = 0x0;
    DMA0CONbits.DIR = 0x1;
    DMA0CONbits.AMODE = 0x2; // DMA Addressing Mode: Peripheral Indirect Addressing mode
    DMA0CONbits.MODE = 0x0;
    DMA0REQ = 70; // continuos mode / Ping-Pong modes disabled Set DMA0 Request Source to ECAN1 Transmit
    DMA0CNT = 7; // '7' DMA will perform 8 transfers
    DMA0PAD = (volatile unsigned int) &C1TXD; // ECAN1 Transmit Data Register (C1TXD)
    DMA0STAL = (unsigned int) &ecan1MsgBuf; // DMA Start Address Low Register
    DMA0STAH = (unsigned int) &ecan1MsgBuf; // DMA Start Address High Register
    DMA0CONbits.CHEN = 0x1; // enable this channel

    //CAN-Rx
    DMA1CONbits.SIZE = 0x0;
    DMA1CONbits.DIR = 0x0;
    DMA1CONbits.AMODE = 0x2; // Set Addressing Mode to Peripheral Indirect Addressing mode.
    DMA1CONbits.MODE = 0x0;
    DMA1REQ = 34; // Set DMA Request Source to the CAN1 receive event.
    DMA1CNT = 7; // Set the number of DMA transfers per DMA request to 8 (0 to 7)
    DMA1PAD = (volatile unsigned int) &C1RXD;
    //printDEBUG(DSYS, "Address [%x]", &ecanTx1MsgBuf);
    //printDEBUG(DSYS, "Address [%x]", &ecanRx2MsgBuf);
    DMA1STAL = (uint16_t) (&ecan1MsgBuf);
    DMA1STAH = (uint16_t) (&ecan1MsgBuf);
    //DMA1STAL = ((uint16_t) &ecan2MsgBuf) & 0xffff;
    //DMA1STAH = (((uint16_t) &ecan2MsgBuf) >> 16) & 0xffff;
    DMA1CONbits.CHEN = 0x1;


    //IEC2bits.C1IE = 1; // Enable CAN1 interrupts.  
    //C1INTE |= 0x02;
    /* At this point the ECAN1 module is ready to transmit a message. Place the ECAN module in
    Normal mode. */
    //    C1CTRL1bits.REQOP = 0; // Request Normal Operation mode.
    //    while (C1CTRL1bits.OPMODE != 0); // Wait for CAN module to switch to Normal Operation mode
    EXIT_CAN_CONFIG_MODUS;
}



uint32_t getBaudrateCAN(void)
{
    uint16_t baud = C1CFG1bits.BRP;
    uint32_t baudrate =(FCAN / (2 * NTQ * (baud + 1)));
    return baudrate;
}
void setBaudrateCAN(uint32_t baudrate)
{
    C1CTRL1bits.REQOP = 4;
    while(C1CTRL1bits.OPMODE != 4);
    uint16_t baud = (FCAN / (2 * NTQ * baudrate)) - 1;
    C1CFG1bits.BRP = baud;
    C1CTRL1bits.REQOP = 0;
    while(C1CTRL1bits.OPMODE != 0);   
}

/**
 * Configures the CAN filter and mask settings for the dsPIC33EP256GP504 microcontroller's CAN module. 
 * This function allows customization of the CAN filtering based on the mode and ID parameters provided. 
 * It includes configurations for both standard and extended data frames.
 *
 * @param mode The mode of the CAN filter, determining the type of data frames to filter (standard or extended).
 * @param sid Standard Identifier for the CAN message.
 * @param eid Extended Identifier for the CAN message.
 * @param sid_mask Mask for the Standard Identifier.
 * @param eid_mask Mask for the Extended Identifier.
 * 
 */
void configureFilterCAN(uint8_t mode, uint32_t sid, uint32_t eid, uint32_t sid_mask, uint32_t eid_mask) {

    //    C1CTRL1bits.REQOP = 4;
    //    while (C1CTRL1bits.OPMODE != 4); // Wait until the mode is active
    SWITCH_TO_CAN_CONFIG_MODUS;

    // Enable configuration window for the filters
    C1CTRL1bits.WIN = 0x1;

    // Decision based on the mode (Standard or Extended)
    switch (mode) {
            // For Standard Data Frames
        case(CAN_MODE_STANDARD_DATA_FRAME_VAR_DLC):
        case(CAN_MODE_STANDARD_DATA_FRAME_8_DLC):
        {
            // Select filter for Standard ID
            C1FMSKSEL1bits.F0MSK = 0x0;

            //printDEBUG(DWARNING, "Set normal format [%xw][%xw]\n", sid, sid_mask);
            // Configuration of mask and filter registers for Standard ID
            C1RXM0SIDbits.SID = sid_mask;
            C1RXF0SIDbits.SID = sid;

            // Configuration to accept only standard messages
            C1RXM0SIDbits.MIDE = 0x1; // Ensures only standard ID messages are accepted.
            C1RXF0SIDbits.EXIDE = 0x0; // Ignores extended IDs

            C1BUFPNT1bits.F0BP = 0xA; // Assigns this filter to a specific buffer

            C1FEN1bits.FLTEN0 = 0x1; // Enabling the filter

            // Closing the configuration window
            C1CTRL1bits.WIN = 0x0;
            break;
        }
            // For Extended Data Frames
        case(CAN_MODE_EXTENDED_DATA_FRAME_VAR_DLC):
        case(CAN_MODE_EXTENDED_DATA_FRAME_8_DLC):
        {
            C1RXF0SIDbits.EXIDE = 0x1; // accept extended IDs
            C1RXM0SIDbits.MIDE = 0x0; // dont accept standard ID messages 
            //TODO ERROR EXPECTED ABOVE
            // Extracting Standard ID and Extended ID from the given EID
            sid = (eid >> 18);
            eid = (eid & 0x3ffff);
            // Extracting masks for SID and EID
            sid_mask = (eid_mask >> 18) & 0x7ff;
            eid_mask = (eid_mask & 0x3ffff);

            //printDEBUG(DWARNING, "Set extended format [%xw][%xw]\n", eid, eid_mask);
            // Configuration of mask and filter registers for Extended ID
            C1RXM1SID = (sid_mask << 5) | ((eid_mask >> 16) & 0x03);
            C1RXM1EID = eid_mask & 0xffff;
            C1RXF2SID = ((sid) << 5) | (0x0A) | ((eid >> 16) & 0x03);
            C1RXF2EID = eid & 0xffff;

            // Assigning the filter to a buffer
            C1BUFPNT1bits.F2BP = 0xA;
            // Deactivating all filters except the selected one
            C1FEN1 = 0;
            // Enabling the filter
            C1FEN1bits.FLTEN2 = 0x1;
            // Selecting the filter for Extended ID
            C1FMSKSEL1bits.F2MSK = 0x1;

            // Closing the configuration window
            C1CTRL1bits.WIN = 0x0;
            break;
        }
    }
    // Return to normal mode
    //    C1CTRL1bits.REQOP = 0;
    //    while (C1CTRL1bits.OPMODE != 0); // Wait until the normal mode is active
    EXIT_CAN_CONFIG_MODUS;
}


////////////////
// Function to send a CAN message.
// Parameters:
// - sid: Standard Identifier for CAN message.
// - eid: Extended Identifier for CAN message.
// - mode: Type of CAN message, such as standard data frame or extended data frame.
// - data: Pointer to the data to be sent.
// - size: Size of the data in bytes.
// - timeout: Time to wait for the message to be sent before timing out.
uint8_t txMessageCAN(uint32_t sid, uint32_t eid, uint8_t mode, uint8_t data[], uint8_t size, uint16_t timeout) {
    
    //printDEBUG(0,"### txMessageCAN SID = %xw EID %xw \n", sid,eid);
    switch (mode) {
        case(CAN_MODE_STANDARD_DATA_FRAME_VAR_DLC):
        {
            ecan1MsgBuf[0][0] = (sid << 2) | (mode & 0x03);
            ecan1MsgBuf[0][1] = 0x0000;
            ecan1MsgBuf[0][2] = 0x0008;
            break;
        }
        case(CAN_MODE_EXTENDED_DATA_FRAME_VAR_DLC):
        {
            /* g_OBD_INFO.can_eid = 0x18DB33F1; */
//            uint32_t sid = (g_OBD_INFO.can_eid >> 16) & 0xfffC;
//            uint32_t eid = g_OBD_INFO.can_eid & 0x3fffF;
            //sid = (sid) & 0xffff;
            sid = (eid >> 16) & 0xfffC;
            eid = eid & 0x3ffff;            
            ecan1MsgBuf[0][0] = (sid) | 0x01;
            ecan1MsgBuf[0][1] =  eid >> 6;
            ecan1MsgBuf[0][2] = ((eid & 0x3f) << 10) | 0x08;
            break;
        }
        case(CAN_MODE_STANDARD_DATA_FRAME_8_DLC):
        {
            ecan1MsgBuf[0][0] = (sid << 2) | (mode & 0x03);
            ecan1MsgBuf[0][1] = 0x0000;
            ecan1MsgBuf[0][2] = 0x0008;
            break;
        }
        case(CAN_MODE_EXTENDED_DATA_FRAME_8_DLC):
        {
            /* g_OBD_INFO.can_eid = 0x18DB33F1; */
//            uint32_t sid = (g_OBD_INFO.can_eid >> 16) & 0xfffC;
//            uint32_t eid = g_OBD_INFO.can_eid & 0x3fffF;
            sid = (eid >> 16) & 0xfffC;
            eid = eid & 0x3ffff;            
            ecan1MsgBuf[0][0] = (sid) | 0x01;
            ecan1MsgBuf[0][1] = eid >> 6;
            ecan1MsgBuf[0][2] = ((eid & 0x3f) << 10) | 0x08;
            break;

        }


    }
    
    ecan1MsgBuf[0][3] = ((uint16_t)data[0])<<8  | (uint16_t)(size&0xFF);
    ecan1MsgBuf[0][4] = ((uint16_t)data[2])<<8  | (uint16_t)data[1];
    ecan1MsgBuf[0][5] = ((uint16_t)data[4])<<8  | (uint16_t)data[3];
    ecan1MsgBuf[0][6] = ((uint16_t)data[6])<<8  | (uint16_t)data[5];
    
        // Request the CAN module to send the message.
    C1TR01CONbits.TXREQ0 = 0x1; // Set the TXREQ0 bit to 1 to start transmission of buffer 0.
    //printDEBUG(DSYS, "Send CAN message [%xw][%xw].... ", sid, eid);
//    for (k = 0; k < 8; k++) {
//        //printDEBUG(DAPPEND, "%x ", ecanTx1MsgBuf[0][k]);
//
//    }
    //printDEBUG(DAPPEND, "\n");
    uint32_t timer = getSYSTIM();
    //while(C1TR01CONbits.TXREQ0 == 1) // While transmission request bit is set
    while ((C1TR01CONbits.TXREQ0 == 1) && (chk4TimeoutSYSTIM(timer, timeout) != SYSTIM_TIMEOUT)) // While transmission request bit is set  
    {
        if (chk4TimeoutSYSTIM(timer, timeout) == SYSTIM_TIMEOUT) 
        {
            C1TR01CONbits.TXREQ0 = 0x0; // Clear the transmission request bit.
            return 1; // Return 1 to indicate a timeout occurred.
        }
    }
    /* Message was placed successfully on the bus */
    //printDEBUG(DAPPEND, " DONE\n");
    return 0;
}
////////////////










void canTransmit(uint8_t data[], uint8_t len)
{
    /* Assign 32x8word Message Buffers for ECAN1 in device RAM. This example uses DMA0 for TX.
       Refer to 21.8.1 ?DMA Operation for Transmitting Data? for details on DMA channel
       configuration for ECAN transmit. */
//    DMA0CONbits.SIZE  = 0x0;
//    DMA0CONbits.DIR   = 0x1;
//    DMA0CONbits.AMODE = 0x2;
//    DMA0CONbits.MODE  = 0x0;
//    DMA0REQ = 70;
//    DMA0CNT = 7;
//    DMA0PAD = (volatile unsigned int)&C1TXD;
//    DMA0STAL = (unsigned int) &ecan1MsgBuf;
//    DMA0STAH = (unsigned int) &ecan1MsgBuf;
//    
//    DMA0CONbits.CHEN = 1;
    
 /* Configure Message Buffer 0 for Transmission and assign priority */
//    C1TR01CONbits.TXEN0 = 0x1;
//    C1TR01CONbits.TX0PRI = 0x3;
    /* At this point the ECAN1 module is ready to transmit a message. Place the ECAN module in
 Normal mode. */
//    C1CTRL1bits.REQOP = 0;
//    while(C1CTRL1bits.OPMODE != 0);
    /* Write to message buffer 0 */
    /* CiTRBnSID = 0bxxx1 0010 0011 1100
       IDE = 0b0
       SRR = 0b0
       SID<10:0>= 0b100 1000 1111 */
    ecan1MsgBuf[0][0] = (0x7DF << 2);
    /* CiTRBnEID = 0bxxxx 0000 0000 0000
       EID<17:6> = 0b0000 0000 0000 */
    ecan1MsgBuf[0][1] = 0x0000;
 /* CiTRBnDLC = 0b0000 0000 xxx0 1111
       EID<17:6> = 0b000000
       RTR = 0b0
       RB1 = 0b0
       RB0 = 0b0
       DLC = 0b1111 */
    ecan1MsgBuf[0][2] = 0x0008;
    
 /* Write message data bytes */
    ecan1MsgBuf[0][3] = ((uint16_t)data[0])<<8  | (uint16_t)(len&0xFF);
    ecan1MsgBuf[0][4] = ((uint16_t)data[2])<<8  | (uint16_t)data[1];
    ecan1MsgBuf[0][5] = ((uint16_t)data[4])<<8  | (uint16_t)data[3];
    ecan1MsgBuf[0][6] = ((uint16_t)data[6])<<8  | (uint16_t)data[5];
    
//    ecan1MsgBuf[0][3] = 0x0102;
//    ecan1MsgBuf[0][4] = 0x000C;
//    ecan1MsgBuf[0][5] = 0x0000;
//    ecan1MsgBuf[0][6] = 0x0000;
    /* Message was placed successfully on the bus */
    
    /* Request message buffer 0 transmission */
    C1TR01CONbits.TXREQ0 = 1;
    /* The following shows an example of how the TXREQ bit can be polled to check if transmission
    is complete. */
    while(C1TR01CONbits.TXREQ0 == 1);
}

bool canReceive(uint8_t mode, uint8_t *data, uint8_t *datalen, uint16_t timeout)
{
 
//    C1CTRL1bits.REQOP = 4;
//    while(C1CTRL1bits.OPMODE != 4);
//    C1CTRL1bits.WIN = 1;
    
    /* Assign 32x8word Message Buffers for ECAN1 in device RAM. This example uses DMA1 for RX.
       Refer to 21.8.1 ?DMA Operation for Transmitting Data? for details on DMA channel
       configuration for ECAN transmit. */
//    DMA1CONbits.SIZE  = 0x0;
//    DMA1CONbits.DIR   = 0x0;
//    DMA1CONbits.AMODE = 0x2;
//    DMA1CONbits.MODE  = 0x0;
//    DMA1REQ  = 34;
//    DMA1CNT  = 7;
//    DMA1PAD  = (volatile unsigned int)&C1RXD;
//    DMA1STAL = (unsigned int) &ecan1MsgBuf;
//    DMA1STAH = (unsigned int) &ecan1MsgBuf;
//    DMA1CONbits.CHEN = 0x1;
    /* Select Acceptance Filter Mask 0 for Acceptance Filter 0 */
//      C1FMSKSEL1bits.F0MSK=0x0; 
    
    /* Configure Acceptance Filter Mask 0 register to mask SID<2:0>
     * Mask Bits (11-bits) : 0b111 1111 1000 */
    
//    C1RXM0SIDbits.SID = 0x7E8;
    
    /* Configure Acceptance Filter 0 to match standard identifier 
       Filter Bits (11-bits): 0b011 1010 xxx with the mask setting, message with SID
       range 0x1D0-0x1D7 will be accepted by the ECAN module. */
//    C1RXF0SIDbits.SID = 0x07E8;
//    
//    /* Acceptance Filter 0 to check for Standard Identifier */
//    C1RXM0SIDbits.MIDE = 0x1;
//    C1RXF0SIDbits.EXIDE= 0x0;
//    
//    /* Acceptance Filter 0 to use Message Buffer 10 to store message */
//    
//    C1BUFPNT1bits.F0BP = 0xA;
//    /* Filter 0 enabled for Identifier match with incoming message */
//    C1FEN1bits.FLTEN0=0x1;
//    
//    /* Clear Window Bit to Access ECAN 
//     * Control Registers */
//    
//    C1CTRL1bits.WIN=0x0;
//    /* Place the ECAN module in normal
//     * mode. */
//    C1CTRL1bits.REQOP = 0;
//    while(C1CTRL1bits.OPMODE != 0);
    /* The following code shows one example of how the application can wait
 for a message to be received in message buffer 10 */
//    while(1)
//    {
//        /* Message was received. */
    uint32_t timer = getSYSTIM();
    //while(C1TR01CONbits.TXREQ0 == 1) // While transmission request bit is set
    while ((C1RXFUL1bits.RXFUL10 == 0) && (chk4TimeoutSYSTIM(timer, timeout) != SYSTIM_TIMEOUT)) // While transmission request bit is set  
    {
        if (chk4TimeoutSYSTIM(timer, timeout) == SYSTIM_TIMEOUT) {
            C1RXFUL1bits.RXFUL10 = 0; // Clear the transmission request bit.
            UART1_Write_String("\n\rNOT CONNECTED");
            return 1; // Return 1 to indicate a timeout occurred.
        }
    }
    C1RXFUL1bits.RXFUL10 = 0;
    char canbuff[40];
    switch (mode) 
    {
        case(CAN_MODE_EXTENDED_DATA_FRAME_8_DLC):
        {
            
            // 1 1111 1111 1100 0000 0000 0000 0000 
            // 00          0011 1111 1111 1100 0000
            //                            0011 1111
            //0x18DAF111
            //6360BC400118
            //0001 1000 1101 10 11 0000 0000 00 00 0000
            //0000 0000 0000 00 10 1111 0001 00 00 0000
            //0000 0000 0000 00 00 0000 0000 00 01 0001
            //18DAF111
//            uint32_t a,b,c;
//            a = (uint32_t)ecan1MsgBuf[10][0]<<16 & 0x1FFC0000;
//            b = (uint32_t)ecan1MsgBuf[10][1]<<6 & 0x3FFC0;
//            c = ecan1MsgBuf[10][2]>>10 & 0x3f;
            if (g_obdInfo.formatProperties.ath == 1)
            {
                sprintf(canbuff,"\r%08lX",(uint32_t)ecan1MsgBuf[10][0]<<16 & 0x1FFC0000 |
                                        (uint32_t)ecan1MsgBuf[10][1]<<6  & 0x3FFC0 |
                                        (uint32_t)ecan1MsgBuf[10][2]>>10 & 0x3f);
                UART1_Write_String(canbuff);
                if(g_obdInfo.formatProperties.ats == 1)
                {
                    UART1_Write(' ');
                }
                if (g_obdInfo.formatProperties.atd == 1)
                {
                    data[0] =  ecan1MsgBuf[10][3] & 0xFF;
                }
            }
            
            break;
        }
        case (CAN_MODE_STANDARD_DATA_FRAME_8_DLC):
        {
            if (g_obdInfo.formatProperties.ath == 1)
            {
                sprintf(canbuff,"\r%03X",ecan1MsgBuf[10][0]>>2);
                UART1_Write_String(canbuff);
                if(g_obdInfo.formatProperties.ats == 1)
                {
                    UART1_Write(' ');
                }
                if (g_obdInfo.formatProperties.atd == 1)
                {
                    data[0] =  ecan1MsgBuf[10][3] & 0xFF;
                }
            }
            break;
        }
    }
    for (int i = 0;i < 4; i++)
    {
        if(i!=0)
        {
            data[(i*2) + 0] =  ecan1MsgBuf[10][i+3] & 0xFF;
        }
        data[(i*2) + 1] = (ecan1MsgBuf[10][i+3]>>8) & 0xFF;
    }
    *datalen =  ecan1MsgBuf[10][3] & 0xFF;
    return true;
    
//    sprintf(canbuff,"\r\n%02X %02X %02X %02X %02X %02X %02X %02X",ecan1MsgBuf[10][3]&0xFF,(ecan1MsgBuf[10][3]>>8)&0xFF
//                                                 ,ecan1MsgBuf[10][4]&0xFF,(ecan1MsgBuf[10][4]>>8)&0xFF
//                                                 ,ecan1MsgBuf[10][5]&0xFF,(ecan1MsgBuf[10][5]>>8)&0xFF
//                                                 ,ecan1MsgBuf[10][6]&0xFF,(ecan1MsgBuf[10][6]>>8)&0xFF);
//    UART1_Write_String(canbuff);
    
    
//    }
}

void iso_tp_send(uint32_t sid, uint32_t eid, uint8_t mode, uint8_t data[], uint16_t size, uint16_t timeout) //uint32_t sid, uint32_t eid, uint8_t mode, uint8_t data[], uint8_t size, uint16_t timeout
{
    if (size <= 7) 
    {
        // Single Frame
        uint8_t frame[CAN_FRAME_DATA_LEN] = {0};
        frame[0] = (ISO_TP_FRAME_TYPE_SF << 4) | (size & 0x0F);
        memcpy(&frame[1], data, size);
        txMessageCAN(sid, eid, mode, frame, size, timeout);
    } 
    else 
    {
        // First Frame
        uint8_t frame[CAN_FRAME_DATA_LEN] = {0};
        uint8_t frameLen = 0;
        frame[0] = (ISO_TP_FRAME_TYPE_FF << 4) | ((size >> 8) & 0x0F);
        frame[1] = size & 0xFF;
        memcpy(&frame[2], data, 6);
        txMessageCAN(sid, eid, mode, frame, CAN_FRAME_DATA_LEN, timeout);

        // Wait for FC
        uint8_t fc_frame[8];
        while (!canReceive(mode, fc_frame,&frameLen,timeout)) 
            //canReceive(uint8_t mode, uint8_t *data, uint8_t *datalen)
        {
            // add timeout or retry logic
        }

        if ((fc_frame[0] >> 4) != ISO_TP_FRAME_TYPE_FC) return;

        // Send Consecutive Frames
        uint16_t offset = 6;
        uint8_t sn = 1;

        while (offset < size) 
        {
            uint8_t frame_cf[CAN_FRAME_DATA_LEN] = {0};
            frame_cf[0] = (ISO_TP_FRAME_TYPE_CF << 4) | (sn++ & 0x0F);
            uint8_t chunk = (size - offset > 7) ? 7 : (size - offset);
            memcpy(&frame_cf[1], data + offset, chunk);
            txMessageCAN(sid, eid, mode, frame_cf + offset, chunk, timeout);
            //txMessageCAN(sid, eid, mode, frame, size, timeout);
            offset += chunk;
            __delay32(1);  // small delay to avoid bus congestion
        }
    }
}


bool iso_tp_receive(uint8_t mode, uint8_t* data, uint8_t* size, uint16_t timeout ,uint32_t sid, uint32_t eid) {
    uint8_t frame[CAN_FRAME_DATA_LEN];
    uint8_t frameLen = 0;
    if (!canReceive(mode, frame, &frameLen, timeout)) return false;
//    for (int i = 0;i <= frameLen; i++)
//    {
//        sprintf(canbuff,"%02X",frame[i]);
//        UART1_Write_String(canbuff);
//    }
//    UART1_Write_String("\r\n");
    uint8_t frame_type = frame[0] >> 4;

    if (frame_type == ISO_TP_FRAME_TYPE_SF) 
    {
        uint8_t len = frame[0] & 0x0F;
        *size = frame[0] & 0x0F;
//        //sprintf(canbuff,"len=%02X ptr=%p ",len,size);
//        sprintf(canbuff,"len=%02X ptr=%p val=%02X", len, size, *size);
//        UART1_Write_String(canbuff);
        
        //memcpy(data, &frame[1], len);
        for (uint8_t i = 0; i < len; i++) 
        {
           data[i] = frame[i + 1];
        }
        
        UART1_Write_String("OKSF");
        return true;
        

    } 
    else if (frame_type == ISO_TP_FRAME_TYPE_FF) 
    {
        UART1_Write_String("OKFF");
        uint16_t total_len = ((frame[0] & 0x0F) << 8) | frame[1];
        sprintf(canbuff,"total_len=%04X ", total_len);
        UART1_Write_String(canbuff);
        memcpy(data, &frame[2], 6);
        for (int i = 0;i < 6; i++)
        {
            sprintf(canbuff,"%02X",data[i]);
            UART1_Write_String(canbuff);
        }
        uint16_t received = 6;

        // Send FC
        uint8_t fc_frame[CAN_FRAME_DATA_LEN] = {0};
        fc_frame[0] = (ISO_TP_FRAME_TYPE_FC << 4) | ISO_TP_FLOW_STATUS_CTS;
        fc_frame[1] = 0x00;  // Block Size = 0 (unlimited)
        fc_frame[2] = 0x00;  // STmin
        //can_send_frame(can_id, fc_frame, CAN_FRAME_DATA_LEN);
        for (int i = 0;i < 8; i++)
        {
            sprintf(canbuff,"%02X",fc_frame[i]);
            UART1_Write_String(canbuff);
        }
        txMessageCAN(sid, eid, mode, fc_frame, CAN_FRAME_DATA_LEN, timeout);
        UART1_Write_String("okFCFRAME");
        // Receive Consecutive Frames
        uint8_t sn_expected = 1;

        while (received < total_len) {
            
            while (!canReceive(mode, frame, &frameLen,timeout)) {
                // add timeout or retry
            }
            
            if ((frame[0] >> 4) != ISO_TP_FRAME_TYPE_CF || (frame[0] & 0x0F) != sn_expected) {
                return false;  // unexpected SN
            }

            sn_expected = (sn_expected + 1) & 0x0F;
            uint8_t chunk = (total_len - received > 7) ? 7 : (total_len - received);
            memcpy(data + received, &frame[1], chunk);
            received += chunk;
        }

        *size = total_len;
         return true;
    }

    return false;
}


void setCANTimConReg(uint32_t timeValue)
{
    uint8_t  timConReg1 = (timeValue >> 16) & 0xff;
    uint16_t timConReg2 = (timeValue & 0xffff);
    C1CTRL1bits.REQOP = 4;
    while(C1CTRL1bits.OPMODE != 4);
    C1CFG1 = timConReg1;
    C1CFG2 = timConReg2;
    C1CTRL1bits.REQOP = 0;
    while(C1CTRL1bits.OPMODE != 0);
}
uint32_t getCANTimConReg(void)
{
    uint32_t timConValue = (uint32_t)(C1CFG1 & 0xff)<<16 | C1CFG2;
    return timConValue;
}
void getErrorCountCAN(uint8_t * rx, uint8_t * tx) {
    *rx = C1EC & 0xff;
    *tx = (C1EC >> 8) & 0xff;
    return;
}
#endif

