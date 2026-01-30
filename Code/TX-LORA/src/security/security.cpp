#include <security/security.h>

// Clé HMAC partagée
extern char hmacKey[32]; // à définir dans ton main.cpp

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


//-------------------------------------------//
//------------- AES FUNCTION  --------------//
//-----------------------------------------//

// Version: le message (et le tableau résultat) doivent faire 16 octets minimum
void encode_message(uint8_t* resultEncrypted, uint8_t* msg) {
    for(int i = 0; i < PAYLOAD_SIZE; i += 16){
        aes.encryptBlock(resultEncrypted + i, msg + i);
    }
}

void decrypt_message(const uint8_t* encrypted, LoRaPayload* resultDecrypted) {
    for(int i = 0; i < PAYLOAD_SIZE; i += 16){
        aes.decryptBlock((uint8_t*)resultDecrypted + i, encrypted + i);
    }
}

//-------------------------------------------//
//-------------  MESSAGE FUNCTION  -----------//
//-----------------------------------------//

String secure_message(const char * msg)
{
        // --- Chiffrement et envoi ---
    LoRaPayload lora = LoRaPayload();
    memset(&lora, 0, sizeof(lora)); // tout à 0
    memcpy(&lora.message, msg, MESSAGE_SIZE); // copier message
    lora.id_device = id_device; // ID de l'end device
    lora.seq_count = seq_count++; // incrémenter le compteur de séquence
    lora.timestamp = now(); // timestamp actuel
    createHMAC((uint8_t*)&lora, lora.hmac, PAYLOAD_SIZE - HMAC_SIZE); // créer HMAC sur le message
    encode_message(payloadEncrypted, (uint8_t*)&lora);//encode le message via aes
    return convertHex(payloadEncrypted, PAYLOAD_SIZE);//conversion en hexa
}

void send_secure_message(String secure_message_str)
{
    LORA_SERIAL.print("AT+TEST=TXLRSTR \"" + secure_message_str  + "\"");//envoie du message
    Serial.println("Message envoyé LoRa : " + secure_message_str);
    blink_led_times(BLINK_COUNT, BLINK_DELAY);
    decode_local_message(secure_message_str);//stockage du message chiffré globalement
}


void decode_local_message(String payloadEncryptedHex)
{
        // --- Test déchiffrement local ---
    Serial.println("\n========== TEST DÉCHIFFREMENT LOCAL ==========");
    uint8_t testEncrypted[PAYLOAD_SIZE];
    LoRaPayload testDecrypted;  // Utiliser la struct, pas char[]
    // 1. Convertir hex → binaire
    hexToBinary(payloadEncryptedHex, testEncrypted, PAYLOAD_SIZE);
    // 2. Déchiffrer dans la struct
    decrypt_message(testEncrypted, &testDecrypted);
    // 3. Afficher les données déchiffrées
    Serial.print("ID déchiffré: ");
    Serial.println(testDecrypted.id_device);
    Serial.print("SEQ déchiffré: ");
    Serial.println(testDecrypted.seq_count);
    Serial.print("TimeStamp déchiffré: ");
    Serial.println(testDecrypted.timestamp);
    Serial.print("Message déchiffré: ");
    for(int i = 0; i < MESSAGE_SIZE && testDecrypted.message[i] != 0; i++) {
        Serial.print((char)testDecrypted.message[i]);
    }
    Serial.println();
    Serial.print("HMAC reçu (premiers 8 octets): ");
    printHex(testDecrypted.hmac, 8);
    // 4. Vérifier le HMAC (passer toute la struct)
    if (verifyHMAC((uint8_t*)&testDecrypted, testDecrypted.hmac, PAYLOAD_SIZE - HMAC_SIZE)) {
        Serial.println(" HMAC VALIDE - Message authentique ");
    } else {
        Serial.println("HMAC INVALIDE - Message falsifié !");
    }
    Serial.println("==============================================\n");
}