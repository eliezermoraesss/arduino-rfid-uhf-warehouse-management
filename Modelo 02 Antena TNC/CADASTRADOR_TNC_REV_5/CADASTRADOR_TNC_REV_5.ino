/*
   REV. 5 ULTIMA ATUALIZACAO 29/06/2019
   PROGRAMADOR E DESENVOLVEDOR: ELIEZER MORAES SILVA

   VERSÃO COM ITENS MC (MECÂNICOS) HABILITADOS

*/

// ********************************************************************
// ************* CADASTRADOR DE TAGS RFID UHF PARA ANTENA TNC *********
// ********************************************************************

// INTERVALO DE CÓDIGOS DO EL0111 AO EL1000 E MC1001 AO 2000

#include <SparkFun_UHF_RFID_Reader.h> //Library for controlling the M6E Nano module
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Keypad.h>

#define delayScan 150
#define delayLCDcadastro 2000
#define delayEscritaEPC 500
#define maxQuantidade 100
#define maxPosicao 2000
#define limiteEL 1000
#define ledRED 7
#define ledGREEN 6
#define ledBLUE 5
#define COMMON_ANODE
#define pinEstadoBluetoothRemota 2
#define pinEstadoBluetoothHost 3
#define I2C_ADDRESS 0x27
#define BACKLIGHT_PIN 3
#define En 2
#define Rw 1
#define Rs 0
#define D4 4
#define D5 5
#define D6 6
#define D7 7
#define SS 53

#define BUZZER1 9
#define BUZZER2 10
//#define BUZZER1 0 //For testing quietly

String conteudo = "";
String conteudoEPC = "";
String conteudoSCANNER = "";
String conteudoSCANNERpartNumber = "";
String conteudoSCANNERlastNumber = "";
String conteudoSCANNERfiltrado = "";
String conteudoVALIDO = "";
String partNumberEPC = "";
String lastNumberEPC = "";
String partNumber = "EL";
String partNumbermMec = "MC";
String inventFile = "invent.txt";
String conteudoLogInventario = "";
String milKey, centKey, dezKey, uniKey = "";
String numeroTeclado = "";
String conteudoEPCSD = "";
String conteudoTID;
String pacoteDadosCadastrador = "";
String numeroProdutoCadastrador = "";
String quantidadeProdutoCadastrador = "";

char tecla_pressionada;

void botaoOKselecao();
void botaoCancelar();
void botaoSelecaoCategoria();
void menuSelecao();
void leituraTID();
void leituraEPC();
void listaComparacaoTAGS();
void condicoesControleCiclos();
void sendStringToHost();
void WRITE_TID_SD_CARD();
void WRITE_EPC_SD_CARD();
void calculoMatematico();
void calculoMatematicoObterPN();
void inventarioRFID();
void contagemInventario();
void setColorRGB();
void dataloggerInventario();
void tecladoMatricial4x4();
void apagarTIDeEPCdoCartaoSD();
void excluirArquivoTID();
void excluirArquivoEPC();
void recebeDadosHost();

bool quantidadeProdCadastrar = false;
bool comparadorEPC = false;
bool cancelaLeituraTag = false;
bool cartaoSDstatus = true;
bool estadoBluetoothPareadoREMOTA = true;
bool estadoBluetoothPareadoHOST = true;
bool inventarioAtivado = false;
bool erroBluetooth = false;
bool modoInventario = false;
bool erroEscritaEPC = false;
bool modoDescadastro = false;
bool excluirTID = false;
bool excluirEPC = false;
bool modoReset = false;
bool erroEscritaTID = false;
bool cadastroOk = false;

byte quantidadeInventariada = 0;
byte numeroQuantidade = 1;
byte controleQuantidadeCadastradas = 0;
byte numControleTeclado = 0;

int valorNumericoPN_SDCARD = 0;
int n = 0;
int mil = 1000;
int cem = 100;
int dez = 10;
int valorCent = 0;
int valorDez = 0;
int numberMil, numberCent, numberDez, numberUnid = 0;
int sendCmdInventary[] = {"0", 0x0, 0x0, 0x0};

long milhar, centena, dezena, unidade = 0;
long valorMilhar, valorCentena, valorDezena = 0;
long valorNumericoPN = 0;
long numeroPosicao = 111;

const byte qtdLinhas = 4; //QUANTIDADE DE LINHAS DO TECLADO
const byte qtdColunas = 4; //QUANTIDADE DE COLUNAS DO TECLADO

