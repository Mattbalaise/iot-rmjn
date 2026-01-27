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
#define MESSAGE_SIZE 39  // Taille max message
#define HMAC_SIZE 32     // SHA256 HMAC
char hmacKey[32] = "nonosousfrozen";
unsigned long lastSeq = 0;
AES256 aes;
char key[32] = "romainsousfrozen";
char resultDecrypted[100];

struct LoRaPayload {
    uint8_t id_device;               // 1 octet
    uint32_t seq_count;              // 4 octets (0-4294967295)
    uint8_t message[MESSAGE_SIZE];   // 39 octets (ajusté)
    uint32_t timestamp;                // 4 octets
    uint8_t hmac[HMAC_SIZE];         // 32 octet
} __attribute__((packed));           // Total = 80 octets toujours

#define PAYLOAD_SIZE sizeof(LoRaPayload)

// =====================
// LoRa
// =====================
#define LORA_RX 0
#define LORA_TX 1
SoftwareSerial loraSerial(LORA_RX, LORA_TX);

// =====================
// WiFi
// =====================
const char *ssid = "POCO F3";
const char *password = "0123456789";

// =====================
// MQTT
// =====================
const char *mqttServer = "10.187.221.149";
const int mqttPort = 1883;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
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
void decrypt_message(const char* hexMsg, uint8_t resultDecrypted[PAYLOAD_SIZE]) 
{
    memset(resultDecrypted, 0, PAYLOAD_SIZE);
    uint8_t binMsg[PAYLOAD_SIZE];
    String sentHexMessage = hexToString(hexMsg);
    Serial.println("sentHexMessage : " + sentHexMessage);
    hexToBinary(sentHexMessage, binMsg, PAYLOAD_SIZE); // On récupère le binaire depuis HEX
    for(int i = 0; i < PAYLOAD_SIZE; i += 16){
      aes.decryptBlock(resultDecrypted + i, binMsg + i);
    }
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
        uint8_t resultDecrypted[PAYLOAD_SIZE];
        decrypt_message(hexMsg.c_str(), resultDecrypted);
        LoRaPayload lora = *(LoRaPayload *)resultDecrypted;
        // Serial.print("ID Device: ");
        // Serial.println(lora.id_device);
        // Serial.print("Seq Count: ");
        // Serial.println(lora.seq_count);
        // Serial.print("Timestamp: ");
        // Serial.println(lora.timestamp);
        // Serial.print("Message: ");
        // Serial.println((char*)lora.message);
        // Serial.println();
        // Serial.print("HMAC Reçu: ");
        // for(int i = 0; i < HMAC_SIZE; i++) {
        //   Serial.print(lora.hmac[i], HEX);
        // }
        // Serial.println();
        //vérification du hmac
        bool verifyhmacvar = verifyHMAC((uint8_t*)&lora, lora.hmac, PAYLOAD_SIZE - HMAC_SIZE);
        if(verifyhmacvar)
        {
          Serial.println("HMAC OK");
          publishMessageToMqtt(lora.message);
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