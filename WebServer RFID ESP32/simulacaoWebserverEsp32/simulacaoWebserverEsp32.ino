#include <SPI.h>
#include <mySD.h>

#define SS 5;

File myFile;

String conteudo = "";

void setup() {

  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 4, 2);   // INTERFACE DE COMUNICAÇÃO UART ARDUINO UNO

  Serial.print("Initializing SD card...");

  pinMode(SS, OUTPUT);

  if (!SD.begin(chipSelect, 23, 19, 18)) {               //(CS, MOSI, MISO, SCK)
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

}

void loop() {

  if (Serial1.available()) {

    conteudo = Serial1.readStringUntil('\n');
    Serial.println(conteudo);

  }

  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}
