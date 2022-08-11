/*
  REV. 02 ULTIMA ATUALIZACAO 01/04/2019
  PROGRAMADOR E DESENVOLVEDOR: ELIEZER MORAES SILVA
*/

// ************************************************************************************************************
// ******************* ANTENA LEITORA RFID UHF TECNOLOGIA TNC MANUTENÇÃO ESPELHO/REPETIDORA *******************
// ************************************************************************************************************

// INTERVALO DE CÓDIGOS DO EL0001 AO EL0100

#include <SoftwareSerial.h> //Used for transmitting to the device
#include <SD.h>
#include <DS3231.h>

SoftwareSerial softSerial(11, 10); //RX, TX

#include "SparkFun_UHF_RFID_Reader.h" //Library for controlling the M6E Nano module
RFID nano; //Create instance

File myFile;
DS3231 RTC(SDA, SCL);

#define delayScan 150
#define ledRED 26
#define ledGREEN 24
#define ledBLUE 22
#define COMMON_ANODE
#define pinEstadoBluetoothHost 28
#define SS 53

bool estadoBluetoothPareadoHOST = false;

byte tagEPCBytes = 0;
int temperatura = 0;

String conteudo = "";
String partNumber = "";
String dadosDatalogger = "";
String data = "";
String hora = "";
String sinal = "-";
String fileNameDataLogger = "movEspMT.txt";

void leituraTAGS();
void setColorRGB();

void setup()
{
  Serial.begin(115200);
  Serial1.begin(38400);   //INTERFACE DE COMUNICAÇÃO UART BLUETOOTH ESPELHO_HOST
  RTC.begin();

  pinMode(ledRED, OUTPUT);
  pinMode(ledGREEN, OUTPUT);
  pinMode(ledBLUE, OUTPUT);

  if (!SD.begin(SS)) {

    while (1) {

      Serial.println("Initialization failed!");

      //FALHA CARTÃO MICRO SD NÃO IDENTIFICADO
      digitalWrite(ledRED, 1);
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
    digitalWrite(ledGREEN, 1);
    digitalWrite(ledBLUE, 0);

  }

  if (setupNano(38400) == false) //Configure nano to run at 38400bps
  {
    Serial.println(F("Module failed to respond. Please check wiring."));
    while (1) { //Freeze!
      digitalWrite(ledRED, 1);
      digitalWrite(ledGREEN, 0);
      digitalWrite(ledBLUE, 0);
      delay(80);
      digitalWrite(ledRED, 0);
      digitalWrite(ledGREEN, 0);
      digitalWrite(ledBLUE, 0);
      delay(200);
    }
  }

  nano.setRegion(REGION_NORTHAMERICA); //Set to North America

  nano.setReadPower(2600); //5.00 dBm. Higher values may caues USB port to brown out
  //Max Read TX Power is 27.00 dBm and may cause temperature-limit throttling

  nano.startReading(); //Begin scanning for tags

}

void loop() {

  estadoBluetoothPareadoHOST = digitalRead(pinEstadoBluetoothHost);

  while (!estadoBluetoothPareadoHOST) {

    setColorRGB(0xFF, 0xA5, 0x00); // LARANJA
    delay(200);

    setColorRGB(0, 0, 0);
    delay(100);

    estadoBluetoothPareadoHOST = digitalRead(pinEstadoBluetoothHost);

  }

  setColorRGB(0, 255, 0); // VERDE

  leituraTAGS();
  //dadosRTC();

  delay(delayScan);

}

void leituraTAGS() {

  if (nano.check() == true) { //Check to see if any new data has come in from module

    byte responseType = nano.parseResponse(); //Break response into tag ID, RSSI, frequency, and timestamp

    if (responseType == RESPONSE_IS_KEEPALIVE) {

      Serial.println("Leitura em andamento...");

    } else if (responseType == RESPONSE_IS_TAGFOUND) {

      setColorRGB(0, 0, 255); //COR AZUL
      Serial.println("DADOS RECEBIDOS");

      //If we have a full record we can pull out the fun bits
      //int rssi = nano.getTagRSSI(); //Get the RSSI for this tag read

      //long freq = nano.getTagFreq(); //Get the frequency this tag was detected at

      //long timeStamp = nano.getTagTimestamp(); //Get the time this was read, (ms) since last keep-alive message*/

      tagEPCBytes = nano.getTagEPCBytes(); //Get the number of bytes of EPC from response

      /*Serial.print(" rssi[");
        Serial.print(rssi);
        Serial.print("]");

        Serial.print(" freq[");
        Serial.print(freq);
        Serial.print("]");

        Serial.print(" time[");
        Serial.print(timeStamp);
        Serial.print("]");
      */
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

      partNumber = conteudo.substring(0, 4);
      String lastNumber = conteudo.substring(16, 20);
      String conteudoEPC = partNumber.toInt() + lastNumber + ',';

      Serial.println(partNumber.toInt());
      Serial.println(lastNumber);
      Serial.println(conteudoEPC);

      Serial1.print(conteudoEPC); //Send the EPC data to HOST >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      dadosRTC();
      datalogger();

      conteudo = "";

    } else if (responseType == ERROR_CORRUPT_RESPONSE) {

      Serial.println("Bad CRC");

    } else {

      //Unknown response
      Serial.print("Unknown error");

    }
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
