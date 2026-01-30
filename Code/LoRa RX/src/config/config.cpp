#include <config/config.h>
#include <map>
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
const char* mqttServer = "10.31.154.149";
const int mqttPort = 1883;
PubSubClient mqttClient(wifiClient);
// =====================
// AES
// ====================
AES256 aes;

//faire un dictonnaire en c++ 
std::map<std::uint16_t, std::pair<std::string, std::string>> device_room = {
    {1, {"Kastler", "128"}},
    {2, {"Kastler","127"}},
    {3, {"Kastler","101"}},
    {4, {"Schweitzer","203"}},
    {5, {"Schweitzer","217"}}
};