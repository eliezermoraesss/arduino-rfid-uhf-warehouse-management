/*
   REV. 2 ULTIMA ATUALIZACAO 07/12/2018
   PROGRAMADOR E DESENVOLVEDOR: ELIEZER MORAES SILVA
*/

//******************* REMOTA ALMOXARIFADO *******************

#define delayScan 500
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
bool resetUpdate = false;
bool stopAtualizacao = false;
bool stopExcel = false;

int valorCorrecaoQuantidade = 0;
int posicaoRegistro = 110;
int posicaoLinhaExcel = 1;
int Reset = 0;
int dadosRecebidos = 110;
uint32_t tempoPausaEmail = 86400000; //24 horas

uint32_t delayEnvioEmail = 0;

void setup() {

  Serial.begin(38400);
  Serial1.begin(38400, SERIAL_8N1, 4, 2);   // INTERFACE DE COMUNICAÇÃO UART BLUETOOTH

  pinMode(ledRED, OUTPUT);
  pinMode(ledGREEN, OUTPUT);
  pinMode(ledBLUE, OUTPUT);
  pinMode(pinBotaoRegQtd, INPUT_PULLUP);

  digitalWrite(ledRED, 0);
  digitalWrite(ledGREEN, 100);
  digitalWrite(ledBLUE, 100);

  delay(30000); //Espera 30 segundos para garantir o pareamento completo com a HOST

}

void loop() {

  botaoRegistro();

  if (stopExcel) {

    if ((millis() - delayEnvioEmail) > tempoPausaEmail) {

      stopExcel = false;
      dadosRecebidos = 110;

    }

  }

  if (modoRegistro) {

    setColorRGB(150, 0, 150);
    atualizacaoHOSTparaREMOTA();

  } else if (!modoRegistro) {

    setColorRGB(150, 150, 0);
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
        delayEnvioEmail = millis();

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
