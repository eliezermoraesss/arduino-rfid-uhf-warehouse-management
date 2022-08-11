/*
  REV. 09 ULTIMA ATUALIZACAO 19/02/2019
  PROGRAMADOR E DESENVOLVEDOR: ELIEZER MORAES SILVA
*/

// *********************************************************************
// ************ ANTENA LEITORA RFID UHF TECNOLOGIA RS232 ***************
// *********************************************************************

// INTERVALO DE CÓDIGOS DO EL0001 AO EL0110

#include <EEPROM.h>
#include <SD.h>
#include <SPI.h>
#include <DS3231.h>
#include <SoftwareSerial.h>

#define delayScan 100
#define delayRemota 300
#define quantidadeProdutos 110
#define quantidadePorProdutos 100
#define minimaEstoque 1
#define ledRED 7
#define ledGREEN 6
#define ledBLUE 5
#define COMMON_ANODE
#define STATUS 13
#define SS 53
#define pinEstadoBluetoothRemota 24

#define senhaGsm "1234"
#define pinBotaoCall 12
#define numeroCall "974018811"

File myFile;
DS3231 RTC(SDA, SCL);
SoftwareSerial gsmSerial(10, 11); // RX, TX

String conteudo = "";
String conteudoBLE = "";
String conteudoBLE_REMOTA = "";
String conteudoEPC = "";
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
String MarcosChante = "+5519997126815";
String EliezerMoraes = "+5519974018811";
String CarlosFaria = "+5519974043659";
String celular = MarcosChante;

unsigned char tag[18];
unsigned char b0;
unsigned char b1;
unsigned char b2;
unsigned char b3;
unsigned char b4;
unsigned char b5;
unsigned char b6;
unsigned char b7;
unsigned char b8;
unsigned char b9;
unsigned char b10;
unsigned char b11;
unsigned char b12;
unsigned char b13;
unsigned char b14;
unsigned char b15;
unsigned char b16;
unsigned char b17;
unsigned char b18;
unsigned char b19;
unsigned char b20;
unsigned char b21;

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
void gsmControle();
void smsNotificacao();
void smsCompra();

byte num = 0;

int numeroProduto = 0;
int posicaoRegistro;
int valorQuantidadePRODUTO = 0;
int temperatura = 0;
int tempValorQuantidadePRODUTO = 0;
unsigned long tempoRTC = 0;

bool remoteToHost = false;
bool estadoBluetoothPareadoRemota = false;

bool temSMS = false;
String telefoneSMS;
String dataHoraSMS;
String mensagemSMS;
String mensagemSMSResposta = "";
String comandoGSM = "";
String ultimoGSM = "";
String horaT, minutoT, segundoT, tempoT = "";

bool callStatus = false;

void leGSM();
void enviaSMS(String telefone, String mensagem);
void fazLigacao(String telefone);
void configuraGSM();

