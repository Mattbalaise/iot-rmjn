#include <Arduino.h>
#include <config/config.h>

//-------------------------------------------//
//-----------  HEXA FUNCTION  --------------//
//-----------------------------------------//
void printHex(const uint8_t *data, size_t len);

String convertHex(const uint8_t* data, size_t len);

void hexToBinary(const String& hex, uint8_t* output, size_t outlen);

//-------------------------------------------//
//-----------   LED FUNCTION  --------------//
//-----------------------------------------//

void blink_led_times(uint8_t times, uint16_t delay_ms) ;