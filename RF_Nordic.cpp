#include "Arduino.h"
#include "RF_Nordic.h"
#include <SPI.h>

static byte _status_byte;
static byte _msg_length;

void RFsetup(char msg_length, byte* rx_address, char en_aa
{
  //This is a simple setup, just initializes 
  delay(100); // allow RF clock to stabilize



  _msg_length = msg_length;
  
  //Turn on SPI, set data rate to 1.5 Mb/s (96MHz/64)
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV64);

  //Load default values from header file
  byte writeDat[1] = {DEFAULT_CONFIG};
  RFwriteReg(CONFIG, writeDat, 1);
  writeDat[0] = 0x1a;
  RFwriteReg(SETUP_RETR, writeDat,1);
  writeDat[0] = 0x03;
  RFwriteReg(EN_AA, writeDat, 1);

  // writeDat[0] = DEFAULT_RETR;
  // RFwriteReg(SETUP_RETR, writeDat, 1);

  //Setup pipe 1 to listen on rx_address, reserve pipe 0 for auto-acks
  RFpipeSetup(0x01, rx_address);
}

byte RFwriteReg(byte reg, byte* data_out, char num_bytes)  
{
  //Bring SS low, initialte SPI transfer
  digitalWrite(SS_PIN, LOW);
  _status_byte = SPI.transfer(W_REGISTER+reg);

  //Step through array containing message, for consistency use arrays for everything
  for (int i=0; i<= (num_bytes - 1); i++)
  {
    SPI.transfer(data_out[i]);
  }

  //Bring SS high, terminate SPI, delay for 1 us to ensure radio can differentiate between bytes
  digitalWrite(SS_PIN,HIGH);
  delayMicroseconds(1);

  //return the status byte if you want it
  return _status_byte;

  //Delay 1 us to allow spi to differenctiate between bytes
  delayMicroseconds(1);
}

byte RFreadReg(byte reg, byte *data_in, char num_bytes)
//Same as above, but reads the register
{

  digitalWrite(SS_PIN, LOW);
  _status_byte = SPI.transfer(R_REGISTER+reg);

  for (int i=0; i<= (num_bytes - 1); i++)
  {
    data_in[i] = SPI.transfer(NOP);
  }

  digitalWrite(SS_PIN,HIGH);
  delayMicroseconds(1);


  return _status_byte;
}

void RFpipeSetup(byte pipe, byte* pipe_address)
{

  //Read what pipes are currently active, turn bit on, then write back to register
  byte readDat[1];
  RFreadReg(EN_RXADDR, readDat,1);
  byte writeDat[1] = {readDat[0]|(1<<pipe)};
  RFwriteReg(EN_RXADDR,writeDat,1);

  //writes address to appropriate register, currently always a 5-byte address
  byte pipe_register = RX_ADDR_P0 + pipe;
  RFwriteReg(pipe_register,pipe_address,5);
}

void RFpipeShutdown(byte pipe)
{
  //read pipe register, turn off bit we want, write result
  byte readDat[1];
  RFreadReg(EN_RXADDR, readDat,1);
  byte writeDat[1] = {readDat[0]&~(1<<pipe)};
  RFwriteReg(EN_RXADDR,writeDat,1);
}

byte RFtx(byte* data_out, byte*tx_address)
{
  RFpipeSetup(0, tx_address);
  //Load message into FIFO and set addresses for TX and auto-ack
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(W_TX_PAYLOAD);
  for (int i=0; i<= (_msg_length - 1); i++)
  {
    SPI.transfer(data_out[i]);
  }
  digitalWrite(SS_PIN,HIGH);
  delayMicroseconds(1);

  RFwriteReg(TX_ADDR, tx_address, 5);
  RFwriteReg(RX_ADDR_P0, tx_address, 5);
  
  digitalWrite(9, HIGH);
  delayMicroseconds(20);
  digitalWrite(9, LOW);

  //Currently doesn't wait for auto-acks, when this function definitely works then create a blocking function, or at least check the tx FIFO for that pipe
}
