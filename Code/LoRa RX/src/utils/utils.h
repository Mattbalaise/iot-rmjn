#include <Arduino.h> 
#include <config/config.h>

String hexToAscii(const String &hex);
void hexToBinary(const String &hex, uint8_t *output, size_t outlen);
String convertHex(const uint8_t *data, size_t len);
void printLoraPayload(LoRaPayload lora);
bool checkFormatMessage(const String &message);