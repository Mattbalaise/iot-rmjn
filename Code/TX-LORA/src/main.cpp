#include <Arduino.h>
#include <SHA256.h>
#include <Ed25519.h>
#include <AES.h>

//------------------------------------------//
//-------------  VARIABLE  ----------------//
//----------------------------------------//
#define LORA_SERIAL      Serial1
#define LORA_BAUD        9600
#define SERIAL_BAUD      9600
#define LORA_FREQ        "868.4"
#define LORA_SF          "7"
#define LED_PIN          LED_BUILTIN
#define BLINK_COUNT      3
#define BLINK_DELAY      200
#define BUFFER_MSG_SIZE 16

uint8_t privateKey[32];
uint8_t publicKey[32];
char msgEncrypted[BUFFER_MSG_SIZE];
AES256 aes;
char key[32] = "romainsousfrozen";
const char* LORA_MSG = "abcdefghijk";



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


//-------------------------------------------//
//----------- MESSAGE FUNCTION  ------------//
//-----------------------------------------//

// Version: le message (et le tableau résultat) doivent faire 16 octets minimum
void encode_message(const char* msg, char resultEncrypted[16]) 
{
    aes.encryptBlock((uint8_t*)resultEncrypted, (const uint8_t*)msg);
}

void send_message(const char* msg) 
{
  String msgHex = convertHex((uint8_t*)msg, BUFFER_MSG_SIZE);
  encode_message(msgHex.c_str(), msgEncrypted);
  LORA_SERIAL.print("AT+TEST=TXLRSTR \"" + String(msgEncrypted)  + "\"");
  Serial.println("Envoi LoRa : " + convertHex((uint8_t*)msgEncrypted, BUFFER_MSG_SIZE));
  blink_led_times(BLINK_COUNT, BLINK_DELAY);
}


//-------------------------------------------//
//----------- DEFAULT FUNCTION  ------------//
//-----------------------------------------//


void setup() 
{
  Serial.begin(SERIAL_BAUD);
  LORA_SERIAL.begin(LORA_BAUD);
  pinMode(LED_PIN, OUTPUT);

  Serial.println("Initializing LoRa Module...");
  LORA_SERIAL.println("AT");
  LORA_SERIAL.println("AT+RESET");
  delay(1500);
  
  LORA_SERIAL.println("AT+MODE=TEST");
  delay(500);
  String at_rfcfg = String("AT+TEST=RFCFG,") + LORA_FREQ + ",SF" + LORA_SF + ",125,12,15,14,ON,OFF,OFF";
  LORA_SERIAL.println(at_rfcfg);
  //set key ash256
  aes.setKey(((uint8_t*)key), strlen(key));
}

void loop() {
  if(LORA_SERIAL.available()) {
    send_message(LORA_MSG);
    Serial.println("Sent packet");
    delay(1000);
  }
}


// void hash_test(){
//     // Génération des clés
//   Ed25519::generatePrivateKey(privateKey);
//   Ed25519::derivePublicKey(publicKey, privateKey);
//   printHex(privateKey, 32);
//   printHex(publicKey, 32); 
  
//   const char* message = "Hello blockchain!";
//   uint8_t signature[64];

//   Ed25519::sign(signature, privateKey, publicKey, message, strlen(message));
//   bool valid = Ed25519::verify(signature, publicKey, message, strlen(message));

//   if (valid) {
//     Serial.println("Signature valide !");
// } else {
//     Serial.println("Signature INVALIDE !");
// }
//       // Génération des clés
//     Ed25519::generatePrivateKey(privateKey);
//     Ed25519::derivePublicKey(publicKey, privateKey);
//     valid = Ed25519::verify(signature, publicKey, message, strlen(message));
//     if (valid) {
//       Serial.println("Signature valide !");
//   } else {
//       Serial.println("Signature INVALIDE !");
//   }
// }