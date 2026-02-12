/***************************************************************
 * Lecture des paquets échangés entre la centrale VERISURE
 * ES6500VSF-ES-M06 (SW version 7.18) et  les capteurs 
 *  d'ouverture  ES700MGLS et le lecteur de badges ES700TR5 
 * 
 * Configuration des registres du CC1101 après analyse avec le logiciel Logic2
 *  du bus SPI de la centrale ES6500VSF
 *  
 *  RF device: CC1101 en 2-FSK pour lecture détecteur VERISURE 
 * Ce sniffer fonctionne également si la centrale est hors service
 ***************************************************************/
#include <ELECHOUSE_CC1101_SRC_DRV.h>

/******* Prototypes *******/
int calcRssi(int);

// Valeurs des registres du CC1101 pour les paquets VERISURE
#define SETTING_IOCFG2     0x0B		
#define SETTING_IOCFG1     0x2E
#define SETTING_IOCFG0     0x06		
#define SETTING_FIFOTHR    0x07		
#define SETTING_SYNC1      0x45
#define SETTING_SYNC0      0x53
#define SETTING_PKTLEN     0x3D
#define SETTING_PKTCTRL1   0x64		
#define SETTING_PKTCTRL0   0x01
#define SETTING_ADDR       0x00
#define SETTING_CHANNR     0x00
#define SETTING_FSCTRL1    0x06
#define SETTING_FSCTRL0    0x49
#define SETTING_FREQ2      0x21
#define SETTING_FREQ1      0x65
#define SETTING_FREQ0      0x6C 
#define SETTING_MDMCFG4    0x5A
#define SETTING_MDMCFG3    0x83
#define SETTING_MDMCFG2    0x02
#define SETTING_MDMCFG1    0x02
#define SETTING_MDMCFG0    0xF8
#define SETTING_DEVIATN    0x32
#define SETTING_MCSM2      0x07
#define SETTING_MCSM1      0x30
#define SETTING_MCSM0      0x18
#define SETTING_FOCCFG     0x16
#define SETTING_BSCFG      0x1C
#define SETTING_AGCCTRL2   0xC7
#define SETTING_AGCCTRL1   0x00
#define SETTING_AGCCTRL0   0xB2
#define SETTING_WOREVT1    0x87
#define SETTING_WOREVT0    0x6B
#define SETTING_WORCTRL    0xF8
#define SETTING_FREND1     0x56
#define SETTING_FREND0     0x10
#define SETTING_FSCAL3     0xE9
#define SETTING_FSCAL2     0x2A
#define SETTING_FSCAL1     0x00
#define SETTING_FSCAL0     0x1F
#define SETTING_RCCTRL1    0x41
#define SETTING_RCCTRL0    0x00
#define SETTING_FSTEST     0x59
#define SETTING_PTEST      0x7F
#define SETTING_AGCTEST    0x3F
#define SETTING_TEST2      0x81
#define SETTING_TEST1      0x31
#define SETTING_TEST0      0x09

#define   BYTES_IN_RXFIFO   0x7F            //byte number in RXfifo
#define   RX_OVERFLOW       0x80            //bit d'overflow

#ifdef ESP32
int pin = 4;        //pin 4 pour ESP32 (éviter pin 2 car doit être à zéro lors du bootloader), 17 pour WEMOS D1 MINI ESP32 sur ILI 9341
#elif ESP8266
int pin = 4;  // for esp8266! Receiver on pin 4 = D2.
#else
int pin = 0;  // for Arduino! Receiver on interrupt 0 => that is pin #2
#endif    

static const char *Register[] = {
"IOCFG2",
"IOCFG1",
"IOCFG0",
"FIFOTHR",
"SYNC1",
"SYNC0",
"PKTLEN",
"PKTCTRL1",
"PKTCTRL0",
"ADDR",
"CHANNR",
"FSCTRL1",
"FSCTRL0",
"FREQ2",
"FREQ1",
"FREQ0",
"MDMCFG4",
"MDMCFG3",
"MDMCFG2",
"MDMCFG1",
"MDMCFG0",
"DEVIATN",
"MCSM2",
"MCSM1",
"MCSM0",
"FOCCFG",
"BSCFG",
"AGCCTRL2",
"AGCCTRL1",
"AGCCTRL0",
"WOREVT1",
"WOREVT0",
"WORCTRL",
"FREND1",
"FREND0",
"FSCAL3",
"FSCAL2",
"FSCAL1",
"FSCAL0",
"RCCTRL1",
"RCCTRL0",
"FSTEST",
"PTEST",
"AGCTEST",
"TEST2",
"TEST1",
"TEST0",
};

byte bufferR[61] = {0};

