// ----------------------------------------------------------------------------
// PTX_Setup_Test
// version: 1.0.0
// date: 2013-01-04
// author: Conor O'Brien
// ----------------------------------------------------------------------------

#include <RF_Nordic.h>
#include <SPI.h>

byte rx_address[5] = {0x5f,0x5a,0x5a,0x5f,0x5f};
byte tx_address[5] = {0x5a,0x5a,0x5a,0x5a,0x5a};
byte write[5];
byte read[5];
byte msg;
byte status;

void setup()
{
	//Start USB
	Serial.begin(9600);

// ********************************************************
// RF_SETUP
	//Start SPI
	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV32);

	//Set pins (apparently have to declare imputs on Teensy3.1)
	pinMode(SS_PIN, OUTPUT);
	digitalWrite(SS_PIN, HIGH);
	pinMode(CE_PIN, OUTPUT);
	digitalWrite(CE_PIN, HIGH);
	pinMode(IRQ_PIN, INPUT);

	//Write PWR_UP, EN_CRC and PRIM_RX to CONFIG
	write[0] = (1<<PWR_UP)|(1<<EN_CRC)|(1<<PRIM_RX);
	RF_writeReg(CONFIG, write, 1);

	//EN_RXADDR default values (defaults, just to make sure for now)
	write[0] = (1<<ERX_P0)|(1<<ERX_P1);
	RF_writeReg(EN_RXADDR, write, 1);

	//Address width (5 for now)
	write[0] = 0x03;
	RF_writeReg(SETUP_AW, write, 1);

	//SETUP_RETR for auto-acks, wait 500 us, 10 retries
	write[0] = 0x1a;
	RF_writeReg(SETUP_RETR, write, 1);

	//RF_CH, sets frequency channel
	write[0] = 0x02;
	RF_writeReg(RF_CH, write, 1);

	//STATUS, writes to all interrupts, acts as reset
	write[0] = 0x70;
	RF_writeReg(STATUS, write, 1);

	//Write rx_address to pipe 1
	RF_writeReg(RX_ADDR_P1, rx_address, 5);

	//Set pipe 0 payload width to 1 byte (for auto-acks)
	write[0] = 0x01;
	RF_writeReg(RX_PW_P0, write, 1);

	//Set pipe 1 payload width to 1 byte
	RF_writeReg(RX_PW_P1, write, 1);

// ********************************************************
// RF_TXADDR

	RF_writeReg(TX_ADDR, tx_address, 5);
	RF_writeReg(RX_ADDR_P0, tx_address, 5);

// ********************************************************
}

void loop()
{
	//Read character to send from keypress
	while(!Serial.available());
	msg = Serial.read();

	//write 1 byte to TX_PLD FIFO
	digitalWrite(SS_PIN, LOW);
	SPI.transfer(W_TX_PAYLOAD);
	SPI.transfer(msg);
	digitalWrite(SS_PIN, HIGH);
	delayMicroseconds(2);

	//Set CE to low before going into TX mode
	digitalWrite(CE_PIN, LOW);
	//Set PRIM_RX to 0;
	write[0] = (1<<PWR_UP)|(1<<EN_CRC);
	RF_writeReg(CONFIG, write, 1);
	
	//Pulse CE_PIN high for 15 us
	digitalWrite(CE_PIN, HIGH);
	delayMicroseconds(15);
	digitalWrite(CE_PIN, LOW);

	//block until IRQ trigger, then check status for interrupts
	while(digitalRead(IRQ_PIN));
	digitalWrite(SS_PIN, LOW);
	status = SPI.transfer(NOP);
	digitalWrite(SS_PIN, HIGH);

	//Return result to console and reset interrupt
	Serial.println(status, BIN);
	Serial.print((char)msg);

	if (status & (1<<TX_DS))
	{
		Serial.println(" : SENT");
		write[0] = (1<<TX_DS);
		RF_writeReg(STATUS, write, 1);

		digitalWrite(SS_PIN, LOW);
		status = SPI.transfer(NOP);
		digitalWrite(SS_PIN, HIGH);
		Serial.println(status, BIN);

	}
	else if (status & (1<<MAX_RT))
	{
		Serial.println(" : MAX_RT");
		write[0] = (1<<MAX_RT);
		RF_writeReg(STATUS, write, 1);

		//Clear TX buffer
		digitalWrite(SS_PIN, LOW);
		status = SPI.transfer(FLUSH_TX);
		digitalWrite(SS_PIN, HIGH);
		delayMicroseconds(2);

		digitalWrite(SS_PIN, LOW);
		status = SPI.transfer(NOP);
		digitalWrite(SS_PIN, HIGH);
		Serial.println(status, BIN);
	}
	else
	{
		Serial.println(" : WEIRD");
	}

	//Turn radio back into a reciever
	write[0] = (1<<PWR_UP)|(1<<EN_CRC)|(1<<PRIM_RX);
	RF_writeReg(CONFIG, write, 1);

}