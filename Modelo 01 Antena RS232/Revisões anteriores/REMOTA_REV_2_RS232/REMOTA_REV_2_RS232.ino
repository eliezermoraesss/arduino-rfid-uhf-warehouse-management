/*
   REV. 1 ULTIMA ATUALIZACAO 19/02/2019
   PROGRAMADOR E DESENVOLVEDOR: ELIEZER MORAES SILVA
*/

// **************************************************************
// ******************* REMOTA RFID MANUTENÇÃO *******************
// **************************************************************

#include <Wire.h>
#include "RTClib.h"

#define delayScan 300
#define quantidadeProdutos 110
#define ledRED 26
#define ledGREEN 33
#define ledBLUE 35
#define pinBotaoRegQtd 32

RTC_DS3231 rtc;

void atualizacaoHOSTparaREMOTA();
void registrarQuantidades();
void atualizaExcel();
void botaoRegistro();
void atualizaExcelColunaEspelho();
void setColorRGB();

String conteudo = "";
String codigoPartNumber = "";
String charPartNumber = "";
String quantity = "";
String comandoSETCelulaQuantidadeExcel = "CELL,SET,O";
String comandoSETCelulaADDExcel = "CELL,SET,P";
String comandoGETCelulaQuantidadeExcel = "CELL,GET,Q";
String comandoGETCelulaADDExcel = "CELL,GET,P";
String comandoSETCelulaQuantidadeExcelEspelho = "CELL,SET,Q";
String hora = "";
String minuto = "";
String segundo = "";
String horaEmail = "";
String horaStart = "10:0";
String horaEnd = "10:1";

bool modoRegistrar = true;
bool modoRegistarAnterior = true;
bool modoRegistro = true;
bool resetUpdate = false;
bool stopAtualizacao = false;
bool stopExcel = false;
bool horaDisparoEmail = true;

int valorCorrecaoQuantidade = 0;
int posicaoRegistro = 0;
int posicaoLinhaExcel = 1;
int Reset = 0;
int dadosRecebidos = 0;
int pulsos = 0;
uint32_t tempoPausaEmail = 86400000; //24 horas

uint32_t delayEnvioEmail = 0;

void setup() {

  Serial.begin(38400);
  Serial1.begin(38400, SERIAL_8N1, 4, 2);   // INTERFACE DE COMUNICAÇÃO UART BLUETOOTH

  pinMode(ledRED, OUTPUT);
  pinMode(ledGREEN, OUTPUT);
  pinMode(ledBLUE, OUTPUT);
  pinMode(pinBotaoRegQtd, INPUT_PULLUP);

#ifndef ESP8266
  while (!Serial); // for Leonardo/Micro/Zero
#endif

  if (!rtc.begin()) {

    Serial.println("Couldn't find RTC");

    while (1) {

      digitalWrite(ledRED, LOW);
      digitalWrite(ledGREEN, HIGH);
      digitalWrite(ledBLUE, HIGH);
      delay(200);
      digitalWrite(ledRED, HIGH);
      digitalWrite(ledGREEN, HIGH);
      digitalWrite(ledBLUE, HIGH);
      delay(100);

    }
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2019, 1, 17, 16, 29, 0));
  }

  digitalWrite(ledRED, LOW);
  digitalWrite(ledGREEN, HIGH);
  digitalWrite(ledBLUE, HIGH);

  delay(20000); //Espera 10 segundos para ocorrer o pareamento completo com a HOST

}

void loop() {

  DateTime now = rtc.now();
  hora = (now.hour());
  minuto = (now.minute());
  segundo = (now.second());
  horaEmail = hora + ':' + minuto;
  Serial.println(horaEmail);

  if (horaEmail == horaStart && horaDisparoEmail) {

    stopExcel = false;

    pulsos++;

    Serial.println("Enviando e-mails!");
    Serial.println(pulsos);

    if (pulsos == quantidadeProdutos) {

      horaDisparoEmail = false;
      pulsos = 0;
      stopExcel = true;

    }
  }

  if (horaEmail == horaEnd) {

    horaDisparoEmail = true;

  }

  botaoRegistro();

  if (modoRegistro) {

    setColorRGB(1, 0, 1); // VERDE

    atualizacaoHOSTparaREMOTA();

  } else if (!modoRegistro) {

    setColorRGB(1, 1, 0); // BLUE

    registrarQuantidades();

  }

  delay(delayScan);

}

