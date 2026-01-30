#include <config/config.h>
#include <utils/utils.h>
//Fonction pour décrypter un message AES
void decrypt_message(const char* hexMsg, uint8_t resultDecrypted[PAYLOAD_SIZE]);
// Fonction pour créer un HMAC
void createHMAC(const uint8_t* message, uint8_t* hmacOut, size_t data_size);
// Fonction pour vérifier un HMAC
bool verifyHMAC(const uint8_t* message, const uint8_t* hmacReceived, size_t data_size);
// Fonction pour décrypter un message sécurisé et retourner la structure LoRaPayload
LoRaPayload decrypt_secure_message(String input);
// Fonction pour vérifier la séquence
bool verifySeq(uint32_t seq_count);