//CONSTRUÇÃO DA MATRIZ DE CARACTERES
char matriz_teclas[qtdLinhas][qtdColunas] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};


byte PinosqtdLinhas[qtdLinhas] = {30, 32, 34, 36}; //PINOS UTILIZADOS PELAS LINHAS
byte PinosqtdColunas[qtdColunas] = {40, 42, 44, 46}; //PINOS UTILIZADOS PELAS COLUNAS

RFID nano; //Create instance
SoftwareSerial softSerial(11, 10); //TX, RX ARDUINO MEGA 2560 --> RX, TX SHIELD RFID SRTR SPARKFUN
LiquidCrystal_I2C lcd(I2C_ADDRESS, En, Rw, Rs, D4, D5, D6, D7, BACKLIGHT_PIN, POSITIVE);
File myFile;
//INICIALIZAÇÃO DO TECLADO
Keypad meuteclado = Keypad( makeKeymap(matriz_teclas), PinosqtdLinhas, PinosqtdColunas, qtdLinhas, qtdColunas);

void setup() {

  pinMode(pinEstadoBluetoothRemota, INPUT_PULLUP);
  pinMode(pinEstadoBluetoothHost, INPUT_PULLUP);
  pinMode(ledRED, OUTPUT);
  pinMode(ledGREEN, OUTPUT);
  pinMode(ledBLUE, OUTPUT);

  lcd.begin(20, 4);
  Serial.begin(9600);
  Serial1.begin(38400); //BLUETOOTH SRTR-HOST
  Serial2.begin(9600); //BLUETOOTH SRTR-DOTR-900U SCANNER

  if (!SD.begin(SS)) {

    Serial.println("Initialization failed!");
    cartaoSDstatus = false;
    digitalWrite(ledRED, LOW);
    digitalWrite(ledGREEN, HIGH);
    digitalWrite(ledBLUE, HIGH);

  } else {

    cartaoSDstatus = true;
    Serial.println("Initialization done.");

  }

  pinMode(BUZZER1, OUTPUT);
  pinMode(BUZZER2, OUTPUT);

  digitalWrite(BUZZER2, LOW); //Pull half the buzzer to ground and drive the other half.

  while (!Serial);

  Serial.println("Initializing...");

  if (setupNano(38400) == false) //Configure nano to run at 38400bps
  {
    Serial.println("Falha ao inicializar o módulo Sparkfun RFID!");
    //while (1); //Freeze!
  }

  nano.setRegion(REGION_NORTHAMERICA); //Set to North America

  nano.setReadPower(500); //5.00 dBm. Higher values may cause USB port to brown out
  //Max Read TX Power is 27.00 dBm and may cause temperature-limit throttling

  lcd.setCursor(1, 0);
  lcd.print("Limpeza do Buffer");
  lcd.setCursor(1, 1);
  lcd.print("de");
  lcd.setCursor(1, 2);
  lcd.print("Inventario");
  lcd.setCursor(1, 3);
  lcd.print("Por favor, aguarde!");

  //for (int e = 111 ; e <= maxPosicao ; e++) {
  //EEPROM.write(e, 0);
  //}

  lcd.clear();

  lcd.setCursor(3, 0);
  lcd.print("SOGEFI GROUP");
  lcd.setCursor(3, 1);
  lcd.print("IIoT RFID");
  lcd.setCursor(3, 2);
  lcd.print("CARREGANDO...");
  for (int a = 0; a <= 19; a++) {
    lcd.setCursor(a, 3);
    lcd.write(byte(255));
    delay(50);
  }
  delay(500);
  lcd.clear();

  SD.remove(inventFile);

}

void loop() {

  if (cartaoSDstatus) setColorRGB(0, 255, 0);           //COR VERDE

  tecladoMatricial4x4();

  if (!modoDescadastro) {

    Serial.println("Aguardando instruções...");

    recebeDadosHost();
    botaoCancelar();
    botaoSelecaoCategoria();
    inventarioRFID();
    dataloggerInventario();
    condicoesControleCiclos();
    botaoOKselecao();
    menuSelecao();

  } else {

    apagarTIDeEPCdoCartaoSD();

  }

  delay(delayScan);

}

void condicoesControleCiclos() {

  if (numeroQuantidade > maxQuantidade) {

    lcd.clear();

    numeroQuantidade = 1;

  } else if (numeroQuantidade < 1) {

    lcd.clear();

    numeroQuantidade = maxQuantidade;

  }
}

