/*
   REV. 3 ULTIMA ATUALIZACAO 10/01/2019
   PROGRAMADOR E DESENVOLVEDOR: ELIEZER MORAES SILVA
*/

// ****************************************************************
// ******************* REMOTA RFID ALMOXARIFADO *******************
// ****************************************************************

// INTERVALO DE CÓDIGOS DO EL0111 AO EL1000

#define delayScan 100
#define delayPareamento 30000 // 30 segundos
#define quantidadeProdutos 1000
#define ledRED 27
#define ledGREEN 25
#define ledBLUE 33
#define pinBotaoRegQtd 32

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
String comandoGETCelulaQuantidadeExcel = "CELL,GET,O";
String comandoGETCelulaADDExcel = "CELL,GET,P";
String comandoSETCelulaQuantidadeExcelEspelho = "CELL,SET,Q";

bool modoRegistrar = true;
bool modoRegistarAnterior = true;
bool modoRegistro = true;
bool stopExcel = false;

int valorCorrecaoQuantidade = 0;
int posicaoRegistro = 110;
int posicaoLinhaExcel = 1;
int Reset = 0;
int dadosRecebidos = 110;

void setup() {

  Serial.begin(38400);
  Serial1.begin(38400, SERIAL_8N1, 4, 2);   // INTERFACE DE COMUNICAÇÃO UART BLUETOOTH

  pinMode(ledRED, OUTPUT);
  pinMode(ledGREEN, OUTPUT);
  pinMode(ledBLUE, OUTPUT);
  pinMode(pinBotaoRegQtd, INPUT_PULLUP);

  digitalWrite(ledRED, 0); // LED INDICADOR VERMELHO
  digitalWrite(ledGREEN, 150);
  digitalWrite(ledBLUE, 150);

  delay(delayPareamento); //Espera 30 segundos para garantir o pareamento completo com a HOST

}

void loop() {

  botaoRegistro();

  if (modoRegistro) {

    setColorRGB(150, 0, 150); // LED INDICADOR VERDE
    atualizacaoHOSTparaREMOTA();

  } else if (!modoRegistro) {

    setColorRGB(150, 150, 0); // LED INDICADOR AZUL
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
        dadosRecebidos = 110;

      } else if (stopExcel && dadosRecebidos == quantidadeProdutos) {

        stopExcel = false;
        dadosRecebidos = 110;

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
    posicaoRegistro = 110;
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

    posicaoRegistro = 110;
    posicaoLinhaExcel = 1;
    modoRegistro = true;

  }
}

void setColorRGB(int vermelho, int verde, int azul) {

#ifdef COMMON_ANODE
  vermelho = 255 - vermelho;
  verde = 255 - verde;
  azul = 255 - azul;
#endif

  digitalWrite(ledRED, vermelho);
  digitalWrite(ledGREEN, verde);
  digitalWrite(ledBLUE, azul);

}
