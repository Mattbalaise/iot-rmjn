#include <Arduino.h>
#include <SHA256.h>
#include <Ed25519.h>
#include <AES.h>
#include <ctime> 
#include <WiFiS3.h>
#include "hmac.h"
#include <time.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

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
#define MESSAGE_SIZE 39  // Taille max message
#define HMAC_SIZE 32     // SHA256 HMAC


struct LoRaPayload {
    uint8_t id_device;               // 1 octet
    uint32_t seq_count;              // 4 octets (0-4294967295)
    uint8_t message[MESSAGE_SIZE];   // 39 octets (ajusté)
    uint32_t timestamp;                // 4 octets
    uint8_t hmac[HMAC_SIZE];         // 32 octet
} __attribute__((packed));           // Total = 80 octets toujours


#define PAYLOAD_SIZE sizeof(LoRaPayload)
#define SEQ_COUNT_SIZE sizeof(seq_count)
#define ID_END_DEVICE_SIZE sizeof(id_device)
uint8_t payloadEncrypted[PAYLOAD_SIZE];
LoRaPayload payload;
uint8_t privateKey[32];
uint8_t publicKey[32];
AES256 aes;
SHA256 sha;
// variable globales (à mettre dans un .env par la suite)
static uint16_t seq_count = 0;
static uint8_t id_device = 0x01;
//key pour aes et hmac
char aesKey[32] = "romainsousfrozen";
char hmacKey[32] = "nonosousfrozen";
//message à envoyer
const char* LORA_MSG = "jeansousfrozen";


// =====================
// WiFi
// =====================
const char *ssid = "POCO F3";
const char *password = "0123456789";


// =====================
// NTP
// =====================
WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, "pool.ntp.org", 0, 60000);

// =====================
// Time state
// =====================
uint32_t baseEpoch = 0;
uint32_t baseMillis = 0;

// =====================
// Sync NTP
// =====================
bool syncTime() {
  if (!ntpClient.update()) {
    return false;
  }

  baseEpoch = ntpClient.getEpochTime();
  baseMillis = millis();
  return true;
}

// =====================
// Current Unix timestamp
// =====================
uint32_t now() {
  return baseEpoch + (millis() - baseMillis) / 1000;
}



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

void decrypt_message(const uint8_t* encrypted, LoRaPayload* resultDecrypted) {
    for(int i = 0; i < PAYLOAD_SIZE; i += 16){
        aes.decryptBlock((uint8_t*)resultDecrypted + i, encrypted + i);
    }
}

//-------------------------------------------//
//-----------  MESSAGE FUNCTION  -----------//
//-----------------------------------------//

void send_message(const char* msg) 
{
    // --- Chiffrement et envoi ---
    memset(&payload, 0, sizeof(payload)); // tout à 0
    memcpy(&payload.message, msg, MESSAGE_SIZE); // copier message
    payload.id_device = id_device; // ID de l'end device
    payload.seq_count = seq_count++; // incrémenter le compteur de séquence
    payload.timestamp = now(); // timestamp actuel
    createHMAC((uint8_t*)&payload, payload.hmac, PAYLOAD_SIZE - HMAC_SIZE); // créer HMAC sur le message

    encode_message(payloadEncrypted, (uint8_t*)&payload);//encode le message via aes
    String payloadEncryptedHex = convertHex(payloadEncrypted, PAYLOAD_SIZE);//conversion en hexa
    LORA_SERIAL.print("AT+TEST=TXLRSTR \"" + payloadEncryptedHex  + "\"");//envoie du message
    Serial.println("Envoi LoRa : " + payloadEncryptedHex);
    blink_led_times(BLINK_COUNT, BLINK_DELAY);

    // --- Test déchiffrement local ---
    Serial.println("\n========== TEST DÉCHIFFREMENT LOCAL ==========");
    uint8_t testEncrypted[PAYLOAD_SIZE];
    LoRaPayload testDecrypted;  // Utiliser la struct, pas char[]
    // 1. Convertir hex → binaire
    hexToBinary(payloadEncryptedHex, testEncrypted, PAYLOAD_SIZE);
    // 2. Déchiffrer dans la struct
    decrypt_message(testEncrypted, &testDecrypted);
    // 3. Afficher les données déchiffrées
    Serial.print("ID déchiffré: 0x");
    Serial.println(testDecrypted.id_device, HEX);
    Serial.print("SEQ déchiffré: ");
    Serial.println(testDecrypted.seq_count);
    Serial.print("TimeStamp déchiffré: ");
    Serial.println(testDecrypted.timestamp);
    Serial.print("Message déchiffré: ");
    for(int i = 0; i < MESSAGE_SIZE && testDecrypted.message[i] != 0; i++) {
        Serial.print((char)testDecrypted.message[i]);
    }
    Serial.println();
    Serial.print("HMAC reçu (premiers 8 octets): ");
    printHex(testDecrypted.hmac, 8);
    // 4. Vérifier le HMAC (passer toute la struct)
    if (verifyHMAC((uint8_t*)&testDecrypted, testDecrypted.hmac, PAYLOAD_SIZE - HMAC_SIZE)) {
        Serial.println(" HMAC VALIDE - Message authentique ");
    } else {
        Serial.println("HMAC INVALIDE - Message falsifié !");
    }
    Serial.println("==============================================\n");
}

//-------------------------------------------//
//----------- DEFAULT FUNCTION  ------------//
//-----------------------------------------//

void setup()
{
  Serial.begin(SERIAL_BAUD);
  LORA_SERIAL.begin(LORA_BAUD);
  pinMode(LED_PIN, OUTPUT);

  // ---------- WiFi ----------
  Serial.print("Connexion WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  
  // NTP init
  ntpClient.begin();

  delay(500);
  
  if (syncTime()) {
    Serial.print("Time synced: ");
    Serial.println(baseEpoch);
  } else {
    Serial.println("NTP sync failed");
  }

  Serial.println("\nWiFi connecté");
  Serial.print("IP Arduino : ");
  Serial.println(WiFi.localIP());

  delay(1000);

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
  static unsigned long lastResync = 0;

  if(LORA_SERIAL.available()) {
    send_message(LORA_MSG);
    Serial.println("Sent packet");
  }

  if (millis() - lastResync > 600000UL) {
    if (syncTime()) {
      Serial.println("NTP resynced");
    }
  lastResync = millis();
  }

  delay(1000);
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