void tecladoMatricial4x4() {

  tecla_pressionada = meuteclado.getKey();

  if (tecla_pressionada == 'A') {

    modoDescadastro = !modoDescadastro;

    if (!modoDescadastro) cancelaLeituraTag = true;

    lcd.clear();

    if (modoDescadastro) {

      modoReset = true;

      lcd.setCursor(3, 1);
      lcd.print("RESET DE TAGS");
      lcd.setCursor(5, 2);
      lcd.print("ATIVADO!");
      delay(delayLCDcadastro);

    } else {

      modoReset = false;

      lcd.setCursor(3, 1);
      lcd.print("RESET DE TAGS");
      lcd.setCursor(4, 2);
      lcd.print("DESATIVADO!");
      delay(delayLCDcadastro);

    }

    Serial.println("BOTÃO RESET DE TAGS");

    lcd.clear();

  }

  if (!modoDescadastro) {

    if (tecla_pressionada == 'C') {

      if (!quantidadeProdCadastrar) {
        numeroPosicao++;
      } else {
        numeroQuantidade++;
      }


      if (numeroPosicao > maxPosicao) {

        lcd.clear();
        numeroPosicao = 111;

      } else if (numeroPosicao <= 110) {

        lcd.clear();
        numeroPosicao = maxPosicao;

      }

    } else if (tecla_pressionada == 'D') {

      if (!quantidadeProdCadastrar) {
        numeroPosicao--;
      } else {
        numeroQuantidade--;
      }

      if (numeroPosicao > maxPosicao) {

        lcd.clear();
        numeroPosicao = 111;

      } else if (numeroPosicao <= 110) {

        lcd.clear();
        numeroPosicao = maxPosicao;

      }
    }

    if (tecla_pressionada == '1' || tecla_pressionada == '2' || tecla_pressionada == '3' || tecla_pressionada == '4' || tecla_pressionada == '5' || tecla_pressionada == '6' || tecla_pressionada == '7' || tecla_pressionada == '8' || tecla_pressionada == '9' || tecla_pressionada == '0') {

      if (numControleTeclado > 3) {
        numControleTeclado = 0;

        numeroTeclado = "";
        milKey = "";
        centKey = "";
        dezKey = "";
        uniKey = "";
        numControleTeclado = 0;

      }

      numControleTeclado++;
      Serial.println(numControleTeclado);

      if (numControleTeclado == 1) milKey = tecla_pressionada;
      if (numControleTeclado == 2) centKey = tecla_pressionada;
      if (numControleTeclado == 3) dezKey = tecla_pressionada;
      if (numControleTeclado == 4) uniKey = tecla_pressionada;

      numeroTeclado = milKey + centKey + dezKey + uniKey;
      Serial.println(numeroTeclado);

      if (numeroTeclado.toInt() >= 0 && numeroTeclado.toInt() <= maxPosicao) {

        if (!quantidadeProdCadastrar) {

          lcd.clear();

          numeroPosicao = numeroTeclado.toInt();

        }

      } else {

        lcd.clear();

        //numeroTeclado = "";
        //milKey = "";
        //centKey = "";
        //dezKey = "";
        //uniKey = "";
        //numControleTeclado = 0;

      }

      if (numeroTeclado.toInt() > 0 && numeroTeclado.toInt() <= maxQuantidade) {

        if (quantidadeProdCadastrar) {

          lcd.clear();

          numeroQuantidade = numeroTeclado.toInt();

        }

      } else {

        lcd.clear();

        //numeroTeclado = "";
        //milKey = "";
        //centKey = "";
        //dezKey = "";
        //uniKey = "";
        //numControleTeclado = 0;

      }
    }
  }
}

void botaoOKselecao() {

  if (tecla_pressionada == '#')  {

    erroEscritaTID = false;
    erroEscritaEPC = false;
    comparadorEPC = false;
    cadastroOk = false;

    if (numeroPosicao > 110) {
      n++;
      lcd.clear();
      quantidadeProdCadastrar = !quantidadeProdCadastrar; //Estado inicial em FALSE;

      numeroTeclado = "";
      milKey = "";
      centKey = "";
      dezKey = "";
      uniKey = "";
      numControleTeclado = 0;

      Serial.println("BOTAO OK PRESSIONADO");
    }

    lcd.clear();

  }
}

