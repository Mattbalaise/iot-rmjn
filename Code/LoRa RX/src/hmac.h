#ifndef HMAC_H
#define HMAC_H

#include <Arduino.h>
#include <SHA256.h>

// Taille du message et du HMAC
#define MESSAGE_SIZE 32
#define HMAC_SIZE    32

// Clé HMAC partagée
extern char hmacKey[32]; // à définir dans ton main.cpp

//----------------------------------------//
// Fonction pour créer un HMAC
//----------------------------------------//
void createHMAC(const uint8_t* message, uint8_t* hmacOut) {
    SHA256 sha;
    sha.resetHMAC(hmacKey, sizeof(hmacKey));
    sha.update(message, MESSAGE_SIZE); // calcul sur message clair
    sha.finalize(hmacOut, HMAC_SIZE);
}

//----------------------------------------//
// Fonction pour vérifier un HMAC
//----------------------------------------//
bool verifyHMAC(const uint8_t* message, const uint8_t* hmacReceived) {
    uint8_t expectedHMAC[HMAC_SIZE];
    createHMAC(message, expectedHMAC); // recalcule HMAC attendu
    // Comparer octet par octet
    for(int i = 0; i < HMAC_SIZE; i++) {
        if(hmacReceived[i] != expectedHMAC[i]) {
            return false; // HMAC différent → message corrompu
        }
    }

    return true; // Tout correspond → HMAC valide
}
#endif