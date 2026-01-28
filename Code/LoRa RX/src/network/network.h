
#include <PubSubClient.h>
#include <wifi.h>
#include <config/config.h>
#include <utils/utils.h>
//  MQTT
void initMqttClient();
void publishMessageToMqtt(uint8_t * receivedMessage);
void reconnectMQTT();
void mqttLoop();
//  WiFi
void connectWifi();
//Lora
void initLora();