void setup() {
	pinMode(5, OUTPUT);					//CS de l'afficheur TFT ILI9341
	pinMode(21, OUTPUT);				//CS du clavier sensitif de TFT ILI9341
	digitalWrite(5, HIGH);
	digitalWrite(21, HIGH);
	
	Serial.begin(115200);
	Serial.println(F("START " __FILE__ " from " __DATE__));
	ELECHOUSE_cc1101.setSpiPin(18, 19, 23, 5); //(sck, miso, mosi, csn1) ESP32
	//ELECHOUSE_cc1101.setSpiPin(18, 19, 23, 16); //(sck, miso, mosi, csn1) WEMOS-D1 MINI ESP32 sur ILI9341, VSPI
	//ELECHOUSE_cc1101.setSpiPin(32, 12, 13, 16); //(sck, miso, mosi, csn1) WEMOS-D1 MINI ESP32 sur ILI9341, cablage sur HSPI en partie

	Serial.println("Interrogation du CC1101");
	//ELECHOUSE_cc1101.Init();		//initialise le bus SPI

  if (ELECHOUSE_cc1101.getCC1101()){       // Check the CC1101 Spi connection.
  Serial.println("Connection OK");
  }else{
  Serial.println("Connection Error");
  }
  ELECHOUSE_cc1101.Init();		//initialise le bus SPI
  ELECHOUSE_cc1101.setGDO0(pin);

	cc1101_RegConfigSettings();
	Serial.println(F("Init CC1101 OK"));
	
	ELECHOUSE_cc1101.SetRx();  // set Receive on
}

