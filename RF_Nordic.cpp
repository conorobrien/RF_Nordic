#include "Arduino.h"
#include "RF_Nordic.h"
#include <SPI.h>

static byte _msg_length;
volatile byte _status;
static byte _write[5];
static byte _read[5];

byte RF_writeReg(byte reg, byte* data_out, char num_bytes)  
{
  //Bring SS low, write command byte through SPI
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(W_REGISTER+reg);
  //Send data bytes, LSByte to MSByte, for consistency use arrays for everything
  for (int i=(num_bytes-1); i>=0; i--)
  {
    SPI.transfer(data_out[i]);
  }
  //Bring SS high, terminate SPI, delay for 1 us to ensure radio can differentiate between bytes
  digitalWrite(SS_PIN,HIGH);
  delayMicroseconds(1);
}

byte RF_readReg(byte reg, byte *data_in, char num_bytes)
{

  digitalWrite(SS_PIN, LOW);
  SPI.transfer(R_REGISTER+reg);

  for (int i=(num_bytes -1); i>= 0; i--)
  {
    data_in[i] = SPI.transfer(NOP);
  }

  digitalWrite(SS_PIN,HIGH);
  delayMicroseconds(1);
}

void RF_setup(byte msg_length, byte* rx_address)
{
  //This is a simple setup, just initializes a single mode with auto-ack enabled
  //Puts it into PRX mode
    //Set pins (apparently have to declare imputs on Teensy3.1)
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH);
  pinMode(CE_PIN, OUTPUT);
  digitalWrite(CE_PIN, HIGH);
  pinMode(IRQ_PIN, INPUT);
  //Store msg_length in private variable
  _msg_length = msg_length;

  //Start SPI
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV32);

  // 200 ms to allow RF clock to stabilize
  delay(200); 

  //Write PWR_UP, EN_CRC and PRIM_RX to CONFIG
  _write[0] = (1<<PWR_UP)|(1<<EN_CRC)|(1<<PRIM_RX);
  RF_writeReg(CONFIG, _write, 1);

  //EN_RXADDR default values (defaults, just to make sure for now)
  _write[0] = (1<<ERX_P0)|(1<<ERX_P1);
  RF_writeReg(EN_RXADDR, _write, 1);

  //Address width (5 for now)
  _write[0] = 0x03;
  RF_writeReg(SETUP_AW, _write, 1);

  //SETUP_RETR for auto-acks, wait 500 us, 10 retries
  _write[0] = 0x1a;
  RF_writeReg(SETUP_RETR, _write, 1);

  //RF_CH, sets frequency channel
  _write[0] = 0x02;
  RF_writeReg(RF_CH, _write, 1);

  //Write rx_address to pipe 1
  RF_writeReg(RX_ADDR_P1, rx_address, 5);

  //Set pipe 0 payload width to 1 byte (for auto-acks)
  _write[0] = 0x01;
  RF_writeReg(RX_PW_P0, _write, 1);

  //Set pipe 1 payload width to message_length
  _write[0] = msg_length;
  RF_writeReg(RX_PW_P1, _write, 1);

  //Flush buffers
  RF_flushTX();
  RF_flushRX();


}

byte RF_status(void)
{
  digitalWrite(SS_PIN, LOW);
  _status = SPI.transfer(NOP);
  digitalWrite(SS_PIN,HIGH);
  delayMicroseconds(1);
  return _status;
}

byte RF_tx(byte* tx_message)
{

  //Set CE to low before going into TX mode
  digitalWrite(CE_PIN, LOW);

  //Set PRIM_RX to 0;
  _write[0] = (1<<PWR_UP)|(1<<EN_CRC);
  RF_writeReg(CONFIG, _write, 1);

  //write message to TX_PLD FIFO
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(W_TX_PAYLOAD);
  for (int i=(_msg_length-1); i>=0; i--)
  {
    SPI.transfer(tx_message[i]);
  }
  digitalWrite(SS_PIN, HIGH);
  delayMicroseconds(1);
  
  //Pulse CE_PIN high for 15 us
  digitalWrite(CE_PIN, HIGH);
  delayMicroseconds(20);
  digitalWrite(CE_PIN, LOW);

  //block until IRQ trigger, then check status for interrupts
  while(digitalRead(IRQ_PIN));
  RF_status();

  //Turn radio back into a reciever
  _write[0] = (1<<PWR_UP)|(1<<EN_CRC)|(1<<PRIM_RX);
  RF_writeReg(CONFIG, _write, 1);
  digitalWrite(CE_PIN, HIGH);

  //Return result (0 = failure, 1 = success, 2 = debug/something wrong) and reset interrupt
  if (_status & (1<<TX_DS))
  {
    RF_resetInterrupt(TX_DS);
    return 1;
  }
  else if (_status & (1<<MAX_RT))
  {
    RF_resetInterrupt(MAX_RT);

    //Clear TX buffer
    RF_flushTX();

    return 0;
  }
  else
  {
    return 2;
  }
}