void botaoCancelar() {

  if (tecla_pressionada == '*') {

    cancelaLeituraTag = true;
    quantidadeProdCadastrar = false;


    numeroTeclado = "";
    milKey = "";
    centKey = "";
    dezKey = "";
    uniKey = "";
    numControleTeclado = 0;

    numeroQuantidade = 1;

    //Serial2.print("I, 0, 0, 0");

    Serial.println("BOTAO CANCELAR PRESSIONADO");

    lcd.clear();

  }
}

void botaoSelecaoCategoria() {

  if (tecla_pressionada == 'B') {

    modoInventario = !modoInventario;

    if (modoInventario) {

      for (int e = 111 ; e <= maxPosicao ; e++) {
        EEPROM.write(e, 0);

      }

      lcd.setCursor(2, 1);
      lcd.print("MODO INVENTARIO");
      lcd.setCursor(6, 2);
      lcd.print("ATIVADO!");
      //Serial2.print("I, 0, 0, 0");
      delay(delayLCDcadastro);

    } else {

      lcd.setCursor(2, 1);
      lcd.print("MODO INVENTARIO");
      lcd.setCursor(5, 2);
      lcd.print("DESATIVADO!");
      delay(delayLCDcadastro);

    }

    Serial.println("BOTÃO SELEÇÃO CATEGORIA");

    lcd.clear();

  }
}

void menuSelecao() {

  //************************************************************************************************************************************************

  if (n == 2 && !cancelaLeituraTag) {

    if (!quantidadeProdCadastrar) {

      while (controleQuantidadeCadastradas > 0 && !comparadorEPC) {

        leituraTID();

        n = 0;
        numeroQuantidade = 1;

        botaoOKselecao();

        if (comparadorEPC && !cancelaLeituraTag) {

          while (comparadorEPC) {

            tecladoMatricial4x4();
            botaoOKselecao();

            Serial.println("ERRO TAG JÁ CADASTRADA!");

            lcd.setCursor(7, 0);
            lcd.print("ERRO!");
            lcd.setCursor(1, 1);
            lcd.print("TAG JA CADASTRADA!");
            lcd.setCursor(1, 2);
            lcd.print("ESCOLHA OUTRA TAG");
            lcd.setCursor(5, 3);
            lcd.print("Pressione #");

          }

          conteudo = "";
          lcd.clear();

        } else if (!comparadorEPC && !cancelaLeituraTag) {

          WRITE_TID_SD_CARD();

          calculoMatematico();

          byte hexEPC[] = {numberMil, numberCent, numberDez, numberUnid}; //VALOR QUE SERÁ ESCRITO NA MEMÓRIA EPC "EPC MEMORY"

          //Serial.println(numberMil);
          //Serial.println(numberCent);
          //Serial.println(numberDez);
          //Serial.println(numberUnid);
          Serial.println("Escrevendo EPC na TAG...");

          byte responseType = nano.writeTagEPC(hexEPC, sizeof(hexEPC)); //The -1 shaves off the \0 found at the end of string

          delay(delayEscritaEPC);

          leituraEPC();

          WRITE_EPC_SD_CARD();

          if (!erroEscritaEPC) {

            sendStringToHost();

            cadastroOk = true;

            while (cadastroOk) {

              tecladoMatricial4x4();
              botaoOKselecao();

              lcd.setCursor(1, 0);
              lcd.print("EPC: ");
              lcd.setCursor(6, 0);
              lcd.print(conteudoEPCSD);
              lcd.setCursor(1, 2);
              lcd.print("PRODUTO CADASTRADO!");
              //lcd.setCursor(4, 3);
              //lcd.print("COM SUCESSO!");
              lcd.setCursor(5, 3);
              lcd.print("Pressione #");

            }

            lcd.clear();

          } else {

            while (erroEscritaEPC) {

              tecladoMatricial4x4();
              botaoOKselecao();

              lcd.setCursor(1, 0);
              lcd.print("FALHA DE CADASTRO!");
              lcd.setCursor(1, 1);
              lcd.print("Recarregar bateria");
              lcd.setCursor(1, 2);
              lcd.print("Verificar micro SD");
              lcd.setCursor(5, 3);
              lcd.print("Pressione #");

            }

            lcd.clear();

          }

          conteudo = "";
          conteudoEPC = "";
          erroEscritaEPC = false;

        }
      }

      comparadorEPC = false;

    }
  }

  if (n == 1 && !cancelaLeituraTag) {

    //estadoBluetoothPareadoHOST = true;

    estadoBluetoothPareadoHOST = digitalRead(pinEstadoBluetoothHost);

    if (estadoBluetoothPareadoHOST) {

      erroBluetooth = false;

      lcd.setCursor(1, 0);
      lcd.print("QUANTOS PRODUTOS");
      lcd.setCursor(1, 1);
      lcd.print("DESEJA CADASTRAR?");
      lcd.setCursor(8, 3);
      lcd.print(numeroQuantidade);

      if (numeroPosicao <= limiteEL) {

        lcd.setCursor(14, 3);
        lcd.print(partNumber);
        lcd.setCursor(16, 3);
        lcd.print(numeroPosicao);

      } else {

        lcd.setCursor(14, 3);
        lcd.print(partNumbermMec);
        lcd.setCursor(16, 3);
        lcd.print(numeroPosicao);

      }

      if (numeroQuantidade > maxQuantidade) {
        numeroQuantidade = 1;
        lcd.clear();
      } else if (numeroQuantidade < 1) {
        numeroQuantidade = maxQuantidade;
        lcd.clear();
      }

      controleQuantidadeCadastradas = numeroQuantidade;

      botaoCancelar();

    } else {

      if (!erroBluetooth) {

        erroBluetooth = true;
        lcd.clear();

      }

      n = -1;
      numeroQuantidade = 1;

      lcd.setCursor(1, 1);
      lcd.print("POR FAVOR AGUARDE");
      lcd.setCursor(4, 2);
      lcd.print("CONECTANDO...");

    }
  }

  if (!quantidadeProdCadastrar && !inventarioAtivado && !modoDescadastro) {

    n = 0;
    cancelaLeituraTag = false;

    lcd.setCursor(1, 0);
    lcd.print("SELECIONE O");
    lcd.setCursor(1, 1);
    lcd.print("PART NUMBER");

    if (modoInventario) {
      lcd.setCursor(14, 3);
      lcd.write('I');
    }

    if (numeroPosicao <= limiteEL) {

      lcd.setCursor(4, 3);
      lcd.print(partNumber);
      lcd.setCursor(6, 3);
      lcd.print(numeroPosicao);

    } else {

      lcd.setCursor(4, 3);
      lcd.print(partNumbermMec);
      lcd.setCursor(6, 3);
      lcd.print(numeroPosicao);

    }

    lcd.setCursor(13, 0);
    lcd.write(byte(255));
    lcd.setCursor(13, 1);
    lcd.write(byte(255));
    lcd.setCursor(13, 2);
    lcd.write(byte(255));
    lcd.setCursor(13, 3);
    lcd.write(byte(255));

    lcd.setCursor(15, 0);
    lcd.print("QUANT");
    lcd.setCursor(15, 1);
    lcd.print("ATUAL");

    lcd.setCursor(17, 3);
    lcd.print(EEPROM.read(numeroPosicao));

  } else if (!quantidadeProdCadastrar && inventarioAtivado && !modoDescadastro) {

    lcd.setCursor(3, 0);
    lcd.print("RECEBENDO DADOS");
    lcd.setCursor(3, 1);
    lcd.print("DO SCANNER...");
    lcd.setCursor(3, 2);
    lcd.print("Inventario em");
    lcd.setCursor(5, 3);
    lcd.print("andamento!");

  }

  //**************************************************************************************************************************************************
}

