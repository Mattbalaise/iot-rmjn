#include <network/network.h>
 // ---------- MQTT ----------
void initMqttClient()
{
  mqttClient.setServer(mqttServer, mqttPort);
}

String FormatMsgMqtt(char * msgBuffer)
{
    std::vector<std::string> mesures;
    std::string msgStr(msgBuffer);  // Créer une std::string
    split(mesures, msgStr, ",");    // split modifie msgStr
   String output = "{";
    for(int i = 0; i < mesures.size(); i++)
    {
      std::string mesure = mesures[i];
      size_t delimiterPos = mesure.find(':');
      if (delimiterPos != std::string::npos) 
      {
          std::string key = mesure.substr(0, delimiterPos);
          std::string value = mesure.substr(delimiterPos + 1);
          output += "\"" + String(key.c_str()) + "\":\"" + String(value.c_str()) + "\"";
          if(i < mesures.size() - 1)
          {
            output += ",";
          }
      }
    }
    output += "}";
    return output;
}

void publishMessageToMqtt(uint8_t * receivedMessage)
{
    char msgBuffer[MESSAGE_SIZE + 1]; // +1 pour '\0'
    memcpy(msgBuffer, receivedMessage, MESSAGE_SIZE);
    msgBuffer[MESSAGE_SIZE] = '\0';
    bool isValid = checkFormatMessage(String(msgBuffer));
    if(!isValid)
    {
      Serial.println("Format de message invalide. Le message ne sera pas envoyé au broker MQTT.");
      return;
    }
    String output = FormatMsgMqtt(msgBuffer);
    mqttClient.publish("lora/reception", output.c_str());
    Serial.println("Message envoyé au broker MQTT.");
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


//Maintient mqtt
void mqttLoop()
{
  if (!mqttClient.connected())
  {
    reconnectMQTT();
  }
  mqttClient.loop();
}

// ---------- WiFi ----------
void connectWifi()
{
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
}

//---------- Lora ----------
void initLora(){
    // ---------- LoRa ----------
  Serial.println("[LoRa RX] Initialisation LoRa...");
  loraSerial.println("AT");
  loraSerial.println("AT+RESET");
  delay(1500);
  loraSerial.println("AT+MODE=TEST");
  delay(500);
  loraSerial.println("AT+TEST=RFCFG,868.4,SF7,125,12,15,14,ON,OFF,OFF");
  loraSerial.println("AT+TEST=RXLRPKT");
  Serial.println("[LoRa RX] Initialisation Wifi...");
}