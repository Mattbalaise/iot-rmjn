#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFiS3.h>
#include <PubSubClient.h>
#include <Crypto.h>
#include <SHA256.h>
#include <AES.h>
#include <hmac.h>
#include <utils.h>

// =====================
// Sécurité LoRa P2P
// =====================
<<<<<<< HEAD
#define PAYLOAD_SIZE 64
#define MESSAGE_SIZE 32  // Taille max message
#define HMAC_SIZE 32     // SHA256 HMAC
char hmacKey[32] = "nonosousfrozen";
=======
#define BUFFER_MSG_SIZE 16
const char *HMAC_KEY = "super_secret_key_123";
>>>>>>> c57bc64684279d41ceccf60151ccca66dfdf181c
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
<<<<<<< HEAD
const char *ssid = "SFR_8B4F";
const char *password = "5tppls139v6u6lm1h9cs";
=======
const char *ssid = "Jean Galaxy S24+";
const char *password = "ABCDE12345";
>>>>>>> c57bc64684279d41ceccf60151ccca66dfdf181c

// =====================
// MQTT
// =====================
<<<<<<< HEAD
const char *mqttServer = "192.168.1.42";
=======
const char *mqttServer = "10.207.123.19";
>>>>>>> c57bc64684279d41ceccf60151ccca66dfdf181c
const int mqttPort = 1883;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


// =====================
// Variables
// =====================
uint8_t binMsg[BUFFER_MSG_SIZE];
char decryptedMsg[BUFFER_MSG_SIZE + 1]; // +1 pour le '\0'

// =====================
// MQTT
// =====================
char decryptedMsg[PAYLOAD_SIZE + 1]; // +1 pour le '\0'

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


//-------------------------------------------//
//------------- AES FUNCTION  --------------//
//-----------------------------------------//

String decrypt_message(const char* hexMsg, char resultDecrypted[PAYLOAD_SIZE+1]) 
{
    uint8_t binMsg[PAYLOAD_SIZE];
    String sentHexMessage = hexToString(hexMsg);
    Serial.println("sentHexMessage : " + sentHexMessage);
    hexToBinary(sentHexMessage, binMsg, PAYLOAD_SIZE); // On récupère le binaire depuis HEX
    for(int i = 0; i < PAYLOAD_SIZE; i += 16){
        aes.decryptBlock((uint8_t*)resultDecrypted + i, binMsg + i);
    }
    resultDecrypted[PAYLOAD_SIZE] = '\0';
    return String(resultDecrypted);
}


//-------------------------------------------//
//------------- MQTT FUNCTION  --------------//
//-----------------------------------------//

void publishMessageToMqtt(uint8_t * receivedMessage)
{
    char msgBuffer[MESSAGE_SIZE + 1]; // +1 pour '\0'
    memcpy(msgBuffer, receivedMessage, MESSAGE_SIZE);
    msgBuffer[MESSAGE_SIZE] = '\0'; // Terminer correctement la chaîne
    String msg = "{\"temp\":\"" + String(msgBuffer) + "\"}";
    Serial.println(msg);
    mqttClient.publish("lora/reception", msg.c_str());
    Serial.println("Message envoyé au broker MQTT.");
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

  //Maintien MQTT
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
      int firstQuote = input.indexOf('\"');
      int lastQuote = input.lastIndexOf('\"');
      if (lastQuote > firstQuote)
      {
        Serial.println("Message reçu.");
        //decrypt message
        String hexMsg = input.substring(firstQuote + 1, lastQuote);
        decrypt_message(hexMsg.c_str(), decryptedMsg);
        //vérification du hmac
        uint8_t receivedMessage[MESSAGE_SIZE];
        uint8_t receivedHMAC[HMAC_SIZE];
        // Copier le message et le HMAC depuis le buffer déchiffré
        memcpy(receivedMessage, decryptedMsg, MESSAGE_SIZE);
        memcpy(receivedHMAC, decryptedMsg + MESSAGE_SIZE, HMAC_SIZE);
        bool verifyhmacvar = verifyHMAC(receivedMessage, receivedHMAC);
        if(verifyhmacvar)
        {
          Serial.println("HMAC OK");
          publishMessageToMqtt(receivedMessage);
        }
        else
        {
          Serial.println("HMAC NOT OK [Le message est peut-être corrompu, il ne sera pas envoyé au broker MQTT]");
        }
        Serial.println("---------------------------------------------------");
      }
    }
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }

  // Remise en écoute LoRa
  loraSerial.println("AT+TEST=RXLRPKT");
}