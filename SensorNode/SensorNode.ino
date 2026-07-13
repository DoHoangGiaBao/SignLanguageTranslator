/*
  Sensor Node (Glove)
*/

#include <esp_now.h>
#include <WiFi.h>

// Receiver MAC address b4:bf:e9:0e:70:40
uint8_t broadcastAddress[] = {0xB4, 0xBF, 0xE9, 0x0E, 0x70, 0x40};
const uint8_t sensorPins[5] = {32, 33, 34, 35, 36};
const uint8_t batteryPin = 39;
const uint8_t warningLED = 13;
const float alpha = 0.3;
float filteredValues[5] = {0, 0, 0, 0, 0};

struct patternMapping {
  const char* pattern;
  const char* meaning;
};

const patternMapping lookupTable[] {
  {"00000", "0"},
  {"01000", "1"},
  {"01100", "2"}, 
  {"01110", "3"},
  {"01111", "4"},
  {"11111", "5"}
};  

const int tableSize = sizeof(lookupTable) / sizeof(lookupTable[0]);

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

bool batteryWarning() {
  float voltage = (analogRead(batteryPin) * (3.3 / 4095) * 2.0 * 1.23);
  Serial.print("Voltage: ");
  Serial.println(voltage);
  return voltage <= 3.2;
}

void setup() {
  Serial.begin(115200);

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

 esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 6;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  for (int i = 0; i < 5; i++) {
    pinMode(sensorPins[i], INPUT);
  }

  pinMode(batteryPin, INPUT);
  pinMode(warningLED, OUTPUT);
}

void loop() {
  String fingerPattern = "";
  const char* message = "Unknown Sign";
  
  if (batteryWarning()) {
    message = "Battery Is Low!";
    if (digitalRead(warningLED) == LOW) {
      digitalWrite(warningLED, HIGH);
    }
  } else {
    if (digitalRead(warningLED) == HIGH) {
      digitalWrite(warningLED, LOW);
    }

    for (int i = 0; i < 5; i++) {
      int rawValue = analogRead(sensorPins[i]);
      filteredValues[i] = (alpha * rawValue) + ((1.0 - alpha) * filteredValues[i]);

      fingerPattern += (filteredValues[i] < 3900) ? "0" : "1";
    }
    Serial.println("Finger Pattern: " + fingerPattern);

    for (int i = 0; i < tableSize; i++) {
      if (fingerPattern == lookupTable[i].pattern) {
        message = lookupTable[i].meaning;
        break;
      }
    }
  }

  Serial.println("Message: " + String(message));

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) message, strlen(message) + 1);
  
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  delay(200);
}
