#include <SoftwareSerial.h>

SoftwareSerial ESP32(11, 10); //(RX, TX)

String conteudoESP32 = "PART NUMBER;TIPO;DESCRICAO;QTD";
String conteudoESP32value = "EL1;CAPACITIVO;SENSOR BALLUFF;3";
byte num = 0;

void setup() {

  Serial.begin(115200);
  ESP32.begin(115200);
}

void loop() {

  num++;

  if(num == 3) num = 0;

  if(num == 1) ESP32.println(conteudoESP32); Serial.println(conteudoESP32);

  if(num == 2)ESP32.println(conteudoESP32value); Serial.println(conteudoESP32value);

  delay(100);

}
