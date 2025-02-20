#include "esp_sleep.h"
#include "ESP32_NOW.h"
#include "WiFi.h"
#include <esp_mac.h>  

/* Definitions */

#define ESPNOW_WIFI_CHANNEL 6
#define BUTTON_GPIO 5  // Change to the GPIO where your button is connected


/* Global Variables */

uint32_t msg_count = 0;


/* Classes */

class ESP_NOW_Broadcast_Peer : public ESP_NOW_Peer {
public:
  // Constructor of the class using the broadcast address
  ESP_NOW_Broadcast_Peer(uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) : ESP_NOW_Peer(ESP_NOW.BROADCAST_ADDR, channel, iface, lmk) {}

  // Destructor of the class
  ~ESP_NOW_Broadcast_Peer() {
    remove();
  }

  // Function to properly initialize the ESP-NOW and register the broadcast peer
  bool begin() {
    if (!ESP_NOW.begin() || !add()) {
      log_e("Failed to initialize ESP-NOW or register the broadcast peer");
      return false;
    }
    return true;
  }

  // Function to send a message to all devices within the network
  bool send_message(const uint8_t *data, size_t len) {
    if (!send(data, len)) {
      log_e("Failed to broadcast message");
      return false;
    }
    return true;
  }
};


// Create a broadcast peer object
ESP_NOW_Broadcast_Peer broadcast_peer(ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, NULL);


void setup() {

  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

 
  // Initialize Wi-Fi 
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  while (!WiFi.STA.started()) {
    delay(100);
  }

  Serial.println("ESP-NOW Example - Broadcast Master");
  Serial.println("Wi-Fi parameters:");
  Serial.println("  Mode: STA");
  Serial.println("  MAC Address: " + WiFi.macAddress());
  Serial.printf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

  // Register the broadcast peer
  if (!broadcast_peer.begin()) {
    Serial.println("Failed to initialize broadcast peer");
    Serial.println("Reebooting in 5 seconds...");
    delay(1000);
    ESP.restart();
  }


  // Configure the button GPIO as an input with pull-up resistor
  pinMode(BUTTON_GPIO, INPUT_PULLUP);

  // Configure wake-up source: GPIO on button press (LOW signal)
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_GPIO, LOW);


  //prepare first message
  char data[32];
  snprintf(data, sizeof(data), "button pressed #%lu", msg_count++);
  Serial.printf("Broadcasting message: %s\n", data);

  //send firt message
  if (!broadcast_peer.send_message((uint8_t *)data, sizeof(data))) {
    Serial.println("Failed to broadcast message");
  }


  delay(1000);

  // Enter deep sleep mode
  Serial.println("Going to sleep now...");
  esp_deep_sleep_start();
}

void loop() {
    // ESP32 will never reach this loop because it will be in deep sleep
}
