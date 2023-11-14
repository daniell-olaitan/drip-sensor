#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
//#include <BlynkSimpleEsp8266.h>
#include "HX711.h"
/*
#define BLYNK_TEMPLATE_ID "TMPL2jtIIdMBI"
#define BLYNK_TEMPLATE_NAME "Intravenous Monitor"
#define BLYNK_AUTH_TOKEN "lXHQJWT8Ofjiqn3KYEpo2BNbMHvPehOI"

BlynkTimer timer;
char auth[] = BLYNK_AUTH_TOKEN;
*/
// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0X58, 0XBF, 0X25, 0XD7, 0X52, 0X30};

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 1

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 0;
const int LOADCELL_SCK_PIN = 2;

HX711 scale;
int reading;
int lastReading;
// REPLACE WITH YOUR CALIBRATION FACTOR


// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
    int id;
    int x;
    } struct_message;

// Create a struct_message called test to store variables to be sent
struct_message myData;

unsigned long lastTime = 0;
unsigned long timerDelay = 1000;   //5000 before

// Callback when data is sent
//void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("\r\nLast Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } 
  // Set ESP-NOW role
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  // Once ESPNow is successfully init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

//Blynk.begin(auth, ssid, pass);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);   // this value is obtained by calibrating the scale with known weights
  scale.tare();
}
 
void loop() {
  if (scale.wait_ready_timeout(200)) {
    reading = round(scale.get_units());
    Serial.print("Weight: ");
    Serial.println(reading);

    lastReading = reading;

  } else {
    Serial.println("HX711 not found.");
  }
  
  if ((millis() - lastTime) > timerDelay) {
    // Set values to send
    myData.id = BOARD_ID;
    myData.x = lastReading;
    
    // Send message via ESP-NOW
    esp_now_send(0, (uint8_t *) &myData, sizeof(myData));
    lastTime = millis();
  }
//int working = lastReading;
//delay(2000);     //uncommented before
/*  Blynk.run();
  Blynk.virtualWrite(V0,lastReading);
  delay(3000);*/
}