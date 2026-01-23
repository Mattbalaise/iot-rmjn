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
const char *HMAC_KEY = "super_secret_key_123";
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
const char *ssid = "Jean Galaxy S24+";
const char *password = "ABCDE12345";

// =====================
// MQTT
// =====================
const char *mqttServer = "10.207.123.19";
const int mqttPort = 1883;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


// =====================
// Variables
// =====================
uint8_t binMsg[BUFFER_MSG_SIZE];
char decryptedMsg[BUFFER_MSG_SIZE + 1]; // +1 pour le '\0'

// =====================
// Fonctions utilitaires
// =====================

String hexToString(const String &hex)
{
  String hexClean = hex;
  hexClean.trim();
  String text = "";

  // On traite de 2 en 2 caractères
  for (unsigned int i = 0; i < hexClean.length(); i += 2)
  {
    // Vérifie qu'il reste au moins 2 caractères
    if (i + 1 >= hexClean.length())
      break;

    String hexByte = hexClean.substring(i, i + 2);
    char c = (char)strtol(hexByte.c_str(), NULL, 16);
    text += c;
  }
  return text;
}

void hexToBinary(const String &hex, uint8_t *output, size_t outlen)
{
  for (size_t i = 0; i < outlen; i++)
  {
    String byteString = hex.substring(i * 2, i * 2 + 2);
    output[i] = (uint8_t)strtol(byteString.c_str(), NULL, 16);
  }
}

String convertHex(const uint8_t *data, size_t len)
{
  String hex = "";
  for (size_t i = 0; i < len; i++)
  {
    if (data[i] < 0x10)
      hex += "0"; // Ajout d'un zéro pour chaque valeur < 0x10
    hex += String(data[i], HEX);
  }
  return hex;
}

// Connexion MQTT
void reconnectMQTT()
{
  while (!mqttClient.connected())
  {
    Serial.print("Connexion MQTT...");
    if (mqttClient.connect("ArduinoR4_LoRa"))
    {
      Serial.println("OK");
      Serial.println("[LoRa RX] En écoute...");
    }
    else
    {
      Serial.print(" échec rc=");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

// =====================
// Decrypt message
// =====================
void decrypt_message(const uint8_t *encrypted, char resultDecrypted[BUFFER_MSG_SIZE])
{
  aes.decryptBlock((uint8_t *)resultDecrypted, encrypted);
  resultDecrypted[BUFFER_MSG_SIZE - 1] = '\0'; // finis la string (si c'est du texte)
}

// =====================
// SETUP
// =====================
void setup()
{

  // Set encryption key
  aes.setKey((uint8_t *)key, strlen(key));

  Serial.begin(9600);
  loraSerial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("[LoRa RX] Initialisation LoRa...");

  // ---------- LoRa ----------
  loraSerial.println("AT");
  loraSerial.println("AT+RESET");
  delay(1500);
  loraSerial.println("AT+MODE=TEST");
  delay(500);
  loraSerial.println("AT+TEST=RFCFG,868.4,SF7,125,12,15,14,ON,OFF,OFF");
  loraSerial.println("AT+TEST=RXLRPKT");

  Serial.println("[LoRa RX] Initialisation Wifi...");

  // ---------- WiFi ----------
  Serial.print("Connexion WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
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
void loop()
{

  // Maintien MQTT
  if (!mqttClient.connected())
  {
    reconnectMQTT();
  }
  mqttClient.loop();

  // ---------- Réception LoRa ----------
  if (loraSerial.available())
  {

    digitalWrite(LED_BUILTIN, HIGH);
    String input = loraSerial.readStringUntil('\n');

    if (input.indexOf("+TEST: RX") != -1 && input.indexOf("\"") != -1)
    {
      Serial.println(input);
      int firstQuote = input.indexOf('\"');
      int lastQuote = input.lastIndexOf('\"');

      if (lastQuote > firstQuote)
      {
        Serial.println("Message reçu.");
        String hexMsg = input.substring(firstQuote + 1, lastQuote);
        Serial.println("hexMsg : " + hexMsg);
        String sentHexMessage = hexToString(hexMsg);
        Serial.println("sentHexMessage : " + sentHexMessage);

        hexToBinary(sentHexMessage, binMsg, BUFFER_MSG_SIZE); // On récupère le binaire depuis HEX
        decrypt_message(binMsg, decryptedMsg);

        Serial.println(" Déchiffré : " + String(decryptedMsg));
        mqttClient.publish("lora/reception", decryptedMsg);
        Serial.println("Message envoyé au broker MQTT.");
        Serial.println("---------------------------------------------------");
      }
    }
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }

  // Remise en écoute LoRa
  loraSerial.println("AT+TEST=RXLRPKT");
}
