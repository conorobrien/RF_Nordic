// ----------------------------------------------------------------------------
// PRX_Setup_Test.ino
// version: 1.0.0
// date: 2014-01-04
// author: Conor O'Brien
// ----------------------------------------------------------------------------

#include <RF_Nordic.h>
#include <SPI.h>

byte writeDat[1];
byte readDat[1];
byte msg[1];
byte rx_address[5] = {0x5a,0x5a,0x5a,0x5a,0x5a};
byte status[1];
byte status_reset[1];
int i = 0;

void setup()
{
	Serial.begin(9600);
	while(!Serial.available());
	Serial.read();
	RFsetup(1,rx_address);
	byte reg[1] = {DEFAULT_CONFIG};
	reg[0] |= (1<<PRIM_RX);
	RFwriteReg(CONFIG,reg,1);
	reg[0] = 0x01;
	RFwriteReg(RX_PW_P1, reg, 1);
	RFwriteReg(RX_PW_P0, reg, 1);
	pinMode(9, OUTPUT);
	digitalWrite(9, HIGH);
	RFreadReg(CONFIG,msg,1);
	Serial.println(msg[0], BIN);
	RFreadReg(EN_AA,msg,1);
	Serial.println(msg[0], BIN);
	RFreadReg(EN_RXADDR,msg,1);
	Serial.println(msg[0], BIN);
	RFreadReg(RX_PW_P1,msg,1);
	Serial.println(msg[0], BIN);

}

void loop()
{
	while(!Serial.available());
	Serial.read();
	RFreadReg(STATUS, status, 1);
	Serial.println(status[0], BIN);
	Serial.print(i);
	Serial.println(" - ");
	i++;

	if (status[0] & (1<<RX_DR))
	{
		digitalWrite(SS_PIN, LOW);
  		SPI.transfer(R_RX_PAYLOAD);
    	msg[0] = SPI.transfer(NOP);
  		digitalWrite(SS_PIN,HIGH);
  		delayMicroseconds(1);

  		status_reset[0] = {status[0]};
  		RFwriteReg(STATUS,status_reset,1);
		Serial.print((char) msg[0]);
		Serial.print(" : ");
		Serial.println("recieved");

		digitalWrite(SS_PIN, LOW);
  		SPI.transfer(FLUSH_RX);
  		digitalWrite(SS_PIN,HIGH);
  		delayMicroseconds(1);
	}
	delay(500);


}