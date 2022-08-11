/*
  REV. 02 ULTIMA ATUALIZACAO 12/12/2018
  PROGRAMADOR E DESENVOLVEDOR: ELIEZER MORAES SILVA
*/

//******************* ANTENNA MESTRE *******************

#include <SparkFun_UHF_RFID_Reader.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <SD.h>
#include <SPI.h>
#include <DS3231.h>

#define delayScan 100
#define quantidadeProdutos 1000
#define quantidadePorProdutos 100
#define ledRED A0
#define ledGREEN A1
#define ledBLUE A2
#define COMMON_ANODE
#define STATUS 13
#define SS 53

RFID nano; //Create instance
SoftwareSerial softSerial(11, 10); //TX, RX ARDUINO MEGA 2560 --> RX, TX SHIELD RFID SRTR SPARKFUN
File myFile;
DS3231 RTC(SDA, SCL);

String conteudo = "";
String conteudoBLE = "";
String conteudoBLE_REMOTA = "";
String conteudoESPELHO = "";
String txt = "";
String conteudoSDcard = "";
String codigoPartNumber = "";
String charPartNumber = "";
String quantity = "";
String charQuantity = "";
String dadosDatalogger = "";
String data = "";
String hora = "";
String sinal = "";
String tempConteudoSDcard = "";
String fileNameDataLogger = "movement.txt";
String conteudoSCANNERpartNumber = "";
String conteudoSCANNERlastNumber = "";
String conteudoSCANNERfiltrado = "";
String conteudoRecebidoBLE = "";

void leituraTAGS();
void atualizacaoHOSTparaREMOTA();
void atualizacaoREMOTAparaHOST();
void listenSRTR();
void WRITE_TAG_SD_CARD();
void BLOCO_COMPARADOR_CONTADOR();
void listenREMOTE();
void setColorRGB();
void datalogger();
void dataloggerADD();
void listenESPELHO();
void BLOCO_COMPARADOR_CONTADOR_ESPELHO();

byte num = 0;
byte tagEPCBytes = 0;

int numeroProduto = 110;
int posicaoRegistro;
int valorQuantidadePRODUTO = 0;
int temperatura = 0;
int tempValorQuantidadePRODUTO = 0;

bool remoteToHost = false;
bool estadoLeitura = false;

void setup() {

  Serial.begin(9600);     // INICIALIZA A COMUNICAÇÃO SERIAL
  Serial3.begin(38400);   // INICIALIZA A COMUNICAÇÃO UART BLUETOOTH HOST_ESPELHO
  Serial2.begin(38400);   // INTERFACE DE COMUNICAÇÃO UART BLUETOOTH HOST_REMOTA
  Serial1.begin(38400);   // INTERFACE DE COMUNICAÇÃO UART BLUETOOTH HOST_SRTR
  RTC.begin();

  pinMode(ledRED, OUTPUT);
  pinMode(ledGREEN, OUTPUT);
  pinMode(ledBLUE, OUTPUT);
  pinMode(STATUS, OUTPUT);

  if (!SD.begin(SS)) {

    while (1) {

      Serial.println("Initialization failed!");

      //FALHA CARTÃO MICRO SD NÃO IDENTIFICADO
      digitalWrite(ledRED, 0);
      digitalWrite(ledGREEN, 1);
      digitalWrite(ledBLUE, 1);
      delay(50);
      digitalWrite(ledRED, 1);
      digitalWrite(ledGREEN, 1);
      digitalWrite(ledBLUE, 1);
      delay(50);

    }

  } else {

    Serial.println("Initialization done.");

    //CARTÃO MICRO SD IDENTIFICADO
    digitalWrite(ledRED, 1);
    digitalWrite(ledGREEN, 0);
    digitalWrite(ledBLUE, 1);

  }

  //while (!Serial);
  //Serial.println();
  //Serial.println("Initializing...");

  if (setupNano(38400) == false) //Configure nano to run at 38400bps
  {
    while (1) { //Freeze!
      Serial.println("Module failed to respond. Please check wiring.");
      digitalWrite(ledRED, 0);
      digitalWrite(ledGREEN, 1);
      digitalWrite(ledBLUE, 1);
      delay(80);
      digitalWrite(ledRED, 1);
      digitalWrite(ledGREEN, 1);
      digitalWrite(ledBLUE, 1);
      delay(150);
    }
  }

  nano.setRegion(REGION_NORTHAMERICA); //Set to North America

  nano.setReadPower(2600); //5.00 dBm. Higher values may cause USB port to brown out
  //Max Read TX Power is 27.00 dBm and may cause temperature-limit throttling

  nano.startReading(); //Begin scanning for tags

  //for (int e = 1 ; e <= quantidadeProdutos ; e++) {
  //EEPROM.write(e, 0);
  //}

}

