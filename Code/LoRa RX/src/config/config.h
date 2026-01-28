#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <AES.h>
#include <PubSubClient.h>
#include <WiFiS3.h>
#include <SoftwareSerial.h>
#include <SHA256.h>
// CONSTANTES
#define MESSAGE_SIZE 39
#define HMAC_SIZE 32
#define LORA_RX 0
#define LORA_TX 1
// STRUCTURE PAYLOAD
struct LoRaPayload {
    uint8_t id_device;               // 1 octet
    uint32_t seq_count;              // 4 octets
    uint8_t message[MESSAGE_SIZE];   // 39 octets
    uint32_t timestamp;              // 4 octets
    uint8_t hmac[HMAC_SIZE];         // 32 octets
} __attribute__((packed));           // Total = 80 octets
#define PAYLOAD_SIZE sizeof(LoRaPayload)
// =====================
// VARIABLES GLOBALES (déclarations extern)
// =====================
// Sécurité
extern char hmacKey[32];
extern char aesKey[32];
extern unsigned long lastSeq;
extern AES256 aes;
extern char key[32];
// LoRa
extern SoftwareSerial loraSerial;
// WiFi
extern const char* ssid;
extern const char* password;
// MQTT
extern const char* mqttServer;
extern const int mqttPort;
extern PubSubClient mqttClient;
#endif