#include <Arduino.h>
#include <SHA256.h>
#include <Ed25519.h>
#include <AES.h>
#include <ctime> 
#include <WiFiS3.h>
#include <time.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#ifndef CONFIG_H
#define CONFIG_H
//------------------------------------------//
//-------------  VARIABLE  ----------------//
//----------------------------------------//
#define LORA_SERIAL      Serial1
#define LORA_BAUD        9600
#define SERIAL_BAUD      9600
#define LORA_FREQ        "868.4"
#define LORA_SF          "7"
#define LED_PIN          LED_BUILTIN
#define BLINK_COUNT      3
#define BLINK_DELAY      200
#define MESSAGE_SIZE 39  // Taille max message
#define HMAC_SIZE 32     // SHA256 HMAC


struct LoRaPayload {
    uint8_t id_device;               // 1 octet
    uint32_t seq_count;              // 4 octets (0-4294967295)
    uint8_t message[MESSAGE_SIZE];   // 39 octets (ajusté)
    uint32_t timestamp;                // 4 octets
    uint8_t hmac[HMAC_SIZE];         // 32 octet
} __attribute__((packed));           // Total = 80 octets toujours


#define PAYLOAD_SIZE sizeof(LoRaPayload)
#define SEQ_COUNT_SIZE sizeof(seq_count)
#define ID_END_DEVICE_SIZE sizeof(id_device)
extern uint8_t payloadEncrypted[PAYLOAD_SIZE];
extern LoRaPayload payload;
extern uint8_t privateKey[32];
extern uint8_t publicKey[32];
extern AES256 aes;
extern SHA256 sha;
// variable globales (à mettre dans un .env par la suite)
extern uint16_t seq_count;
extern uint8_t id_device;
//key pour aes et hmac
extern char aesKey[32];
extern char hmacKey[32];
extern unsigned long lastResync;
//message à envoyer
extern char* LORA_MSG;


// =====================
// WiFi
// =====================
extern const char *ssid;
extern const char *password;


// =====================
// NTP
// =====================
extern WiFiUDP ntpUDP;
extern NTPClient ntpClient;

// =====================
// Time state
// =====================
extern uint32_t baseEpoch;
extern uint32_t baseMillis;
#endif // CONFIG_H