void loop() {

  listenESPELHO();

  delay(delayScan);

}

void BLOCO_COMPARADOR_CONTADOR_ESPELHO() {

  if (SD.exists(conteudoESPELHO + ".txt")) {

    setColorRGB(0, 1, 1); //COR VERMELHO

    valorQuantidadePRODUTO = EEPROM.read(conteudoSCANNERpartNumber.toInt());

    valorQuantidadePRODUTO--; //DECREMENTA A QUANTIDADE DO PRODUTO ----------------------------------------------------

    EEPROM.write(conteudoSCANNERpartNumber.toInt(), valorQuantidadePRODUTO);

    Serial.print("PART NUMBER: ");
    Serial.println(conteudoSCANNERpartNumber);
    Serial.print("QUANTIDADE EM ESTOQUE: ");
    Serial.println(valorQuantidadePRODUTO);

    dadosRTC();

    sinal = "-";

    datalogger();

    sinal = "";

    Serial.println("Removendo arquivo .txt...");
    SD.remove(conteudoESPELHO + ".txt");

    Serial.println(" - Exists.");
    conteudoRecebidoBLE = "";
    conteudoESPELHO = "";

  } else {

    Serial.println(" - Doesn't Exist. LEITURA ANTENA RS232");
    conteudoRecebidoBLE = "";
    conteudoESPELHO = "";

  }
}

void datalogger() {

  dadosDatalogger = data + ',' + hora + ',' + conteudoSDcard.toInt() + ',' + sinal + ',' + String(valorQuantidadePRODUTO);
  Serial.println(dadosDatalogger);

  myFile = SD.open(fileNameDataLogger, FILE_WRITE);

  if (myFile) {

    Serial.print("SALVANDO INFORMAÇÃO DE HISTÓRICO");
    myFile.println(dadosDatalogger);

    myFile.close();
    Serial.println(" Salvo!");

  } else {

    Serial.println("ERRO AO CRIAR O ARQUIVO DATALOGGER.txt");

  }
}

void dadosRTC() {

  data = RTC.getDateStr();
  Serial.println(data);

  hora = RTC.getTimeStr();
  Serial.println(hora);

  temperatura = RTC.getTemp();
  Serial.println(temperatura);

}

void listenESPELHO() {

  if (Serial3.available()) {

    conteudoESPELHO = Serial3.readStringUntil(',');

    Serial.println(conteudoESPELHO);

    BLOCO_COMPARADOR_CONTADOR_ESPELHO();

  }
}

void setColorRGB(int vermelho, int verde, int azul) {

  /*#ifdef COMMON_ANODE
    vermelho = 1 - vermelho;
    verde = 1 - verde;
    azul = 1 - azul;
    #endif*/

  digitalWrite(ledRED, vermelho);
  digitalWrite(ledGREEN, verde);
  digitalWrite(ledBLUE, azul);

}

//Gracefully handles a reader that is already configured and already reading continuously
//Because Stream does not have a .begin() we have to do this outside the library
boolean setupNano(long baudRate)
{
  nano.begin(softSerial); //Tell the library to communicate over software serial port

  //Test to see if we are already connected to a module
  //This would be the case if the Arduino has been reprogrammed and the module has stayed powered
  softSerial.begin(baudRate); //For this test, assume module is already at our desired baud rate
  while (!softSerial); //Wait for port to open

  //About 200ms from power on the module will send its firmware version at 115200. We need to ignore this.
  while (softSerial.available()) softSerial.read();

  nano.getVersion();

  if (nano.msg[0] == ERROR_WRONG_OPCODE_RESPONSE)
  {
    //This happens if the baud rate is correct but the module is doing a ccontinuous read
    nano.stopReading();

    Serial.println(F("Module continuously reading. Asking it to stop..."));

    delay(1500);
  }
  else
  {
    //The module did not respond so assume it's just been powered on and communicating at 115200bps
    softSerial.begin(115200); //Start software serial at 115200

    nano.setBaud(baudRate); //Tell the module to go to the chosen baud rate. Ignore the response msg

    softSerial.begin(baudRate); //Start the software serial port, this time at user's chosen baud rate
  }

  //Test the connection
  nano.getVersion();
  if (nano.msg[0] != ALL_GOOD) return (false); //Something is not right

  //The M6E has these settings no matter what
  nano.setTagProtocol(); //Set protocol to GEN2

  nano.setAntennaPort(); //Set TX/RX antenna ports to 1

  return (true); //We are ready to rock
}