byte RF_txAddress(byte* tx_address)
{
  //Write tx_address into the TX_ADDR and the pipe0 rx address for auto-acks
  RF_writeReg(TX_ADDR, tx_address, 5);
  RF_writeReg(RX_ADDR_P0, tx_address, 5);
}

byte RF_rx(byte* rx_message)
{
    digitalWrite(SS_PIN, LOW);
    SPI.transfer(R_RX_PAYLOAD);
    for (int i=(_msg_length-1); i>=0; i--)
    {
      rx_message[i] = SPI.transfer(NOP);
    } 
    digitalWrite(SS_PIN, HIGH);
    RF_resetInterrupt(RX_DR);
}

byte RF_rxAvailable()
{
  RF_status();
  byte fifo_status[1];
  RF_readReg(FIFO_STATUS,fifo_status,1);

  if ((_status & (1<<RX_DR)) || (~fifo_status[0] & (1<<RX_EMPTY)))
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void RF_flushRX(void)
{
    digitalWrite(SS_PIN, LOW);
    SPI.transfer(FLUSH_RX);
    digitalWrite(SS_PIN, HIGH);
    delayMicroseconds(1);
}

void RF_flushTX(void)
{
    digitalWrite(SS_PIN, LOW);
    SPI.transfer(FLUSH_TX);
    digitalWrite(SS_PIN, HIGH);
    delayMicroseconds(1);
}

void RF_resetInterrupt(char interrupt)
{
  byte writeDat[1] = {(1<<interrupt)};
  RF_writeReg(STATUS, writeDat, 1);
}

void RF_printReg(void)
{
  byte values[5];
  Serial.print("CONFIG:     ");
  RF_readReg(CONFIG,values,1);
  Serial.println(values[0],BIN);

  Serial.print("EN_AA:      ");
  RF_readReg(EN_AA,values,1);
  Serial.println(values[0],BIN);

  Serial.print("EN_RXADDR:  ");
  RF_readReg(EN_RXADDR,values,1);
  Serial.println(values[0],BIN);

  Serial.print("SETUP_AW:   ");
  RF_readReg(SETUP_AW,values,1);
  Serial.println(values[0],BIN);

  Serial.print("SETUP_RETR: ");
  RF_readReg(SETUP_RETR,values,1);
  Serial.println(values[0],BIN);

  Serial.print("RF_CH:      ");
  RF_readReg(RF_CH,values,1);
  Serial.println(values[0],BIN);

  Serial.print("RF_SETUP:   ");
  RF_readReg(RF_SETUP,values,1);
  Serial.println(values[0],BIN);

  Serial.print("STATUS:     ");
  RF_readReg(STATUS,values,1);
  Serial.println(values[0],BIN);

  Serial.print("OBSERVE_TX: ");
  RF_readReg(OBSERVE_TX,values,1);
  Serial.println(values[0],BIN);

  Serial.print("RPD:        ");
  RF_readReg(RPD,values,1);
  Serial.println(values[0],BIN);

  Serial.print("RX_ADDR_P0: ");
  RF_readReg(RX_ADDR_P0,values,5);
  Serial.print(values[0],HEX);
  Serial.print(values[1],HEX);
  Serial.print(values[2],HEX);
  Serial.print(values[3],HEX);
  Serial.println(values[4],HEX);

  Serial.print("RX_ADDR_P1: ");
  RF_readReg(RX_ADDR_P1,values,5);
  Serial.print(values[0],HEX);
  Serial.print(values[1],HEX);
  Serial.print(values[2],HEX);
  Serial.print(values[3],HEX);
  Serial.println(values[4],HEX);

  Serial.print("TX_ADDR:    ");
  RF_readReg(TX_ADDR,values,5);
  Serial.print(values[0],HEX);
  Serial.print(values[1],HEX);
  Serial.print(values[2],HEX);
  Serial.print(values[3],HEX);
  Serial.println(values[4],HEX);

  Serial.print("FIFO_STATUS: ");
  RF_readReg(FIFO_STATUS,values,1);
  Serial.println(values[0],BIN);
  
  Serial.print("RX_PW_P0:   ");
  RF_readReg(RX_PW_P0,values,1);
  Serial.println(values[0],BIN);

  Serial.print("RX_PW_P1:   ");
  RF_readReg(RX_PW_P1,values,1);
  Serial.println(values[0],BIN);

  Serial.print("DYNPD:      ");
  RF_readReg(DYNPD,values,1);
  Serial.println(values[0],BIN);

  Serial.print("FEATURE:    ");
  RF_readReg(FEATURE,values, 1);
  Serial.println(values[0],BIN);
}



















