void sendStringToHost() {

  setColorRGB(0, 0, 255);           //COR VERDE

  Serial1.println(conteudoEPCSD);  //NOME DO ARQUIVO

  for (int n = 1; n <= 1; n++) {


    if (n == 1) Serial1.println(numeroPosicao);       //N° DO PRODUTO

    delay(delayScan);

  }
}

void recebeDadosHost() {

  if (Serial1.available()) {

    pacoteDadosCadastrador = Serial1.readStringUntil('\n');
    Serial.println(pacoteDadosCadastrador);

    lcd.setCursor(0, 3);
    lcd.write('.');
    lcd.setCursor(1, 3);
    lcd.write('.');

    if (pacoteDadosCadastrador.length() <= 8) {

      numeroProdutoCadastrador = pacoteDadosCadastrador.substring(0, pacoteDadosCadastrador.indexOf(','));
      quantidadeProdutoCadastrador = pacoteDadosCadastrador.substring(pacoteDadosCadastrador.indexOf(',') + 1, pacoteDadosCadastrador.length());

      if (numeroProdutoCadastrador.length() <= 4 && quantidadeProdutoCadastrador.toInt() <= maxQuantidade) {

        EEPROM.write(numeroProdutoCadastrador.toInt(), quantidadeProdutoCadastrador.toInt());

        Serial.println(numeroProdutoCadastrador);
        Serial.println(quantidadeProdutoCadastrador);

      }
    }
  }
}

