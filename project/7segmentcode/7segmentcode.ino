#include <esp_now.h>
#include <WiFi.h>
#include "SevSeg.h"

uint8_t Main_Address[] = {0xE8, 0xDB, 0x84, 0x00, 0xFC, 0xD4};

#define PIN_RED    23 // GIOP23
#define PIN_GREEN  22 // GIOP22
#define PIN_BLUE   21 // GIOP21

typedef struct main_message_in {
  bool do_random;
  int reset;
} main_message_in;


typedef struct message_out {
  // seven segment
  bool is_random;
  int random_number;
  int random_color; // 1 : red, 2 : blue

  // player One
  bool p1_is_active;
  bool p1_is_challenge;
  int p1_number;

  // player Two
  bool p2_is_active;
  bool p2_is_challenge;
  int p2_number;

  // player Three
  bool p3_is_active;
  bool p3_is_challenge;
  int p3_number;

} message_out;

message_out random_num_message;
main_message_in is_ready_to_random;

esp_now_peer_info_t peerInfo;



//7seg
SevSeg sevseg; //Instantiate a seven segment object
hw_timer_t * refreshTimer = NULL;
int displayNum = 0;

int ran_thou = 0;
int ran_hun = 0;
int ran_tens = 0;
int ran_ones = 0;

int SEGMENT_D1 = 16;
int SEGMENT_D2 = 17;
int SEGMENT_D3 = 5;
int SEGMENT_D4 = 18;
int SEGMENT_A = 13;
int SEGMENT_B = 12;
int SEGMENT_C = 14;
int SEGMENT_D = 27;
int SEGMENT_E = 26;
int SEGMENT_F = 25;
int SEGMENT_G = 33;
int SEGMENT_DP = 32;



// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&is_ready_to_random, incomingData, sizeof(is_ready_to_random));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("From main : ");
  Serial.println(is_ready_to_random.do_random);
  Serial.println();

  if (is_ready_to_random.reset == 1) {
    sevseg.setNumber(0);
    analogWrite(PIN_RED,0);
    analogWrite(PIN_GREEN,0);
    analogWrite(PIN_BLUE,0);
  }

  if (is_ready_to_random.do_random) {

    int digit = random(2,5);

    ran_thou = random(1,8);
    ran_hun = random(1,8);
    ran_tens = random(1,8);
    ran_ones = random(1,8);

    if (ran_thou == 7)
      ran_thou = 9;
    if (ran_hun == 7)
      ran_hun = 9;
    if (ran_tens == 7)
      ran_tens = 9;
    if (ran_ones == 7)
      ran_ones = 9;
    
    if (digit <= 3) {
      ran_thou = 0;
      if (digit <= 2) {
        ran_hun = 0;
      }
    }

    displayNum = (ran_thou * 1000) + (ran_hun * 100) + (ran_tens * 10) + (ran_ones);
    sevseg.setNumber(displayNum);

    int color = random(1,3); //1 : red, 2 : blue
    int red = 0;
    int blue = 0;
    int green = 0;
    if (color == 1)
      red = 255;
    else if (color == 2)
      blue = 255;
    analogWrite(PIN_RED,red);
    analogWrite(PIN_GREEN,green);
    analogWrite(PIN_BLUE,blue);

    // strcpy(random_num_message.is_random, "FROM MAIN BOARD");
    random_num_message.is_random = 1;
    if (color == 1) {
      displayNum = (ran_ones * 1000) + (ran_tens * 100) + (ran_hun * 10) + (ran_thou);
      displayNum = displayNum / pow(10,(4-digit));
    }
    Serial.println(displayNum);
    random_num_message.random_number = displayNum; 
    random_num_message.random_color = color;
      esp_err_t result = esp_now_send(Main_Address, (uint8_t *) &random_num_message, sizeof(random_num_message));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
    delay(5000);
  
  }
}

//7segment
void IRAM_ATTR onRefreshTimer(){
  sevseg.refreshDisplay();
}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, Main_Address, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);


  //7segment
  byte numDigits = 4;
  byte digitPins[] = {SEGMENT_D1, SEGMENT_D2, SEGMENT_D3, SEGMENT_D4};
  byte segmentPins[] = {SEGMENT_A, SEGMENT_B, SEGMENT_C, SEGMENT_D, SEGMENT_E, SEGMENT_F, SEGMENT_G};
  bool resistorsOnSegments = true; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_CATHODE; // See README.md for options
  bool updateWithDelays = false; // Default 'false' is Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = false; // Use 'true' if your decimal point doesn't exist or isn't connected. Then, you only need to specify 7 segmentPins[]

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
  updateWithDelays, leadingZeros, disableDecPoint);

  sevseg.setNumber(0);
  sevseg.setBrightness(100);

  refreshTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(refreshTimer, &onRefreshTimer, true);
  timerAlarmWrite(refreshTimer, 1000, true);  // 1000 Hz
  timerAlarmEnable(refreshTimer);
}
 
void loop() {
  
    

  

  
}
