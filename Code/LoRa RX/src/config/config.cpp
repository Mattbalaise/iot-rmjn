#include <config/config.h>
// =====================
// Sécurité LoRa P2P
// =====================
char hmacKey[32] = "nonosousfrozen";
unsigned long lastSeq = 0;
char key[32] = "romainsousfrozen";
// =====================
// LoRa
// =====================
SoftwareSerial loraSerial(LORA_RX, LORA_TX);
// =====================
// WiFi
// =====================
const char *ssid = "POCO F3";
const char *password = "0123456789";
WiFiClient wifiClient; 
// =====================
// MQTT
// =====================
const char* mqttServer = "10.128.241.223";
const int mqttPort = 1883;
PubSubClient mqttClient(wifiClient);
// =====================
// AES
// ====================
AES256 aes;