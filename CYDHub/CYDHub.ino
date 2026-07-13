/*
  CYD Hub (Text Display)
*/
#define BLYNK_TEMPLATE_ID "TMPL69_5WFYrO"
#define BLYNK_TEMPLATE_NAME "SignedLanguageTranslator"
#define BLYNK_AUTH_TOKEN "HHSUMesuPI3MHJkrmUubgP5zZFgbFajB"

#include <esp_now.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TFT_eSPI.h>

char ssid[] = "iPhone";
char pass[] = "159951753357";
char receivedMessage[32] = "";

TFT_eSPI tft = TFT_eSPI();

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memset(receivedMessage, 0, sizeof(receivedMessage));

  memcpy(receivedMessage, incomingData, len);
  Serial.println(receivedMessage);
  Blynk.virtualWrite(V0, receivedMessage);
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

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {
  Blynk.run();
}