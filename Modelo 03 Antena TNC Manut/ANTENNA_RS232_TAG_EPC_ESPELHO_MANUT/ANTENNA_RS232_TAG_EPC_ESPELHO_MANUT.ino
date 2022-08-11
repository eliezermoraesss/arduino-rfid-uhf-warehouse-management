/*
  REV. 01 ULTIMA ATUALIZACAO 11/04/2019
  PROGRAMADOR E DESENVOLVEDOR: ELIEZER MORAES SILVA
*/

#include <SD.h>
#include <SPI.h>
#include <DS3231.h>

// ************************************************************************
// ******************* ANTENA LEITORA RFID RS232 ESPELHO ******************
// ************************************************************************

// INTERVALO DE CÓDIGOS DO EL0111 AO EL1000

#define delayScan 150
#define ledRED 7
#define ledGREEN 6
#define ledBLUE 5
#define COMMON_ANODE
#define STATUS 13
#define pinEstadoBluetoothHost 24
#define SS 53

File myFile;
DS3231 RTC(SDA, SCL);

bool estadoBluetoothPareadoHOST = false;

int temperatura = 0;

String conteudo = "";
String partNumber = "";
String dadosDatalogger = "";
String data = "";
String hora = "";
String sinal = "-";
String fileNameDataLogger = "movEspAx.txt";

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
void datalogger();
void setColorRGB();

void setup() {

  Serial.begin(9600);     //Start the Serial Communication
  Serial1.begin(38400);   //INTERFACE DE COMUNICAÇÃO UART BLUETOOTH ESPELHO_HOST
  Serial2.begin(38400);   //INTERFACE DE COMUNICAÇÃO UART SHIELD RS232
  RTC.begin();
  //Serial2.setTimeout(150);

  pinMode(ledRED, OUTPUT);
  pinMode(ledGREEN, OUTPUT);
  pinMode(ledBLUE, OUTPUT);
  pinMode(STATUS, OUTPUT);

  if (!SD.begin(SS)) {

    while (1) {

      Serial.println("Initialization failed!");

      //FALHA CARTÃO MICRO SD NÃO IDENTIFICADO
      digitalWrite(ledRED, 255);
      digitalWrite(ledGREEN, 0);
      digitalWrite(ledBLUE, 0);
      delay(50);
      digitalWrite(ledRED, 0);
      digitalWrite(ledGREEN, 0);
      digitalWrite(ledBLUE, 0);
      delay(50);

    }

  } else {

    Serial.println("Initialization done.");

    //CARTÃO MICRO SD IDENTIFICADO
    digitalWrite(ledRED, 0);
    digitalWrite(ledGREEN, 255);
    digitalWrite(ledBLUE, 0);

  }

}

void loop() {

  //estadoBluetoothPareadoHOST = true;
  
  estadoBluetoothPareadoHOST = digitalRead(pinEstadoBluetoothHost);

  while (!estadoBluetoothPareadoHOST) {

    setColorRGB(255, 0, 0); // VERMELHO
    delay(200);

    setColorRGB(0, 0, 0);
    delay(100);

    estadoBluetoothPareadoHOST = digitalRead(pinEstadoBluetoothHost);

  }

  setColorRGB(0, 255, 0); // VERDE

  leituraTAGS();

  delay(delayScan);

}

void leituraTAGS() {

  if (Serial2.available()) {

    digitalWrite(STATUS, 1);

    setColorRGB(0, 0, 255); // AZUL
    //setColorRGB(0xFF, 0xA5, 0x00); // LARANJA

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

    partNumber = conteudo.substring(0, 4);
    String lastNumber = conteudo.substring(4, 8);
    String conteudoEPC = partNumber.toInt() + lastNumber + ',';

    //Serial2.flush();
    Serial.println(conteudo);
    Serial.println(partNumber.toInt());
    Serial.println(lastNumber);
    Serial.println(conteudoEPC);

    Serial1.print(conteudoEPC); //Send the EPC data to HOST >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    dadosRTC();
    datalogger();

  } else {

    digitalWrite(STATUS, 0);

  }

  conteudo = "";

}

void dadosRTC() {

  data = RTC.getDateStr();
  Serial.println(data);

  hora = RTC.getTimeStr();
  Serial.println(hora);

  temperatura = RTC.getTemp();
  Serial.println(temperatura);

}

void datalogger() {

  dadosDatalogger = data + ',' + hora + ',' + partNumber.toInt() + ',' + sinal;
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
