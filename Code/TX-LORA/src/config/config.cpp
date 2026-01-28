#include <Arduino.h>
#include <SHA256.h>
#include <Ed25519.h>
#include <AES.h>
#include <ctime> 
#include <WiFiS3.h>
#include <time.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <config/config.h>

uint8_t payloadEncrypted[PAYLOAD_SIZE];
LoRaPayload payload;
uint8_t privateKey[32];
uint8_t publicKey[32];
AES256 aes;
SHA256 sha;
// variable globales (à mettre dans un .env par la suite)
uint16_t seq_count = 0;
uint8_t id_device = 1;
//key pour aes et hmac
char aesKey[32] = "romainsousfrozen";
char hmacKey[32] = "nonosousfrozen";
//message à envoyer
char* LORA_MSG = "jeansousfrozen";
unsigned long lastResync = 0;

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