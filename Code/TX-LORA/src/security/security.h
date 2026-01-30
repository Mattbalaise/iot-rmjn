#include <Arduino.h>
#include <SHA256.h>
#include <config/config.h>
#include <utils/utils.h>
#include <network/network.h>

//----------------------------------------//
// Fonction pour créer un HMAC
//----------------------------------------//
void createHMAC(const uint8_t* message, uint8_t* hmacOut, size_t data_size);

//----------------------------------------//
// Fonction pour vérifier un HMAC
//----------------------------------------//
bool verifyHMAC(const uint8_t* message, const uint8_t* hmacReceived, size_t data_size);
//------------- AES FUNCTION  --------------//
void encode_message(uint8_t* resultEncrypted, uint8_t* msg);

void decrypt_message(const uint8_t* encrypted, LoRaPayload* resultDecrypted);
//-------------  MESSAGE FUNCTION  -----------//
String secure_message(const char * msg);

void send_secure_message(String secure_message_str);

void decode_local_message(String payloadEncryptedHex);