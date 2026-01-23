#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <Crypto.h>
#include <SHA256.h>
#include <AES.h>

// =====================
// Sécurité LoRa P2P
// =====================
#define BUFFER_MSG_SIZE 16
const char* HMAC_KEY = "super_secret_key_123";
unsigned long lastSeq = 0;
AES256 aes;
char key[32] = "romainsousfrozen";
char resultDecrypted[100];

// =====================
// LoRa
// =====================
#define LORA_RX 0
#define LORA_TX 1
SoftwareSerial loraSerial(LORA_RX, LORA_TX);

// =====================
// WiFi
// =====================
const char* ssid = "Jean Galaxy S24+";
const char* password = "ABCDE12345";

// =====================
// MQTT
// =====================
const char* mqttServer = "10.208.19.19";
const int mqttPort = 1883;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// =====================
// Fonctions utilitaires
// =====================

String hexToString(const String& hex) 
{
  String text = "";
  // On traite de 2 en 2 caractères
  for (unsigned int i = 0; i < hex.length(); i += 2) {
    // Vérifie qu'il reste au moins 2 caractères
    if (i + 1 >= hex.length()) break;

    String hexByte = hex.substring(i, i + 2);
    char c = (char) strtol(hexByte.c_str(), NULL, 16);
    text += c;
  }
  return text;
}


String convertHex(const uint8_t* data, size_t len) {
  String hex = "";
  for (size_t i = 0; i < len; i++) {
    if (data[i] < 0x10) hex += "0";       // Ajout d'un zéro pour chaque valeur < 0x10
    hex += String(data[i], HEX);
  }
  return hex;
}

// Connexion MQTT
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connexion MQTT...");
    if (mqttClient.connect("ArduinoR4_LoRa")) {
      Serial.println("OK");
    } else {
      Serial.print(" échec rc=");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

// =====================
// decrypt message
// =====================
String decrypt_message(const char* msg, char resultDecrypted[BUFFER_MSG_SIZE]) 
{
    aes.decryptBlock((uint8_t*)resultDecrypted, (const uint8_t*)msg);
}

// =====================
// SETUP
// =====================
void setup() {

  Serial.begin(9600);
  loraSerial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  // ---------- LoRa ----------
  loraSerial.println("AT");
  loraSerial.println("AT+RESET");
  delay(1500);
  loraSerial.println("AT+MODE=TEST");
  delay(500);
  loraSerial.println("AT+TEST=RFCFG,868.4,SF7,125,12,15,14,ON,OFF,OFF");
  loraSerial.println("AT+TEST=RXLRPKT");

  Serial.println("[LoRa RX] En écoute...");

  // ---------- WiFi ----------
  Serial.print("Connexion WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connecté");
  Serial.print("IP Arduino : ");
  Serial.println(WiFi.localIP());

  // ---------- MQTT ----------
  mqttClient.setServer(mqttServer, mqttPort);
}

// =====================
// LOOP
// =====================
void loop() {

  // Maintien MQTT
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  // ---------- Réception LoRa ----------
  if (loraSerial.available()) {

    digitalWrite(LED_BUILTIN, HIGH);
    String input = loraSerial.readStringUntil('\n');

    if (input.indexOf("+TEST: RX") != -1 && input.indexOf("\"") != -1) {

      int firstQuote = input.indexOf('\"');
      int lastQuote  = input.lastIndexOf('\"');

      if (lastQuote > firstQuote) {
        
          String hexMsg = input.substring(firstQuote + 1, lastQuote);
          // Décodage
          char msgDecrypted[BUFFER_MSG_SIZE];
          decrypt_message(hexMsg.c_str(), msgDecrypted);
          Serial.println(" hex decrypted : " + String(msgDecrypted));
          // String msgfinal = hexToString(msgDecrypted);
          // Serial.println("msg decrypted : " + msgfinal);
          mqttClient.publish("lora/reception", msgDecrypted);
          Serial.println("Envoyé au broker MQTT");
      }
    }
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }

  // Remise en écoute LoRa
  loraSerial.println("AT+TEST=RXLRPKT");
}
