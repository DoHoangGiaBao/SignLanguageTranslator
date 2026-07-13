/*
  CYD Hub (Text Display)
*/

#include <esp_now.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <TFT_eSPI.h>

char receivedMessage[32] = "";

TFT_eSPI tft = TFT_eSPI();

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memset(receivedMessage, 0, sizeof(receivedMessage));

  memcpy(receivedMessage, incomingData, len);
  Serial.println(receivedMessage);
  drawDynamic();
}

void drawStatic() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("Sign Language Translator", TFT_HEIGHT / 2, 10, 4);
  tft.drawFastHLine(10, 40, TFT_HEIGHT - 20, TFT_LIGHTGREY);
  tft.drawCentreString("Received Message", TFT_HEIGHT / 2, 50, 4);
}

void drawDynamic() {
  tft.fillRect(0, 80, TFT_HEIGHT, 30, TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.drawCentreString(receivedMessage, TFT_HEIGHT / 2, 80, 4);
}

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  drawStatic();
  drawDynamic();

  WiFi.mode(WIFI_STA);

  #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    WiFi.setChannel(6); 
  #else
    esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);
  #endif

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop() {
  
}