void WRITE_TID_SD_CARD() {

  myFile = SD.open(String(conteudo) + ".txt", FILE_WRITE);

  if (myFile) {

    Serial.print("Creating TID file .txt");
    myFile.print(numeroPosicao);

    myFile.close();
    Serial.println(" Done!");

  } else {

    Serial.println("error creating TID file");

    erroEscritaTID = true;

    while (erroEscritaTID) {

      tecladoMatricial4x4();
      botaoOKselecao();

      lcd.setCursor(1, 0);
      lcd.print("FALHA DE CADASTRO!");
      lcd.setCursor(8, 1);
      lcd.print("TID");
      lcd.setCursor(1, 2);
      lcd.print("Verificar micro SD");
      lcd.setCursor(5, 3);
      lcd.print("Pressione #");

    }

    lcd.clear();

  }
}

void leituraTID() {

sair:

  controleQuantidadeCadastradas--;

  byte myTID[4]; //TIDs are 20 bytes
  byte tidLength;
  byte responseType = 0;

  if (!cancelaLeituraTag) {

    while (responseType != RESPONSE_SUCCESS)//RESPONSE_IS_TAGFOUND)
    {

      tecladoMatricial4x4();
      botaoCancelar();

      if (cancelaLeituraTag) goto sair;

      tidLength = sizeof(myTID); //Length of TID is modified each time .readTID is called

      responseType = nano.readTID(myTID, tidLength, 500); //Scan for a new tag up to 500ms

      Serial.println("Searching for TID tag...");

      if (!modoDescadastro) {

        lcd.setCursor(2, 0);
        lcd.print("POSICIONE A TAG");
        lcd.setCursor(2, 1);
        lcd.print("SOBRE O LEITOR...");

        if (numeroPosicao <= limiteEL) {

          lcd.setCursor(6, 3);
          lcd.print(partNumber);
          lcd.setCursor(8, 3);
          lcd.print(numeroPosicao);

        } else {

          lcd.setCursor(6, 3);
          lcd.print(partNumbermMec);
          lcd.setCursor(8, 3);
          lcd.print(numeroPosicao);

        }

      } else {

        lcd.setCursor(3, 0);
        lcd.print("POSICIONE A TAG");
        lcd.setCursor(3, 1);
        lcd.print("SOBRE O LEITOR");
        lcd.setCursor(3, 2);
        lcd.print("PARA EFETUAR O");
        lcd.setCursor(6, 3);
        lcd.print("RESET...");

      }
    }

    //Beep! Piano keys to frequencies: http://www.sengpielaudio.com/KeyboardAndFrequencies.gif
    tone(BUZZER1, 2093, 150); //C
    delay(150);
    tone(BUZZER1, 2349, 150); //D
    delay(150);
    tone(BUZZER1, 2637, 150); //E
    delay(150);

    for (byte x = 0 ; x < tidLength  ; x++) {

      conteudo.concat(String(myTID[x], HEX));

    }

    Serial.println(conteudo);

    lcd.clear();

    if (!modoDescadastro) {

      listaComparacaoTAGS();

    } else {

      excluirArquivoTID();

    }
  }
}

void listaComparacaoTAGS() {

  if (SD.exists(conteudo + ".txt")) {

    comparadorEPC = true;
    Serial.println("Exists.");

  } else {

    comparadorEPC = false;
    Serial.println("Doesn't exist.");

  }

}

void calculoMatematico() {

  numberMil = numeroPosicao / mil;                      //  MILHAR
  valorCent = numeroPosicao - (numberMil * mil);

  numberCent = valorCent / cem;                 //  CENTENA
  valorDez = valorCent - (numberCent * cem);

  numberDez = valorDez / dez;                   //  DEZENA

  numberUnid = valorDez - (numberDez * dez);    //  UNIDADE

}