void loop() {
  
	if (ELECHOUSE_cc1101.CheckReceiveFlag()) {		//si un paquet est reçu par le CC1101 on l'affiche
		Serial.print("<");
		byte len = ReceiveData(bufferR);
		Serial.print(len);
		Serial.print("> ");
		for (int i=0; i<len; i++) {
      if(bufferR[i] < 0x10) Serial.print('0');
      Serial.print(bufferR[i],HEX);
      Serial.print(" ");
    }
		Serial.println("");
		Serial.print(F("Rssi:"));				//affiche le RSSI du paquet reçu
		Serial.print(calcRssi(bufferR[len-2]));
		Serial.print("  LQI:");					//affiche LQI du paquet reçu (masque le CRC)
		Serial.println(bufferR[len-1] & 0x7F);
	}
    //delay(10); 
     
  if (Serial.available())	{		//Sur commande clavier affiche l'état du CC1101
    int i=Serial.read();
	  switch (i) {
		  case 's':
			  print_all_status();
			  break;
		  case 'r':
			  show_registers();
			  break;
		  default:
			  Serial.print("Marcstate=");  //debug
			  Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_MARCSTATE),HEX); //debug affiche l'état de la "machine"// statements
			  break;
	  }
  }
}
/****************************************************************
*FUNCTION NAME:ReceiveData
*FUNCTION     :read data received from RXfifo
*INPUT        :rxBuffer: buffer to store data
*OUTPUT       :size of data received
****************************************************************/
byte ReceiveData(byte *rxBuffer)
{
	byte size;
	byte status[2];

	if(ELECHOUSE_cc1101.SpiReadStatus(CC1101_RXBYTES) & BYTES_IN_RXFIFO)		//teste le nombre de bytes dans le RX FIFO
	{
		//size=SpiReadReg(CC1101_RXFIFO);	//code d'origine, la longueur utile est le premier byte après le Sync word --> pb si 2 paquet dans le RX FIFO
		size = ELECHOUSE_cc1101.SpiReadStatus(CC1101_RXBYTES) & BYTES_IN_RXFIFO; //mon code afin de comparer avec log du data analyser -> lit tout ce qui a été reçu
		ELECHOUSE_cc1101.SpiReadBurstReg(CC1101_RXFIFO,rxBuffer,size);
		//SpiReadBurstReg(CC1101_RXFIFO,status,2); //code d'origine, on met l'append status dans status
		ELECHOUSE_cc1101.SpiStrobe(CC1101_SFRX);
		ELECHOUSE_cc1101.SpiStrobe(CC1101_SRX);
		return size;
	}
	else
	{
		ELECHOUSE_cc1101.SpiStrobe(CC1101_SFRX);
		ELECHOUSE_cc1101.SpiStrobe(CC1101_SRX);
 		return 0;
	}
}
/****************************************************************
*FUNCTION NAME:RegConfigSettings
*FUNCTION     :CC1101 register config //details refer datasheet of CC1101/CC1100
*INPUT        :none
*OUTPUT       :none
****************************************************************/
void cc1101_RegConfigSettings(void) 
{   
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_IOCFG2, SETTING_IOCFG2);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_IOCFG1, SETTING_IOCFG1);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_IOCFG0, SETTING_IOCFG0);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_FIFOTHR, SETTING_FIFOTHR);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_SYNC1, SETTING_SYNC1);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_SYNC0, SETTING_SYNC0);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_PKTLEN, SETTING_PKTLEN);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_PKTCTRL1, SETTING_PKTCTRL1);   
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_PKTCTRL0, SETTING_PKTCTRL0);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_ADDR, SETTING_ADDR);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_CHANNR, SETTING_CHANNR);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_FSCTRL1, SETTING_FSCTRL1);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_FSCTRL0, SETTING_FSCTRL0);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_FREQ2, SETTING_FREQ2);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_FREQ1, SETTING_FREQ1);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_FREQ0, SETTING_FREQ0);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_MDMCFG4, SETTING_MDMCFG4);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_MDMCFG3, SETTING_MDMCFG3);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_MDMCFG2, SETTING_MDMCFG2);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_MDMCFG1, SETTING_MDMCFG1);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_MDMCFG0, SETTING_MDMCFG0);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_DEVIATN, SETTING_DEVIATN);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_MCSM2, SETTING_MCSM2);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_MCSM1, SETTING_MCSM1);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_MCSM0, SETTING_MCSM0);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_FOCCFG, SETTING_FOCCFG);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_BSCFG, SETTING_BSCFG);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_AGCCTRL2, SETTING_AGCCTRL2);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_AGCCTRL1, SETTING_AGCCTRL1);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_AGCCTRL0, SETTING_AGCCTRL0);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_WOREVT1, SETTING_WOREVT1);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_WOREVT0, SETTING_WOREVT0);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_WORCTRL, SETTING_WORCTRL);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_FREND1, SETTING_FREND1);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_FREND0, SETTING_FREND0);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_FSCAL3, SETTING_FSCAL3);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_FSCAL2, SETTING_FSCAL2);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_FSCAL1, SETTING_FSCAL1);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_FSCAL0, SETTING_FSCAL0);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_RCCTRL1, SETTING_RCCTRL1);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_RCCTRL0, SETTING_RCCTRL0);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_FSTEST, SETTING_FSTEST);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_PTEST, SETTING_PTEST);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_AGCTEST, SETTING_AGCTEST);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_TEST2, SETTING_TEST2);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_TEST1, SETTING_TEST1);
	ELECHOUSE_cc1101.SpiWriteReg(CC1101_TEST0, SETTING_TEST0);
}
//----------------------------------------------------
void print_all_status (void)  //test interrogation CC1101 OK affichage des registres d'état
{
	Serial.println(F("Registres de status du CC1101 (HEX)"));
	Serial.print (F("PARTNUM=")); Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_PARTNUM),HEX);
	Serial.print (F("VERSION="));Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_VERSION),HEX);
	Serial.print (F("FREQEST="));Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_FREQEST),HEX);
	Serial.print (F("LQI="));Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_LQI),HEX);
	Serial.print (F("RSSI="));Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_RSSI),HEX);
	Serial.print (F("RSSI en dBm (DEC)="));Serial.println(ELECHOUSE_cc1101.getRssi());
	Serial.print (F("MARCSTATE="));Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_MARCSTATE),HEX);
	Serial.print (F("WORTIME1="));Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_WORTIME1),HEX);
	Serial.print (F("WORTIME0="));Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_WORTIME0),HEX);
	Serial.print (F("PKTSTATUS="));Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_PKTSTATUS),HEX);
	Serial.print (F("VCO_VC_DAC="));Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_VCO_VC_DAC),HEX);
	Serial.print (F("TXBYTES="));Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_TXBYTES),HEX);
	Serial.print (F("RXBYTES="));Serial.println(ELECHOUSE_cc1101.SpiReadStatus(CC1101_RXBYTES),HEX);
}
//----------------------------------------------------
void show_registers(void)
{
	Serial.println("CC1101 Register (HEX)");
	for (int i = 0; i<=46; i++){
		print_register(i);
	}
}
//----------------------------------------------------
void print_register(byte b)
{
	Serial.print(Register[b]);
	int len = strlen(Register[b]);
	len = 10 - len;
	for (int i = 0; i<len; i++){
		Serial.print(" ");
	}
  uint8_t value = ELECHOUSE_cc1101.SpiReadStatus(b);
	if (value < 0x10) Serial.print('0');
  Serial.println(value, HEX);  
}
//----------------------------------------------------

/****************************************************************
*FUNCTION NAME:RSSI Level
*FUNCTION     :Calculating the RSSI Level
*INPUT        :none
*OUTPUT       :none
****************************************************************/
int calcRssi(int rssi)
{
	if (rssi >= 128){rssi = (rssi-256)/2-74;}
	else{rssi = (rssi/2)-74;}
	return rssi;
}
//***************************************************************

