#include <security/security.h>

uint32_t last_seq_count = 0;

//-------------------------------------------//
//------------- AES FUNCTION  --------------//
//-----------------------------------------//
void decrypt_message(const char* hexMsg, uint8_t resultDecrypted[PAYLOAD_SIZE]) 
{
    memset(resultDecrypted, 0, PAYLOAD_SIZE);
    uint8_t binMsg[PAYLOAD_SIZE];
    String sentHexMessage = hexToAscii(hexMsg);
    Serial.println("sentHexMessage : " + sentHexMessage);
    hexToBinary(sentHexMessage, binMsg, PAYLOAD_SIZE); // On récupère le binaire depuis HEX
    for(int i = 0; i < PAYLOAD_SIZE; i += 16){
      aes.decryptBlock(resultDecrypted + i, binMsg + i);
    }
}
//----------------------------------------//
// Fonction pour créer un HMAC
//----------------------------------------//
void createHMAC(const uint8_t* message, uint8_t* hmacOut, size_t data_size) 
{
    SHA256 sha;
    sha.resetHMAC(hmacKey, sizeof(hmacKey));
    sha.update(message, data_size); // calcul sur message clair
    sha.finalize(hmacOut, HMAC_SIZE);
}
//----------------------------------------//
// Fonction pour vérifier un HMAC
//----------------------------------------//
bool verifyHMAC(const uint8_t* message, const uint8_t* hmacReceived, size_t data_size) {
    uint8_t expectedHMAC[HMAC_SIZE];
    createHMAC(message, expectedHMAC, data_size); // recalcule HMAC attendu
    // Comparer octet par octet
    for(int i = 0; i < HMAC_SIZE; i++) {
        if(hmacReceived[i] != expectedHMAC[i]) {
            return false; // HMAC différent → message corrompu
        }
    }
    return true; // Tout correspond → HMAC valide
}

LoRaPayload decrypt_secure_message(String input){
    int firstQuote = input.indexOf('\"');
    int lastQuote = input.lastIndexOf('\"');
    if (lastQuote > firstQuote)
    {
        uint8_t resultDecrypted[PAYLOAD_SIZE];
        String hexMsg = input.substring(firstQuote + 1, lastQuote);
        decrypt_message(hexMsg.c_str(), resultDecrypted);
        LoRaPayload lora = *(LoRaPayload *)resultDecrypted;
        printLoraPayload(lora);
        return lora;
    }
    return {};// Retourne une structure vide si la décryption échoue
}

//----------------------------------------//
// Fonction pour vérifier la séquence
//----------------------------------------//
bool verifySeq(uint32_t seq_count){
    if(seq_count > last_seq_count){
        last_seq_count = seq_count;
        return true; // Séquence valide
    }
    return false; // Séquence invalide (rejetée)
}