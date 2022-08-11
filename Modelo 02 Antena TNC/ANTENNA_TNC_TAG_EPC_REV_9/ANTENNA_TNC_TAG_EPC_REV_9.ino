/*
  REV. 09 ULTIMA ATUALIZACAO 25/06/2019
  PROGRAMADOR E DESENVOLVEDOR: ELIEZER MORAES SILVA

  VERSÃO COM ITENS MC (MECÂNICOS) HABILITADOS

*/

// ******************************************************************************
// ******************* ANTENA LEITORA RFID UHF TECNOLOGIA TNC *******************
// ******************************************************************************

//INTERVALO DE CÓDIGOS DO EL0111 AO EL1000 E MC1001 AO 2000

#include <SparkFun_UHF_RFID_Reader.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <SD.h>
#include <SPI.h>
#include <DS3231.h>

#define delayScan 150
#define quantidadeProdutos 2000
#define quantidadePorProdutos 100
#define ledRED A0
#define ledGREEN A1
#define ledBLUE A2
#define COMMON_ANODE
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
String dia = "";
String mes = "";
String ano = "";
String tempConteudoSDcard = "";
String fileNameDataLogger = "movement.txt";
String conteudoSCANNERpartNumber = "";
String conteudoSCANNERlastNumber = "";
String conteudoSCANNERfiltrado = "";
String conteudoRecebidoBLE = "";
String conteudoESPELHOpartNumber = "";
String horaBackupEstoque = "17:00:00"; //(h/min/seg)
String backupFileName = "B";
String conteudoBackup = "";
String pacoteDadosCadastrador = "";

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
void backupDiariodoEstoque();
void enviarValoresCadastrador();

byte num = 0;
byte tagEPCBytes = 0;

int numeroProduto = 110;
int numeroProdutoCadastrador = 111;
int valorQuantidadePRODUTO = 0;
int temperatura = 0;
int tempValorQuantidadePRODUTO = 0;

bool remoteToHost = false;
bool estadoLeitura = false;
bool backupMode = false;

void setup() {

  Serial.begin(9600);     // INICIALIZA A COMUNICAÇÃO SERIAL
  Serial3.begin(38400);   // INICIALIZA A COMUNICAÇÃO UART BLUETOOTH HOST_ESPELHO
  Serial2.begin(9600);   // INTERFACE DE COMUNICAÇÃO UART HC-12 HOST_REMOTA
  Serial1.begin(38400);   // INTERFACE DE COMUNICAÇÃO UART BLUETOOTH HOST_SRTR
  RTC.begin();

  pinMode(ledRED, OUTPUT);
  pinMode(ledGREEN, OUTPUT);
  pinMode(ledBLUE, OUTPUT);

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

  //for (int e = 1001 ; e <= 2000 ; e++) {
    //EEPROM.write(e, 0);
  //}

}

void loop() {

  if (!backupMode && hora == horaBackupEstoque) {

    backupMode = true;

  }

  if (backupMode) {

    dadosRTC();
    backupDiariodoEstoque();

  } else {

    if (!remoteToHost) leituraTAGS();

    listenSRTR();
    enviarValoresCadastrador();
    listenREMOTE();
    listenESPELHO();
    dadosRTC();

    if (!remoteToHost && !estadoLeitura) {
      atualizacaoHOSTparaREMOTA();
      setColorRGB(1, 0, 1);           //COR VERDE
    }

    if (remoteToHost && !estadoLeitura) {
      atualizacaoREMOTAparaHOST();
      setColorRGB(1, 1, 0);           //COR AZUL
    }

    if (!estadoLeitura) {

      Serial.println("Leitura em andamento...");
      delay(delayScan);

    }
  }
}

void listenSRTR() {

  if (Serial1.available()) {

    setColorRGB(1, 1, 0); //COR AZUL

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

      if (SD.exists(txt)) {

        SD.remove(txt);
        Serial.println("TAG já cadastrada! Substituindo o produto cadastrado na tag...");

      }

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

      dadosRTC();

      sinal = "+";

      dataloggerADD();

      sinal = "";

    }
  }
}

void enviarValoresCadastrador() {

  pacoteDadosCadastrador = String(numeroProdutoCadastrador) + ',' + String(EEPROM.read(numeroProdutoCadastrador));

  Serial1.println(pacoteDadosCadastrador); //Envia para o cadastrador uma STRING, um pacote de dados, contendo o Part Number e a quantidade correspondente do produto.
  //(Ex. 111,2) (partnumber,quantidade)
  Serial.println(pacoteDadosCadastrador);

  numeroProdutoCadastrador++;

  if (numeroProdutoCadastrador > quantidadeProdutos) numeroProdutoCadastrador = 111;

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

      setColorRGB(1, 1, 0); //COR AZUL

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

  Serial.println(conteudoSCANNERfiltrado);

  if (SD.exists(conteudoSCANNERfiltrado + ".txt")) {

    setColorRGB(0, 1, 1); //COR VERMELHO

    valorQuantidadePRODUTO = EEPROM.read(conteudoSCANNERpartNumber.toInt());

    valorQuantidadePRODUTO--; //DECREMENTA A QUANTIDADE DO PRODUTO ----------------------------------------------------

    if (valorQuantidadePRODUTO > 150 && valorQuantidadePRODUTO <= 255) { //Instrução para não permitir que a quantidade mude de 0 para 255, por alguma falha de leitura da tag.
      valorQuantidadePRODUTO = 0; //Se a falha ocorrer, o valor final vai para 0.
    }

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
    SD.remove(conteudoSCANNERfiltrado + ".txt");

    conteudoSCANNERfiltrado = "";

    Serial.println(" - Exists.");

  } else {

    Serial.println(" - Doesn't Exist. LEITURA ANTENA TNC");

  }
}

