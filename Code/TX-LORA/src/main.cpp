#include <Arduino.h>
#include <SHA256.h>
#include <Ed25519.h>
#include <AES.h>
#include "hmac.h"

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
#define PAYLOAD_SIZE 64
#define MESSAGE_SIZE 32  // Taille max message
#define HMAC_SIZE 32     // SHA256 HMAC

uint8_t privateKey[32];
uint8_t publicKey[32];
char message[MESSAGE_SIZE] = "23"; // message clair
uint8_t payload[PAYLOAD_SIZE];                      // buffer final
uint8_t payloadEncrypted[PAYLOAD_SIZE];                      // buffer final
uint8_t hmacResult[HMAC_SIZE];
AES256 aes;
SHA256 sha;
char aesKey[32] = "romainsousfrozen";
char hmacKey[32] = "nonosousfrozen";
const char* LORA_MSG = "romaingddddd";




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


//-------------------------------------------//
//------------- AES FUNCTION  --------------//
//-----------------------------------------//

// Version: le message (et le tableau résultat) doivent faire 16 octets minimum
void encode_message(uint8_t* resultEncrypted, uint8_t* msg) {
    for(int i = 0; i < PAYLOAD_SIZE; i += 16){
        aes.encryptBlock(resultEncrypted + i, msg + i);
    }
}

String decrypt_message(const uint8_t* encrypted, char resultDecrypted[PAYLOAD_SIZE+1]) {
    for(int i = 0; i < PAYLOAD_SIZE; i += 16){
        aes.decryptBlock((uint8_t*)resultDecrypted + i, encrypted + i);
    }
    resultDecrypted[PAYLOAD_SIZE] = '\0';
    return String(resultDecrypted);
}

//-------------------------------------------//
//-----------  MESSAGE FUNCTION  -----------//
//-----------------------------------------//

void send_message(const char* msg) 
{
    // --- Chiffrement et envoi ---
    memset(payload, 0, PAYLOAD_SIZE); // tout à 0
    memcpy(payload, message, strlen(message)); // copier message
    createHMAC((uint8_t*)message, payload + MESSAGE_SIZE);//création du hmac
    encode_message(payloadEncrypted, payload);//encode le message via aes
    String payloadEncryptedHex = convertHex(payloadEncrypted, PAYLOAD_SIZE);//conversion en hexa
    LORA_SERIAL.print("AT+TEST=TXLRSTR \"" + payloadEncryptedHex  + "\"");//envoie du message
    Serial.println("Envoi LoRa : " + payloadEncryptedHex);
    blink_led_times(BLINK_COUNT, BLINK_DELAY);






    
    // --- Test déchiffrement local ---
    uint8_t test[PAYLOAD_SIZE];
    char decrypted[PAYLOAD_SIZE+1]; // +1 pour le '\0'

    hexToBinary(payloadEncryptedHex, test, PAYLOAD_SIZE); // On récupère le binaire depuis HEX
    decrypt_message(test, decrypted);
    //vérification du hmac
    uint8_t receivedMessage[MESSAGE_SIZE];
    uint8_t receivedHMAC[HMAC_SIZE];
    // Copier le message et le HMAC depuis le buffer déchiffré
    memcpy(receivedMessage, decrypted, MESSAGE_SIZE);
    memcpy(receivedHMAC, decrypted + MESSAGE_SIZE, HMAC_SIZE);
    uint8_t expectedHMAC[HMAC_SIZE];
    bool verifyhmacvar = verifyHMAC(receivedMessage, receivedHMAC);
    if(verifyhmacvar){
      Serial.println("c'est ok le hmac");
      Serial.println("message : " + String(receivedMessage, MESSAGE_SIZE));
    }
    else{
            Serial.println("nok hmac");
    }


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
  aes.setKey(((uint8_t*)aesKey), strlen(aesKey));
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


