#ifndef RF_Nordic_h
#define RF_Nordic_h

#include "Arduino.h"


byte RF_writeReg(byte reg, byte* data_out, char num_bytes);
byte RF_readReg(byte reg, byte* data_in, char num_bytes);
void RF_setup(byte msg_length,byte* rx_address);
byte RF_status(void);
byte RF_tx(byte* data_out);
byte RF_txAddress(byte* tx_address);
byte RF_rx(byte* data_in);
byte RF_rxAvailable(void);
void RF_flushRX(void);
void RF_flushTX(void);
void RF_resetInterrupt(char interrupt); 
void RF_printReg(void);

#define SS_PIN 10
#define CE_PIN 9
#define IRQ_PIN 14

// Command mnemonic for nRF24L01+
#define R_REGISTER    	(0x00)
#define W_REGISTER   	(0x20)
#define R_RX_PAYLOAD  	(0x61)
#define W_TX_PAYLOAD  	(0xa0)
#define FLUSH_TX    	(0xe1)
#define FLUSH_RX    	(0xe2)
#define REUSE_TX_PL 	(0xe3)
#define ACTIVATE		(0x50)
#define R_RX_PL_WID		(0x60)
#define W_ACK_PAYLOAD	(0xa8)
#define W_TX_PAYLOAD_NOACK	(0xb0)
#define NOP         	(0xff)


// Register and Bit Mnemonic
#define CONFIG      (0x00)
#define MASK_RX_DR	6
#define MASK_TX_DS	5
#define MASK_MAX_RT	4
#define EN_CRC		3
#define CRCO		2
#define PWR_UP		1
#define PRIM_RX		0

#define EN_AA       (0x01)
#define EN_AA_P5	5
#define EN_AA_P4	4
#define EN_AA_P3	3
#define EN_AA_P2	2
#define EN_AA_P1	1
#define EN_AA_P0	0


#define EN_RXADDR   (0x02)
#define ERX_P5		5
#define ERX_P4		4
#define ERX_P3		3
#define ERX_P2		2
#define ERX_P1		1
#define ERX_P0		0

#define SETUP_AW    (0x03)
#define AW 			0		//BITS 1:0

#define SETUP_RETR  (0x04)
#define ARD 		4		// BITS 7:4
#define ARC 		0 		// BITS 3:0

#define RF_CH       (0x05)

#define RF_SETUP    (0x06)
#define PLL_LOCK	4
#define RF_DR		3
#define RF_PWR		1		//BITS 2:1
#define LNA_HCURR	0

#define STATUS      (0x07)
#define RX_DR 		6
#define TX_DS 		5
#define MAX_RT 		4
#define RX_P_NO		1		//BITS 3:1
#define TX_FULL		0

#define OBSERVE_TX  (0x08)
#define PLOS_CNT 	4 		//BITS 7:4
#define ARC_CNT 	0 		//BITS 3:0

#define RPD          (0x09)

//Pipe Addresses and control registers (see datasheet)
#define RX_ADDR_P0  (0x0a)
#define RX_ADDR_P1  (0x0b)
#define RX_ADDR_P2  (0x0c)
#define RX_ADDR_P3  (0x0d)
#define RX_ADDR_P4  (0x0e)
#define RX_ADDR_P5  (0x0f)
#define TX_ADDR     (0x10)
#define RX_PW_P0    (0x11)
#define RX_PW_P1    (0x12)
#define RX_PW_P2    (0x13)
#define RX_PW_P3    (0x14)
#define RX_PW_P4    (0x15)
#define RX_PW_P5    (0x16)

#define FIFO_STATUS (0x17)
#define TX_REUSE	6
#define TX_FULL		5
#define TX_EMPTY	4
#define RX_FULL		1
#define RX_EMPTY	0

#define DYNPD		(0x1c)
#define DPL_P5		5
#define DPL_P4		4
#define DPL_P3		3
#define DPL_P2		2
#define DPL_P1		1
#define DPL_P0		0

#define FEATURE		(0x1d)
#define EN_DPL		2
#define EN_ACK_PAY	1
#define EN_DYN_ACK	0

#endif