//Gracefully handles a reader that is already configured and already reading continuously
//Because Stream does not have a .begin() we have to do this outside the library
boolean setupNano(long baudRate) {
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

    //Serial.println(F("Module continuously reading. Asking it to stop..."));

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

void inventarioRFID() {

  if (Serial2.available() && modoInventario) {

    conteudoSCANNER = Serial2.readStringUntil('\n');

    /*Serial.print("CONTEUDO STRING RECEBIDA: ");
      Serial.println(conteudoSCANNER);
      Serial.println(conteudoSCANNER.length());
      Serial.println();*/

    if (conteudoSCANNER.length() == 33) {

      inventarioAtivado = true;

      conteudoSCANNERpartNumber = conteudoSCANNER.substring(4, 12);
      conteudoSCANNERlastNumber = conteudoSCANNER.substring(24, 28);

      conteudoSCANNERfiltrado = conteudoSCANNERpartNumber + conteudoSCANNERlastNumber;

      /*Serial.print("CONTEUDO FILTRADO: ");
        Serial.println(conteudoSCANNERfiltrado);
        Serial.print("CONTEUDO PART NUMBER: ");
        Serial.println(conteudoSCANNERpartNumber);
        Serial.print("CONTEUDO PART NUMBER TRATADO: ");
        Serial.println(conteudoSCANNERpartNumber.toInt());
        Serial.print("CONTEUDO LAST NUMBER: ");
        Serial.println(conteudoSCANNERlastNumber);
        Serial.println();*/

      calculoMatematicoObterPN();

      conteudoVALIDO = String(valorNumericoPN) + conteudoSCANNERlastNumber + ".txt";

      Serial.println(conteudoVALIDO);

      //Serial.print("FILE NAME EPC: ");
      //Serial.println(conteudoVALIDO);

      contagemInventario();

    }

    conteudoSCANNER = "";

  }
}

void dataloggerInventario() {

  estadoBluetoothPareadoREMOTA = digitalRead(pinEstadoBluetoothRemota);

  //Serial.print("ESTADO PAREAMENTO BLUETOOTH: ");
  //Serial.println(estadoBluetoothPareadoREMOTA);

  if (inventarioAtivado && !estadoBluetoothPareadoREMOTA) {

    lcd.clear();

    myFile = SD.open(inventFile, FILE_WRITE);

    if (myFile) {

      for (int d = 1; d <= maxPosicao; d++) {

        Serial.println("Creating inventary log File");

        conteudoLogInventario = String(d) + ',' + String(EEPROM.read(d));

        myFile.println(conteudoLogInventario);

        lcd.setCursor(2, 0);
        lcd.print("GERANDO ARQUIVO");
        lcd.setCursor(1, 1);
        lcd.print("DE INVENTARIO BAAN");
        lcd.setCursor(0, 2);
        lcd.print("Por favor aguarde...");
        lcd.setCursor(8, 3);
        lcd.print(d);

      }

      myFile.close();

      Serial.println(" Done!");

      inventarioAtivado = false;

      lcd.clear();

    } else {

      Serial.println("error creating inventary log file");

    }
  }
}

void calculoMatematicoObterPN() {

  milhar = (conteudoSCANNERpartNumber.toInt() / 1000000);
  //Serial.print("MILHAR = ");
  //Serial.println(milhar);

  valorMilhar = (conteudoSCANNERpartNumber.toInt() - (milhar * 1000000));
  centena = valorMilhar / 10000;
  //Serial.print("CENTENA = ");
  //Serial.println(centena);

  valorCentena = (valorMilhar - (centena * 10000));
  dezena = valorCentena / 100;
  //Serial.print("DEZENA = ");
  //Serial.println(dezena);

  valorDezena = (valorCentena - (dezena * 100));
  unidade = valorDezena;
  //Serial.print("UNIDADE = ");
  //Serial.println(unidade);

  valorNumericoPN = (milhar * 1000) + (centena * 100) + (dezena * 10) + unidade;
  //Serial.print("#PN = ");
  //Serial.println(valorNumericoPN);

}

void contagemInventario() {

  if (SD.exists(conteudoVALIDO)) {

    Serial.print(conteudoVALIDO);
    Serial.println(" EPC FILE exists!");

    quantidadeInventariada = EEPROM.read(valorNumericoPN);
    quantidadeInventariada++;

    Serial.print("QUANTIDADE INVENTARIADA = ");
    Serial.println(quantidadeInventariada);

    EEPROM.write(valorNumericoPN, quantidadeInventariada);

    SD.remove(conteudoVALIDO);
    Serial.println("Excluíndo o arquivo...");

  } else {

    Serial.println("EPC FILE doesn't exist!");

  }
}

void leituraEPC() {

  byte myEPC[12]; //Most EPCs are 12 bytes
  byte myEPClength;
  byte responseTypeEPC = 0;

  while (responseTypeEPC != RESPONSE_SUCCESS)//RESPONSE_IS_TAGFOUND)
  {
    myEPClength = sizeof(myEPC); //Length of EPC is modified each time .readTagEPC is called

    responseTypeEPC = nano.readTagEPC(myEPC, myEPClength, 500); //Scan for a new tag up to 500ms
    Serial.println("Searching for EPC tag...");

  }

  for (byte y = 0 ; y < myEPClength ; y++) {

    if (y < 4) {

      conteudoEPC.concat(String(myEPC[y], HEX));

    } else {

      if (myEPC[y] < 0x10) conteudoEPC = conteudoEPC + "0";
      conteudoEPC.concat(String(myEPC[y], HEX));

    }
  }

  Serial.println(conteudoEPC);

  partNumberEPC = conteudoEPC.substring(0, 4);
  lastNumberEPC = conteudoEPC.substring(16, 20);

  valorNumericoPN_SDCARD = partNumberEPC.toInt();

  conteudoEPCSD = String(valorNumericoPN_SDCARD) + lastNumberEPC;

  Serial.print("#PN EPC: ");
  Serial.println(partNumberEPC);

  Serial.print("FINAL EPC: ");
  Serial.println(lastNumberEPC);

  if (modoDescadastro) {

    excluirArquivoEPC();

  }

}

void WRITE_EPC_SD_CARD() {

  myFile = SD.open(conteudoEPCSD + ".txt", FILE_WRITE);

  if (myFile) {

    Serial.print("Creating EPC file .txt");
    myFile.print(partNumberEPC);

    myFile.close();
    Serial.println(" Done!");

  } else {

    erroEscritaEPC = true;

    Serial.println("error creating EPC file");

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

void apagarTIDeEPCdoCartaoSD() {

  leituraTID();

  if (excluirTID && excluirEPC) {

    lcd.setCursor(4, 0);
    lcd.print("TAG RESETADA");
    lcd.setCursor(8, 1);
    lcd.print("COM");
    lcd.setCursor(6, 2);
    lcd.print("SUCESSO!");
    lcd.setCursor(6, 3);
    lcd.print("TID/EPC");
    delay(delayLCDcadastro);
    lcd.clear();

  } else if (!excluirTID && excluirEPC) {

    lcd.setCursor(4, 0);
    lcd.print("TAG RESETADA");
    lcd.setCursor(8, 1);
    lcd.print("COM");
    lcd.setCursor(6, 2);
    lcd.print("SUCESSO!");
    lcd.setCursor(8, 3);
    lcd.print("EPC");
    delay(delayLCDcadastro);
    lcd.clear();

  } else if (excluirTID && !excluirEPC) {

    lcd.setCursor(4, 0);
    lcd.print("TAG RESETADA");
    lcd.setCursor(8, 1);
    lcd.print("COM");
    lcd.setCursor(6, 2);
    lcd.print("SUCESSO!");
    lcd.setCursor(8, 3);
    lcd.print("TID");
    delay(delayLCDcadastro);
    lcd.clear();

  } else if (!excluirTID && !excluirEPC && modoReset) {

    lcd.setCursor(2, 1);
    lcd.print("TAG JA RESETADA!");
    lcd.setCursor(5, 2);
    lcd.print("Por favor");
    lcd.setCursor(1, 2);
    lcd.print("Escolha outra TAG");
    delay(delayLCDcadastro);
    lcd.clear();

  }

  excluirTID = false;
  excluirEPC = false;

}

void excluirArquivoTID() {

  if (SD.exists(conteudo + ".txt")) {

    Serial.print("Arquivo TID ");
    Serial.print(conteudo);
    Serial.println(" .txt excluído com sucesso!");

    SD.remove(conteudo + ".txt");

    excluirTID = true;

  } else {

    Serial.print("Arquivo TID ");
    Serial.print(conteudo);
    Serial.println(" .txt não encontrado!");

    excluirTID = false;

  }

  conteudo = "";

  leituraEPC();

}

void excluirArquivoEPC() {

  if (SD.exists(conteudoEPCSD + ".txt")) {

    Serial.print("Arquivo EPC ");
    Serial.print(conteudoEPCSD);
    Serial.println(" .txt excluído com sucesso!");

    SD.remove(conteudoEPCSD + ".txt");

    excluirEPC = true;

  } else {

    Serial.print("Arquivo EPC ");
    Serial.print(conteudoEPCSD);
    Serial.println(" .txt não encontrado!");

    excluirEPC = false;

  }

  conteudoEPC = "";
  conteudoEPCSD = "";

}
