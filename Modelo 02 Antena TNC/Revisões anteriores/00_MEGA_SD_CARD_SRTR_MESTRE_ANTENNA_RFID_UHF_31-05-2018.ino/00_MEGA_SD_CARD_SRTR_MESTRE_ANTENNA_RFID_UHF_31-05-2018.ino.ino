/*
  REV. 00 ULTIMA ATUALIZACAO 08/10/2018
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
#define ledRED 7
#define ledGREEN 6
#define ledBLUE 5
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

byte num = 0;
byte tagEPCBytes = 0;

int numeroProduto = 0;
int posicaoRegistro;
int valorQuantidadePRODUTO = 0;
int temperatura = 0;
int tempValorQuantidadePRODUTO = 0;

bool remoteToHost = false;
bool estadoLeitura = false;

void setup() {

  Serial.begin(115200);    // Inicia a comunicacao serial
  Serial1.begin(38400);   // INTERFACE DE COMUNICAÇÃO UART BLUETOOTH HOST_SRTR
  Serial3.begin(38400);   // INTERFACE DE COMUNICAÇÃO UART BLUETOOTH HOST_REMOTA
  RTC.begin();

  pinMode(ledRED, OUTPUT);
  pinMode(ledGREEN, OUTPUT);
  pinMode(ledBLUE, OUTPUT);
  pinMode(STATUS, OUTPUT);

  while (!Serial);
  //Serial.println();
  //Serial.println("Initializing...");

  if (setupNano(38400) == false) //Configure nano to run at 38400bps
  {
    //Serial.println("Module failed to respond. Please check wiring.");
    while (1); //Freeze!
  }

  nano.setRegion(REGION_NORTHAMERICA); //Set to North America

  nano.setReadPower(500); //5.00 dBm. Higher values may cause USB port to brown out
  //Max Read TX Power is 27.00 dBm and may cause temperature-limit throttling

  nano.startReading(); //Begin scanning for tags

  if (!SD.begin(SS)) {

    Serial.println("Initialization failed!");
    digitalWrite(STATUS, 1); //FALHA CARTÃO MICRO SD NÃO IDENTIFICADO
    return;

  } else {

    Serial.println("Initialization done.");
    digitalWrite(STATUS, 0);  //CARTÃO MICRO SD IDENTIFICADO

  }

  //for (int e = 1 ; e <= quantidadeProdutos ; e++) {
  //EEPROM.write(e, 0);
  //}

}

void loop() {

  if (!remoteToHost) leituraTAGS();
  listenSRTR();
  listenREMOTE();

  if (!remoteToHost && !estadoLeitura) {
    atualizacaoHOSTparaREMOTA();
    setColorRGB(0, 255, 0);           //COR VERDE
  }

  if (remoteToHost && !estadoLeitura) {
    atualizacaoREMOTAparaHOST();
    setColorRGB(0, 0, 255);           //COR AZUL
  }

  if (!estadoLeitura) {
    
    Serial.println("Leitura...");
    delay(delayScan);
    
  }
}

void listenSRTR() {

  if (Serial1.available()) {

    setColorRGB(0, 0, 255); //AZUL

    conteudoBLE = Serial1.readStringUntil('\n');

    num++;

    if (num == 1) {

      byte numberCharString = conteudoBLE.length();

      if (numberCharString == 4) txt = conteudoBLE.substring(0, 3) + ".txt";
      if (numberCharString == 5) txt = conteudoBLE.substring(0, 4) + ".txt";
      if (numberCharString == 6) txt = conteudoBLE.substring(0, 5) + ".txt";
      if (numberCharString == 7) txt = conteudoBLE.substring(0, 6) + ".txt";
      if (numberCharString == 8) txt = conteudoBLE.substring(0, 7) + ".txt";
      if (numberCharString == 9) txt = conteudoBLE.substring(0, 8) + ".txt";

      Serial.println(txt);
      Serial.println(numberCharString);

    }

    Serial.println("DADO BLE RECEBIDO");
    Serial.println(conteudoBLE);

    if (num == 2) {

      WRITE_TAG_SD_CARD();

      num = 0;

      myFile = SD.open(txt);

      if (myFile) {

        while (myFile.available()) {

          conteudoSDcard = myFile.readStringUntil('\n');

          tempConteudoSDcard = conteudoSDcard.toInt();

          valorQuantidadePRODUTO = EEPROM.read(conteudoSDcard.toInt());

          valorQuantidadePRODUTO++; //INCREMENTA A QUANTIDADE DO PRODUTO ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

          tempValorQuantidadePRODUTO = valorQuantidadePRODUTO;

          EEPROM.write(conteudoSDcard.toInt(), valorQuantidadePRODUTO);

          Serial.print("PART NUMBER: ");
          Serial.println(conteudoSDcard);
          Serial.print("QUANTIDADE EM ESTOQUE: ");
          Serial.println(valorQuantidadePRODUTO);

        }

        myFile.close();

      } else {

        Serial.println("ERRO AO LER O ARQUIVO.txt");

      }

      //dadosRTC();

      sinal = "+";

      dataloggerADD();

      sinal = "";

    }
  }
}

void WRITE_TAG_SD_CARD() {

  Serial.println("GRAVANDO NO CARTÃO SD");

  myFile = SD.open(txt, FILE_WRITE);

  if (myFile) {

    Serial.print("Writing TAG in SD CARD");
    myFile.println(conteudoBLE);              //GRAVA OS DADOS DA STRING RECEBIDA VIA BLUETOOOTH NO CARTÃO MICRO SD

    myFile.close();
    Serial.println(" Done!");

  } else {

    Serial.println("ERRO AO CRIAR O ARQUIVO.txt");

  }
}

void leituraTAGS() {

  if (nano.check() == true) { //Check to see if any new data has come in from module

    byte responseType = nano.parseResponse(); //Break response into tag ID, RSSI, frequency, and timestamp

    if (responseType == RESPONSE_IS_KEEPALIVE) {

      estadoLeitura = false;

      //Serial.println(F("Scanning"));

    } else if (responseType == RESPONSE_IS_TAGFOUND) {

      estadoLeitura = true;

      //If we have a full record we can pull out the fun bits
      int rssi = nano.getTagRSSI(); //Get the RSSI for this tag read

      long freq = nano.getTagFreq(); //Get the frequency this tag was detected at

      long timeStamp = nano.getTagTimestamp(); //Get the time this was read, (ms) since last keep-alive message*/

      tagEPCBytes = nano.getTagEPCBytes(); //Get the number of bytes of EPC from response

      Serial.print(" rssi[");
      Serial.print(rssi);
      Serial.print("]");

      Serial.print(" freq[");
      Serial.print(freq);
      Serial.print("]");

      Serial.print(" time[");
      Serial.print(timeStamp);
      Serial.print("]");

      //Print EPC bytes, this is a subsection of bytes from the response/msg array
      Serial.print(" epc[");

      for (byte x = 0 ; x < tagEPCBytes ; x++) {

        if (x < 4) {

          conteudo.concat(String(nano.msg[31 + x], HEX));

        } else {

          if (nano.msg[31 + x] < 0x10) conteudo = conteudo + "0";
          conteudo.concat(String(nano.msg[31 + x], HEX));

        }

      }

      Serial.print(conteudo);

      Serial.println("]");

      conteudoSCANNERpartNumber = conteudo.substring(0, 4);
      //Serial.println(conteudoSCANNERpartNumber);

      conteudoSCANNERlastNumber = conteudo.substring(conteudo.length() - 4, conteudo.length());
      //Serial.println(conteudoSCANNERlastNumber);

      conteudoSCANNERfiltrado = conteudoSCANNERpartNumber.toInt() + conteudoSCANNERlastNumber;
      Serial.println(conteudoSCANNERfiltrado);

      BLOCO_COMPARADOR_CONTADOR();

      conteudo = "";

    } else if (responseType == ERROR_CORRUPT_RESPONSE) {

      //Serial.println("Bad CRC");

    } else {

      //Unknown response
      //Serial.print("Unknown error");

    }
  }
}