void BLOCO_COMPARADOR_CONTADOR_ESPELHO() {

  Serial.println(conteudoESPELHO);

  if (SD.exists(conteudoESPELHO + ".txt")) {

    setColorRGB(0, 1, 1); //COR VERMELHO

    valorQuantidadePRODUTO = EEPROM.read(conteudoESPELHOpartNumber.toInt());

    valorQuantidadePRODUTO--; //DECREMENTA A QUANTIDADE DO PRODUTO ----------------------------------------------------

    if (valorQuantidadePRODUTO > 150 && valorQuantidadePRODUTO <= 255) { //Instrução para não permitir que a quantidade mude de 0 para 255, por alguma falha de leitura da tag.
      valorQuantidadePRODUTO = 0; //Se a falha ocorrer, o valor final vai para 0.
    }

    EEPROM.write(conteudoESPELHOpartNumber.toInt(), valorQuantidadePRODUTO);

    Serial.print("PART NUMBER ESPELHO: ");
    Serial.println(conteudoESPELHOpartNumber.toInt());
    Serial.print("QUANTIDADE EM ESTOQUE: ");
    Serial.println(valorQuantidadePRODUTO);

    dadosRTC();

    sinal = "-";

    datalogger();

    sinal = "";

    Serial.println("Removendo arquivo .txt...");

    SD.remove(conteudoESPELHO + ".txt");

    Serial.println(" - Exists.");

    conteudoESPELHO = "";

  } else {

    Serial.println(" - Doesn't Exist. LEITURA ANTENA RS232");

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

  // FORMATO DE DATA -> DD.MM.AAAA
  // FORMATO DE HORA -> H:MIN:S

  data = RTC.getDateStr();
  hora = RTC.getTimeStr();
  temperatura = RTC.getTemp();

  Serial.println(data);
  Serial.println(hora);
  Serial.println(temperatura);

  dia = data.substring(0, data.indexOf('.'));
  mes = data.substring(data.indexOf('.') + 1, (data.length() - data.lastIndexOf('.')));
  ano = data.substring((data.length() - data.lastIndexOf('.')) + 3, data.length());

  Serial.println(dia);
  Serial.println(mes);
  Serial.println(ano);

}

void atualizacaoHOSTparaREMOTA() {

  if (numeroProduto < quantidadeProdutos) {

    numeroProduto++;
    int valorProdutoBLE = EEPROM.read(numeroProduto);

    Serial2.println(String(numeroProduto - 110) + ',' + String(valorProdutoBLE));
    //Serial.println(String(numeroProduto) + ',' + String(valorProdutoBLE));

  }

  if (numeroProduto == quantidadeProdutos) numeroProduto = 110;

}

void listenREMOTE() {

  if (Serial2.available()) {

    conteudoBLE_REMOTA = Serial2.readStringUntil('\n');

    Serial.println(conteudoBLE_REMOTA);

    remoteToHost = true;

  } else {

    remoteToHost = false;

  }
}

void listenESPELHO() {

  if (Serial3.available()) {

    conteudoESPELHO = Serial3.readStringUntil(',');

    conteudoESPELHOpartNumber = conteudoESPELHO.substring(0, 4);

    setColorRGB(1, 1, 0); //COR AZUL

    BLOCO_COMPARADOR_CONTADOR_ESPELHO();

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

void backupDiariodoEstoque() {

  //******* 17:00 horas ******* Horário para o sistema efetuar o backup dos dados de estoque e salvar uma cópia diariamente no cartão microSD.

  backupFileName = backupFileName + dia + mes + ano;

  myFile = SD.open(backupFileName + ".txt", FILE_WRITE);

  if (myFile) {

    for (int d = 111; d <= quantidadeProdutos; d++) {

      Serial.print(d);
      Serial.println(" - Criando arquivo de backup...");

      conteudoBackup = String(d) + ',' + String(EEPROM.read(d));

      myFile.println(conteudoBackup);

      delay(10);

    }

    myFile.close();

    Serial.println("Backup criado com sucesso!");

    backupMode = false;

  } else {

    Serial.println("Erro ao criar arquivo de backup...");

    backupMode = false;

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
