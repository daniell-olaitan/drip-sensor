#include <ESP8266WiFi.h>
#include <Arduino.h>
#include <espnow.h>


// Define GPIO pins for control
const int s1g = D0;
const int s1r = D1;
const int s2g = D2;
const int s2r = D6;
const int buzz = D8;
const int s3g = 1;
const int s3r = 3;

int space1, space2, sc1, sc2, sp1, sp2;

long p, c;
bool power1, power2;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    int id;
    int x;
    } struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;

// Create an array with all the structures
struct_message boardsStruct[2] = {board1, board2};

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct[myData.id-1].x = myData.x;
  //delay(1000);
  Serial.printf("x value: %d \n", boardsStruct[myData.id-1].x);
  Serial.println();
}
 
void setup() {
  pinMode(s1g, OUTPUT);
  pinMode(s1r, OUTPUT);
  pinMode(s2g, OUTPUT);
  pinMode(s2r, OUTPUT);
  pinMode(s3g, OUTPUT);
  pinMode(s3r, OUTPUT);
  pinMode(buzz, OUTPUT);

  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

  sc1 = boardsStruct[0].x;
  sc2 = boardsStruct[1].x;

  p = millis();
  c = p;

  power1 = true;
  power2 = true;
}

void loop()
{
  c = millis();
  
  // Access the variables for each board
  sp1 = sc1;
  sp2 = sc2;
  
  sc1 = boardsStruct[0].x;
  sc2 = boardsStruct[1].x;

  space1 = sc1;
  space2 = sc2;

  bool space1On = space1 > 3 && space1 < 10;
  bool space2On = space2 > 3 && space2 < 10;

  if (c - p >= 60000) {
    if (sp1 - sc1 <= 3)
      power1 = false;
    else
      power1 = true;

    if (sp2 - sc2 <= 3)
      power2 = false;
    else
      power2 = true;

    p = c;
  }
  
  if (power1) {
    if (space1On) {
      // Turn on the red LED (s1r) and turn off the green LED (s1g)
      digitalWrite(s1g, LOW);
      digitalWrite(s1r, HIGH);
      //digitalWrite(buzz, HIGH);
    } 
  
    else if (space1 >= 11) {
      // Turn on the green LED (s1g) and turn off the red LED (s1r)
      digitalWrite(s1r, LOW);
      digitalWrite(s1g, HIGH);
      //digitalWrite(buzz, LOW);
    }
  }else {
    //digitalWrite(buzz, HIGH);
    digitalWrite(s1g, LOW);
    digitalWrite(s1r, HIGH);
  }

  if (power2) {  
    if (space2On) {
      // Turn on the red LED (s1r) and turn off the green LED (s1g)
      digitalWrite(s2g, LOW);
      digitalWrite(s2r, HIGH);
      //digitalWrite(buzz, HIGH);
    } 
  
    else if (space2 >= 11) {
      // Turn on the green LED (s1g) and turn off the red LED (s1r)
      digitalWrite(s2r, LOW);
      digitalWrite(s2g, HIGH);
      //digitalWrite(buzz, LOW);
    }
  }else {
    //digitalWrite(buzz, HIGH);
    digitalWrite(s2g, LOW);
    digitalWrite(s2r, HIGH);
  }

  if (!power1 || !power2 || (power1 && space1On) || (power2 && space2On))
    digitalWrite(buzz, HIGH);
  else
    digitalWrite(buzz, LOW);
  
  if (space1 < 3) {
    // Turn off both LEDs when b is zero
    digitalWrite(s1r, LOW);
    digitalWrite(s1g, LOW);
    //digitalWrite(buzz, LOW);
  }

  if (space2 < 3) {
    digitalWrite(s2r, LOW);
    digitalWrite(s2g, LOW);
  }
}