void BLOCO_COMPARADOR_CONTADOR() {

  if (SD.exists(conteudoSCANNERfiltrado + ".txt")) {

    valorQuantidadePRODUTO = EEPROM.read(conteudoSCANNERpartNumber.toInt());

    valorQuantidadePRODUTO--; //DECREMENTA A QUANTIDADE DO PRODUTO ----------------------------------------------------

    EEPROM.write(conteudoSCANNERpartNumber.toInt(), valorQuantidadePRODUTO);

    Serial.print("PART NUMBER: ");
    Serial.println(conteudoSCANNERpartNumber);
    Serial.print("QUANTIDADE EM ESTOQUE: ");
    Serial.println(valorQuantidadePRODUTO);

    //dadosRTC();

    sinal = "-";

    datalogger();

    sinal = "";

    Serial.println("Removendo arquivo .txt...");
    SD.remove(conteudoSCANNERfiltrado + ".txt");

    conteudoSCANNERfiltrado = "";

    Serial.println(" - Exists.");

  } else {

    //Serial.println(" - Doesn't Exist.");

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

void dataloggerADD() {

  dadosDatalogger = data + ',' + hora + ',' + tempConteudoSDcard + ',' + sinal + ',' + String(tempValorQuantidadePRODUTO);

  Serial.println(dadosDatalogger);

  myFile = SD.open(fileNameDataLogger, FILE_WRITE);

  if (myFile) {

    Serial.print("SALVANDO INFORMAÇÃO DE HISTÓRICO.");
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

void atualizacaoHOSTparaREMOTA() {

  if (numeroProduto < quantidadeProdutos) {

    numeroProduto++;
    int valorProdutoBLE = EEPROM.read(numeroProduto);

    Serial3.println(String(numeroProduto) + ',' + String(valorProdutoBLE));

  }

  if (numeroProduto == quantidadeProdutos) numeroProduto = 0;

}

void listenREMOTE() {

  if (Serial3.available()) {

    conteudoBLE_REMOTA = Serial3.readStringUntil('\n');

    Serial.println(conteudoBLE_REMOTA);

    remoteToHost = true;

  } else {

    remoteToHost = false;

  }
}

void atualizacaoREMOTAparaHOST() {

  charPartNumber = conteudoBLE_REMOTA.indexOf(',');
  codigoPartNumber = conteudoBLE_REMOTA.substring(0, charPartNumber.toInt());
  quantity = conteudoBLE_REMOTA.substring(charPartNumber.toInt() + 1, conteudoBLE_REMOTA.length());

  if (conteudoBLE_REMOTA.length() <= 9) {

    EEPROM.write(codigoPartNumber.toInt(), quantity.toInt());

    Serial.print("STRING RECEBIDA: ");
    Serial.println(conteudoBLE_REMOTA);
    Serial.print("PART NUMBER: ");
    Serial.println(codigoPartNumber);
    Serial.print("QUANTIDADE: ");
    Serial.println(quantity);
    Serial.println();

  }
}

void setColorRGB(int vermelho, int verde, int azul) {

#ifdef COMMON_ANODE
  vermelho = 255 - vermelho;
  verde = 255 - verde;
  azul = 255 - azul;
#endif

  analogWrite(ledRED, vermelho);
  analogWrite(ledGREEN, verde);
  analogWrite(ledBLUE, azul);

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
