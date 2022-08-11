/*
   REV. 1 ULTIMA ATUALIZACAO 31/10/2018
   PROGRAMADOR E DESENVOLVEDOR: ELIEZER MORAES SILVA
*/

//******************* REMOTA *******************

#define delayScan 100
#define quantidadeProdutos 150
#define delayEmail 
#define ledRED 25
#define ledGREEN 26
#define ledBLUE 27
#define pinBotaoRegQtd 32

void atualizacaoHOSTparaREMOTA();
void registrarQuantidades();
void atualizaExcel();
void botaoRegistro();

String conteudo = "";
String codigoPartNumber = "";
String charPartNumber = "";
String quantity = "";
String comandoSETCelulaQuantidadeExcel = "CELL,SET,O";
String comandoSETCelulaADDExcel = "CELL,SET,P";
String comandoGETCelulaQuantidadeExcel = "CELL,GET,O";
String comandoGETCelulaADDExcel = "CELL,GET,P";

bool modoRegistrar = true;
bool modoRegistarAnterior = true;
bool modoRegistro = true;
bool resetUpdate = false;
bool stopAtualizacao = false;
bool stopExcel = false;

int valorCorrecaoQuantidade = 0;
int posicaoRegistro = 0;
int posicaoLinhaExcel = 1;
int Reset = 0;

void setup() {

  Serial.begin(38400);
  Serial1.begin(38400, SERIAL_8N1, 4, 2);   // INTERFACE DE COMUNICAÇÃO UART BLUETOOTH

  pinMode(ledRED, OUTPUT);
  pinMode(ledGREEN, OUTPUT);
  pinMode(ledBLUE, OUTPUT);
  pinMode(pinBotaoRegQtd, INPUT_PULLUP);

}

void loop() {

  botaoRegistro();

  if (modoRegistro) {

    atualizacaoHOSTparaREMOTA();

  } else if (!modoRegistro) {

    registrarQuantidades();

  }

  delay(delayScan);

}

void atualizacaoHOSTparaREMOTA() {

  if (Serial1.available()) {

    conteudo = Serial1.readStringUntil('\n');

    if (conteudo.length() <= 9) {

      charPartNumber = conteudo.indexOf(',');
      codigoPartNumber = conteudo.substring(0, charPartNumber.toInt());
      quantity = conteudo.substring(charPartNumber.toInt() + 1, conteudo.length());

      /*Serial.print("STRING RECEBIDA: ");
      Serial.println(conteudo);
      Serial.print("PART NUMBER: ");
      Serial.println(codigoPartNumber);
      Serial.print("QUANTIDADE: ");
      Serial.println(quantity);*/

      atualizaExcel();

    }
  }
}

void atualizaExcel() {

  codigoPartNumber = codigoPartNumber.toInt() + 1;

  String setQuantidadeCelula = comandoSETCelulaQuantidadeExcel + String(codigoPartNumber) + ',' + quantity.toInt();
  
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

  Serial.println(comandoGETCelulaQuantidadeExcel + String(posicaoLinhaExcel));  //COLUNA O
  int TEMPAnt = Serial.parseInt();
  Serial.println(comandoGETCelulaADDExcel + String(posicaoLinhaExcel));         //COLUNA P
  int TEMP = Serial.parseInt();

  valorCorrecaoQuantidade = TEMPAnt + TEMP;

  String setQuantidadeCelula = comandoSETCelulaQuantidadeExcel + String(posicaoLinhaExcel) + ',' + String(valorCorrecaoQuantidade);
  Serial.println(setQuantidadeCelula);

  String setReset = comandoSETCelulaADDExcel + String(posicaoLinhaExcel) + ',' + String(Reset);
  Serial.println(setReset);

  Serial1.println(String(posicaoRegistro) + ',' + String(valorCorrecaoQuantidade));

  if (posicaoRegistro == quantidadeProdutos) {

    posicaoRegistro = 0;
    posicaoLinhaExcel = 1;
    modoRegistro = true;

  }
}
