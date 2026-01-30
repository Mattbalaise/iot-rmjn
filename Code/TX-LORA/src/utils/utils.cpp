#include <utils/utils.h>
//-------------------------------------------//
//-----------  HEXA FUNCTION  --------------//
//-----------------------------------------//
void printHex(const uint8_t *data, size_t len) 
{
  for (size_t i = 0; i < len; i++) {
    if (data[i] < 0x10) Serial.print("0"); // ajoute un zéro pour aligner
    Serial.print(data[i], HEX);
  }
  Serial.println();
}

String convertHex(const uint8_t* data, size_t len) {
  String hex = "";
  for (size_t i = 0; i < len; i++) {
    if (data[i] < 0x10) hex += "0";       // Ajout d'un zéro pour chaque valeur < 0x10
    hex += String(data[i], HEX);
  }
  return hex;
}

void hexToBinary(const String& hex, uint8_t* output, size_t outlen) {
  for (size_t i = 0; i < outlen; i++) {
    String byteString = hex.substring(i*2, i*2+2);
    output[i] = (uint8_t)strtol(byteString.c_str(), NULL, 16);
  }
}

//-------------------------------------------//
//-----------   LED FUNCTION  --------------//
//-----------------------------------------//

void blink_led_times(uint8_t times, uint16_t delay_ms) 
{
  for(uint8_t i = 0; i < times; ++i) {
    digitalWrite(LED_PIN, HIGH);
    delay(delay_ms);
    digitalWrite(LED_PIN, LOW);
    delay(delay_ms);
  }
}

String format_message(float* data){
  String message = "";
  message += "temp:" + String(data[0], 2) + ";"
  "humidity:" + String(data[1], 2) + ";" +
  "pressure:" + String(data[2]) + ";";
  return message;
}