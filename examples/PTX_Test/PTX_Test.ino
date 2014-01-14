// ----------------------------------------------------------------------------
// PTX_Test.ino
// version: 1.2.0
// date: 2014-01-04
// author: Conor O'Brien
// ----------------------------------------------------------------------------
#include <SPI.h>
#include <RF_Nordic.h>

#define MSG_LENGTH 5

byte rx_address[5] = {0x51,0x51,0x51,0x51,0x51};
byte tx_address[5] = {0x5a,0x5a,0x5a,0x5a,0x5a};
byte msg[5];
byte tx_status;

void setup()
{
	Serial.begin(9600);	
	while(!Serial.available());
	Serial.read();
	Serial.print("Setup...");
	RF_setup(5, rx_address);
	Serial.print("Setting txAddress...");
	RF_txAddress(tx_address);
	Serial.println("Done");
}

void loop()
{
	// Block until character sent from screen, then transmit and return result
	for (int i = 0; i<=(5-1); i++)
	{
		while(!Serial.available());
		msg[i] = Serial.read();
		Serial.println((char) msg[i]);
	}

	Serial.print("Sending:");
	Serial.print((char)msg[0]);
	Serial.print((char)msg[1]);
	Serial.print((char)msg[2]);
	Serial.print((char)msg[3]);
	Serial.println((char)msg[4]);

	tx_status = RF_tx(msg);

	if (tx_status == 0)
	{
		Serial.println(" : MAX_RT");
	}
	else if (tx_status == 1)
	{
		Serial.println(" : SENT");
	}
	else
	{
		Serial.println(" : WEIRD");
	}
	Serial.println(RF_status(),BIN);
}