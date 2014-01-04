// ----------------------------------------------------------------------------
// PTX_Setup_Test
// version: 1.0.0
// date: 2013-01-04
// author: Conor O'Brien
// ----------------------------------------------------------------------------

#include <RF_Nordic.h>
#include <SPI.h>

byte writeDat[5];
byte readDat[5];
byte msg[1] = {0x55};
byte status[1];
byte tx_address[] = {0x5a,0x5a,0x5a,0x5a,0x5a};

void setup()
{
	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV64);

	Serial.begin(9600);
	while(!Serial.available());
	Serial.read();
	byte rx_address[5] = {0x5f,0x5a,0x5a,0x5f,0x5f};
	RFsetup(1,rx_address);
	pinMode(9,OUTPUT);
	digitalWrite(9, LOW);
  	pinMode(SS_PIN,OUTPUT);
  	byte reg[1] = {0x01};

  	RFwriteReg(RX_PW_P1, reg, 1);
	RFwriteReg(RX_PW_P0, reg, 1);

}

void loop()
{

	while(!Serial.available());
	msg[0] = Serial.read();

	RFtx(msg, tx_address);
	delay(500);
	RFreadReg(STATUS, readDat, 1);
	Serial.print("STATUS1: ");
	Serial.println(readDat[0],BIN);
	writeDat[0]= B00010000;
	RFwriteReg(STATUS,writeDat,1);
	RFreadReg(STATUS, readDat, 1);
	Serial.print("STATUS2: ");
	Serial.println(readDat[0],BIN);
}