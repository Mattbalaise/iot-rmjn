
#include <network/network.h>
// =====================
// NTP
// =====================
bool syncTime() {
  if (!ntpClient.update()) 
{
    return false;
  }
  baseEpoch = ntpClient.getEpochTime();
  baseMillis = millis();
  return true;
}
uint32_t now() {
  return baseEpoch + (millis() - baseMillis) / 1000;
}


void sync_ntp_clock()
{
    if (millis() - lastResync > 600000UL) {
    if (syncTime()) {
      Serial.println("NTP resynced");
    }
  lastResync = millis();
  }
}

void init_ntp_clock()
{
      // NTP init
  ntpClient.begin();
  delay(500);
  if (syncTime())
  {
    Serial.print("Time synced: ");
    Serial.println(baseEpoch);
  } else {
    Serial.println("NTP sync failed");
  }
}
  // ---------- WiFi ----------
void init_wifi(){
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

void init_lora_config()
{
  delay(1000);
  Serial.println("Initializing LoRa Module...");
  LORA_SERIAL.println("AT");
  LORA_SERIAL.println("AT+RESET");
  delay(1500);
  
  LORA_SERIAL.println("AT+MODE=TEST");
  delay(500);
  String at_rfcfg = String("AT+TEST=RFCFG,") + LORA_FREQ + ",SF" + LORA_SF + ",125,12,15,14,ON,OFF,OFF";
  LORA_SERIAL.println(at_rfcfg);
}