void setup() {

  Serial.begin(9600);    // Inicia a comunicacao serial
  Serial1.begin(38400);   // INTERFACE DE COMUNICAÇÃO UART BLUETOOTH HOST_SRTR
  Serial2.begin(38400);   // INTERFACE DE COMUNICAÇÃO UART SHIELD RS232
  Serial3.begin(38400);   // INTERFACE DE COMUNICAÇÃO UART BLUETOOTH HOST_REMOTA
  gsmSerial.begin(9600);
  Serial2.setTimeout(150);
  RTC.begin();

  pinMode(ledRED, OUTPUT);
  pinMode(ledGREEN, OUTPUT);
  pinMode(ledBLUE, OUTPUT);
  pinMode(STATUS, OUTPUT);
  pinMode(pinBotaoCall, INPUT_PULLUP);

  if (!SD.begin(SS)) {

    while (1) {

      digitalWrite(STATUS, 1); //FALHA CARTÃO MICRO SD NÃO IDENTIFICADO

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

    digitalWrite(STATUS, 0);  //CARTÃO MICRO SD IDENTIFICADO

  }

  //for (int e = 1 ; e <= quantidadeProdutos ; e++) {
  //EEPROM.write(e, 0);
  //}

  configuraGSM();

}

void loop() {

  setColorRGB(0, 255, 0);           //COR VERDE

  estadoBluetoothPareadoRemota = digitalRead(pinEstadoBluetoothRemota);

  leituraTAGS();
  leGSM();
  gsmControle();
  listenSRTR();
  listenREMOTE();

  if (estadoBluetoothPareadoRemota) {

    if (!remoteToHost) {
      atualizacaoHOSTparaREMOTA();
    }

    if (remoteToHost) {
      atualizacaoREMOTAparaHOST();
      setColorRGB(0, 0, 255);           //COR AZUL
    }
  }

  if (!estadoBluetoothPareadoRemota) delay(delayScan);

  if (estadoBluetoothPareadoRemota) delay(delayRemota);

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

      if (SD.exists(txt)) {

        SD.remove(txt);
        Serial.println("TAG já cadastrada! Substituindo o produto na tag...");

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

  if (Serial2.available()) {

    digitalWrite(STATUS, 1);
    setColorRGB(0, 0, 255); //AZUL

    Serial.println("DADOS RECEBIDOS");

    for (int i = 0; i <= 18; i++) {

      tag[i] = Serial2.read(); // Leitura dos dados e tratamento dos dados

      if (i == 1) b1 = tag[1];
      if (i == 2) b2 = tag[2];
      if (i == 3) b3 = tag[3];
      if (i == 4) b4 = tag[4];
      if (i == 5) b5 = tag[5];
      if (i == 6) b6 = tag[6];
      if (i == 7) b7 = tag[7];
      if (i == 8) b8 = tag[8];
      if (i == 9) b9 = tag[9];
      if (i == 10) b10 = tag[10];
      if (i == 11) b11 = tag[11];
      if (i == 12) b12 = tag[12];
      if (i == 13) b13 = tag[13];
      if (i == 14) b14 = tag[14];
      if (i == 15) b15 = tag[15];
      if (i == 16) b16 = tag[16];
      if (i == 17) b17 = tag[17];
      if (i == 18) b18 = tag[18];

    }

    unsigned char PRODUTO[18] = {b4, b5, b6, b7, b14, b15};

    for (char g = 0; g < 6; g++) {

      if (g < 4) {

        conteudo.concat(String(PRODUTO[g], HEX));

      } else {

        if (PRODUTO[g] < 0x10) conteudo = conteudo + "0";
        conteudo.concat(String(PRODUTO[g], HEX));

      }
    }

    String partNumber = conteudo.substring(0, 4);
    String lastNumber = conteudo.substring(4, 8);
    conteudoEPC = partNumber.toInt() + lastNumber;

    //Serial2.flush();
    Serial.println(conteudo);
    Serial.println(partNumber.toInt());
    Serial.println(lastNumber);
    Serial.println(conteudoEPC);

    BLOCO_COMPARADOR_CONTADOR();


  } else {

    digitalWrite(STATUS, 0);

  }

  conteudo = "";

}

void BLOCO_COMPARADOR_CONTADOR() {

  if (SD.exists(conteudoEPC + ".txt")) {

    setColorRGB(255, 0, 0);           //COR VERMELHO

    myFile = SD.open(conteudoEPC + ".txt");

    if (myFile) {

      if (myFile.available()) {

        conteudoSDcard = myFile.readStringUntil('\n');

        valorQuantidadePRODUTO = EEPROM.read(conteudoSDcard.toInt());

        valorQuantidadePRODUTO--; //DECREMENTA A QUANTIDADE DO PRODUTO ----------------------------------------------------

        EEPROM.write(conteudoSDcard.toInt(), valorQuantidadePRODUTO);

        Serial.print("PART NUMBER: ");
        Serial.println(conteudoSDcard);
        Serial.print("QUANTIDADE EM ESTOQUE: ");
        Serial.println(valorQuantidadePRODUTO);

      }
      // close the file:
      Serial.println("Fechando arquivo .txt...");

      myFile.close();

      dadosRTC();

      sinal = "-";
      datalogger();
      sinal = "";

      if (valorQuantidadePRODUTO > minimaEstoque) {

        smsNotificacao();

      } else {

        smsCompra();

      }

      mensagemSMSResposta = "";

      Serial.println("Removendo arquivo .txt...");
      SD.remove(conteudoEPC + ".txt");

    } else {

      Serial.println("ERRO AO LER O ARQUIVO.txt");

    }

    conteudo = "";

    Serial.println(" - Exists.");

  } else {

    Serial.println(" - Doesn't Exist.");

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
  hora = RTC.getTimeStr();
  temperatura = RTC.getTemp();
  Serial.println(hora);

  horaT = hora.substring(0, 2);
  minutoT = hora.substring(3, 5);
  segundoT = hora.substring(6, 8);
  tempoT = horaT + minutoT + segundoT;
  tempoRTC = tempoT.toInt();
  Serial.println(tempoRTC);

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

void leGSM()
{
  static String textoRec = "";
  static unsigned long delay1 = 0;
  static int count = 0;
  static unsigned char buffer[64];

  if (gsmSerial.available()) {

    while (gsmSerial.available()) {

      buffer[count++] = gsmSerial.read();
      if (count == 64)break;
    }

    textoRec += (char*)buffer;
    delay1   = millis();

    for (int i = 0; i < count; i++) {
      buffer[i] = NULL;
    }
    count = 0;
  }


  if ( ((millis() - delay1) > 100) && textoRec != "" ) {

    if ( textoRec.substring(2, 7) == "+CMT:" ) {
      temSMS = true;
    }

    if (temSMS) {

      telefoneSMS = "";
      dataHoraSMS = "";
      mensagemSMS = "";

      byte linha = 0;
      byte aspas = 0;
      for (int nL = 1; nL < textoRec.length(); nL++) {

        if (textoRec.charAt(nL) == '"') {
          aspas++;
          continue;
        }

        if ( (linha == 1) && (aspas == 1) ) {
          telefoneSMS += textoRec.charAt(nL);
        }

        if ( (linha == 1) && (aspas == 5) ) {
          dataHoraSMS += textoRec.charAt(nL);
        }

        if ( linha == 2 ) {
          mensagemSMS += textoRec.charAt(nL);
        }

        if (textoRec.substring(nL - 1, nL + 1) == "\r\n") {
          linha++;
        }
      }
    } else {
      comandoGSM = textoRec;
    }

    textoRec = "";
  }
}

void enviaSMS(String telefone, String mensagem) {
  gsmSerial.print("AT+CMGS=\"" + telefone + "\"\n");
  gsmSerial.print(mensagem + "\n");
  gsmSerial.print((char)26);
}

void fazLigacao(String telefone) {
  gsmSerial.println("ATH0\n");
  gsmSerial.print((char)26);
  gsmSerial.println("ATD " + telefone + ";\n");
  gsmSerial.print((char)26);
}

void configuraGSM() {
  gsmSerial.print("AT+CMGF=1\n;AT+CNMI=2,2,0,0,0\n;ATX4\n;AT+COLP=1\n");
}

void gsmControle() {

  if (comandoGSM != "") {
    Serial.println(comandoGSM);
    ultimoGSM = comandoGSM;
    comandoGSM = "";
  }

  if (temSMS) {

    Serial.println("Chegou Mensagem!!");
    Serial.println();

    Serial.print("Remetente: ");
    Serial.println(telefoneSMS);
    Serial.println();

    Serial.print("Data/Hora: ");
    Serial.println(dataHoraSMS);
    Serial.println();

    Serial.println("Mensagem:");
    Serial.println(mensagemSMS);
    Serial.println();

    mensagemSMS.trim();

    if (mensagemSMS == senhaGsm) {

      //mensagemSMSResposta = data + '\n' + hora + '\n' + "O produto EL" + conteudoSDcard;

      Serial.println("Mensagem Recebida!");
      //enviaSMS(telefoneSMS, mensagemSMSResposta);

    }

    temSMS = false;

  }

  if (!digitalRead(pinBotaoCall) && !callStatus) {
    Serial.println("Afetuando Ligacao...");
    fazLigacao(numeroCall);
    callStatus = true;
  }

  if (ultimoGSM.indexOf("+COLP:") > -1) {
    Serial.println("LIGACAO EM ANDAMENTO");
    ultimoGSM = "";
  }

  if (ultimoGSM.indexOf("NO CARRIER") > -1) {
    Serial.println("LIGACAO TERMINADA");
    ultimoGSM = "";
    callStatus = false;
  }

  if (ultimoGSM.indexOf("BUSY") > -1) {
    Serial.println("LINHA/NUMERO OCUPADO");
    ultimoGSM = "";
    callStatus = false;
  }

  if (ultimoGSM.indexOf("NO DIALTONE") > -1) {
    Serial.println("SEM LINHA");
    ultimoGSM = "";
    callStatus = false;
  }

  if (ultimoGSM.indexOf("NO ANSWER") > -1) {
    Serial.println("NAO ATENDE");
    ultimoGSM = "";
    callStatus = false;
  }
}

void smsNotificacao() {

  mensagemSMSResposta = "SOGEFI RFID SYSTEM\n>>> MOVIMENTACAO <<<\n\nO PRODUTO EL" + conteudoSDcard + " FOI RETIRADO DO ESTOQUE\n\nQUANTIDADE ATUAL -> " + valorQuantidadePRODUTO + '\n' + '\n' + data + '\n' + hora + '\n' + temperatura + " graus Celsius";

  Serial.println("Enviando SMS de notificação!");
  enviaSMS(celular, mensagemSMSResposta);

}

void smsCompra() {

  mensagemSMSResposta = "SOGEFI RFID SYSTEM\n>>> ALERTA DE COMPRA <<<\n\nPRODUTO EL" + conteudoSDcard + '\n' + '\n' + "ESTOQUE MINIMO ATINGIDO\nQUANTIDADE ATUAL -> " + valorQuantidadePRODUTO + '\n' + '\n' + data + '\n' + hora + '\n' + temperatura + " graus Celsius";

  Serial.println("Enviando SMS de compra!");
  enviaSMS(celular, mensagemSMSResposta);

}