void atualizacaoHOSTparaREMOTA() {

  if (Serial1.available()) {

    conteudo = Serial1.readStringUntil('\n');

    if (conteudo.length() <= 9) {

      dadosRecebidos++;

      charPartNumber = conteudo.indexOf(',');
      codigoPartNumber = conteudo.substring(0, charPartNumber.toInt());
      quantity = conteudo.substring(charPartNumber.toInt() + 1, conteudo.length());

      /*Serial.print("STRING RECEBIDA: ");
      Serial.println(conteudo);
      Serial.print("PART NUMBER: ");
      Serial.println(codigoPartNumber);
      Serial.print("QUANTIDADE: ");
      Serial.println(quantity);*/


      if (!stopExcel && dadosRecebidos == quantidadeProdutos) {

        stopExcel = true;
        dadosRecebidos = 0;

      }

      if (!stopExcel) atualizaExcel();

      if (stopExcel) atualizaExcelColunaEspelho();

    }
  }
}

void atualizaExcel() {

  codigoPartNumber = codigoPartNumber.toInt() + 1;

  String setQuantidadeCelula = comandoSETCelulaQuantidadeExcel + String(codigoPartNumber) + ',' + quantity.toInt();

  Serial.println(setQuantidadeCelula);

}

void atualizaExcelColunaEspelho() {

  codigoPartNumber = codigoPartNumber.toInt() + 1;

  String setQuantidadeCelula = comandoSETCelulaQuantidadeExcelEspelho + String(codigoPartNumber) + ',' + quantity.toInt();

  Serial.println(setQuantidadeCelula);

}

void botaoRegistro() {

  modoRegistrar = digitalRead(pinBotaoRegQtd);

  if (modoRegistrar && !modoRegistarAnterior) {

    modoRegistro = !modoRegistro;
    posicaoRegistro = 0;
    posicaoLinhaExcel = 1;
    Serial.println("MODO REGISTRO");

  }
  modoRegistarAnterior = modoRegistrar;
}

void registrarQuantidades() {

  posicaoRegistro++;
  posicaoLinhaExcel++;

  Serial.println(comandoGETCelulaQuantidadeExcel + String(posicaoLinhaExcel));  //COLUNA Q
  int TEMPAnt = Serial.parseInt();
  Serial.println(comandoGETCelulaADDExcel + String(posicaoLinhaExcel));         //COLUNA P
  int TEMP = Serial.parseInt();

  valorCorrecaoQuantidade = TEMPAnt + TEMP;

  if (!stopExcel) {

    String setQuantidadeCelula = comandoSETCelulaQuantidadeExcel + String(posicaoLinhaExcel) + ',' + String(valorCorrecaoQuantidade);
    Serial.println(setQuantidadeCelula);

  } else {

    String setQuantidadeCelula = comandoSETCelulaQuantidadeExcelEspelho + String(posicaoLinhaExcel) + ',' + String(valorCorrecaoQuantidade);
    Serial.println(setQuantidadeCelula);

  }

  String setReset = comandoSETCelulaADDExcel + String(posicaoLinhaExcel) + ',' + String(Reset);
  Serial.println(setReset);

  Serial1.println(String(posicaoRegistro) + ',' + String(valorCorrecaoQuantidade));

  if (posicaoRegistro == quantidadeProdutos) {

    posicaoRegistro = 0;
    posicaoLinhaExcel = 1;
    modoRegistro = true;

  }
}

void setColorRGB(int vermelho, int verde, int azul) {

#ifdef COMMON_ANODE
  vermelho = 1 - vermelho;
  verde = 1 - verde;
  azul = 1 - azul;
#endif

  digitalWrite(ledRED, vermelho);
  digitalWrite(ledGREEN, verde);
  digitalWrite(ledBLUE, azul);

}
