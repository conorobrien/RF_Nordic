// ----------------------------------------------------------------------------
// PRX_Test.ino
// version: 1.2.0
// date: 2014-01-08
// author: Conor O'Brien
// ----------------------------------------------------------------------------

#include <RF_Nordic.h>
#include <SPI.h>
byte status;

byte rx_address[5] = {0x5a,0x5a,0x5a,0x5a,0x5a};
byte msg[1];
int i = 0;

void setup()
{
	Serial.begin(9600);
	while (!Serial.available());
	Serial.read();
	Serial.print("Setup...");
	RF_setup(5, rx_address);
	Serial.println("Done");
}

void loop()
{
	while (!RF_rxAvailable());
	Serial.println(RF_status(), BIN);
	Serial.println("READING...");
	RF_rx(msg);
	Serial.print((char)msg[0]);
	Serial.print((char)msg[1]);
	Serial.print((char)msg[2]);
	Serial.print((char)msg[3]);
	Serial.println((char)msg[4]);
	Serial.print(i);
	Serial.println(" :----------");
	i++;
}