#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <esp_now.h>
#include <WiFi.h>

#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);

#define SS_PIN 5
#define RST_PIN 27

#define BUTTON_PIN 36
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 


uint8_t Main_Address[] = {0xE8, 0xDB, 0x84, 0x00, 0xFC, 0xD4};


// Phase
const int SETUP = 0;
const int PLAYING = 1;
const int SENDING = 2;
const int FASTEST = 3;
const int CHALLENGE = 4;
const int FASTEST_CHALLENGE = 5;
const int WAIT_CHALLENGE = 6;
const int END_ROUND = 7;
const int END = 8;

const int RESET = -1;


typedef struct player_message_in {
  int round;
  int phase; // play, CHALLENGE, wait, fastest
  int score;
} player_message_in;

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

player_message_in playerData_in;
message_out data_out;

int p_phase = SETUP;

esp_now_peer_info_t peerInfo;


void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&playerData_in, incomingData, sizeof(playerData_in));

  Serial.print("Round: ");
  Serial.println(playerData_in.round);

  Serial.print("Phase: ");
  Serial.println(playerData_in.phase);

  Serial.print("Score: ");
  Serial.println(playerData_in.score);

  p_phase = playerData_in.phase;
  
  display.display();
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0,0); 
  display.print("score : ");
  display.print(playerData_in.score);
  display.display();

  if (p_phase == FASTEST)
  {
    display.display();
    display.setCursor(0,20); 
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.print("Fastest");
    display.display();

  } else if (p_phase == CHALLENGE) {
    display.display();
    display.setCursor(0,20); 
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.print("Pressed to challenge");

    display.display();

  } else if (p_phase == FASTEST_CHALLENGE) {
    display.display();
    display.setCursor(0,20); 
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.print("You challenge!");

    display.display();

  } else if (p_phase == WAIT_CHALLENGE) {
    display.display();
    display.setCursor(0,20); 
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.print("Wait for challenger..");

    display.display();

  } else if (p_phase == RESET) {
    ESP.restart();
  }
  
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}



int count = 0;
int scan_number = 0;
int lastState = 0;
int currentState;

void setup() {
  
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)

  // Clear the buffer.
  display.clearDisplay();
  display.display();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0); 
  display.print("Pressed button");
  display.setCursor(0,10); 
  display.print("to ready.");
  display.display();

  pinMode(BUTTON_PIN, INPUT);

  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } else {
    Serial.println("Success initializing ESP-NOW");
  }

  esp_now_register_recv_cb(OnDataRecv);

  esp_now_register_send_cb(OnDataSent);

  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Register peer & Add peer
  memcpy(peerInfo.peer_addr, Main_Address, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}



void loop() {
    if (p_phase == SETUP) {
      if (readButton() == 1) {
        send_to_main();
        display.clearDisplay();
        display.display();
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0,0); 
        display.print("Wait for other...");
        display.display();
      }
    }
  

  if (p_phase == PLAYING) {
    if (rfid.PICC_IsNewCardPresent())
        readRFID();
    if (readButton() == 1) {
      if (scan_number != 0) {
        send_to_main();
        scan_number = 0;
        p_phase++;
        display.display();
        display.fillRect(0, 17, 26, 14, BLACK);
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0,20); 
        display.print("Sending...");
        display.display();
      }
    }
  }


  if (p_phase == CHALLENGE) {
    if (readButton() == 1) {
      send_to_main();
    }
  }


  if (p_phase == FASTEST_CHALLENGE) {
    
  }
  

  delay(100);
}



bool readButton() {
  int currentState = digitalRead(BUTTON_PIN);
  if (lastState == 0 && currentState == 1) {
    Serial.println("Button pressed");
    lastState = currentState;
    return 1;
  }
  lastState = currentState;
  return 0;
}



void readRFID()
{
    if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
      count++;
      if (count == 5) {
        display.display();
        display.setTextColor(BLACK);
        display.setTextSize(1);
        display.setCursor(0,20); 
        display.fillRect(0, 20, 35, 8, BLACK);
        display.display();
        scan_number = 0;
        count = 1;
      }

      String uid = "";
      for (int i = 0; i < rfid.uid.size; i++) {
        uid.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
        uid.concat(String(rfid.uid.uidByte[i], HEX));
      }
      uid.toUpperCase();
      Serial.print(uid);
      int scan_num = 0;
      if (uid == " 33 7B A6 AC")
        scan_num = 1;
      else if (uid == " 04 07 4E 68 73 00 00")
        scan_num = 2;
      else if (uid == " 04 46 26 75 73 00 00")
        scan_num = 3;
      else if (uid == " 04 BB 00 68 73 00 00")
        scan_num = 4;
      else if (uid == " 04 05 79 73 73 00 00")
        scan_num = 5;
      else if (uid == " 04 43 30 73 73 00 00")
        scan_num = 6;
      else if (uid == " 04 0D 1A 68 73 00 00")
        scan_num = 9;
      Serial.print(" count : ");
      Serial.print(count);
      Serial.print(" number : ");
      Serial.print(scan_num);
      Serial.println();

      scan_number = scan_num + (scan_number * 10);
      

      display.display();
      display.setTextColor(WHITE); // or BLACK);
      display.setTextSize(1);
      if (count > 1) {
        display.fillRect(8*(count - 2), 20, 6, 8, WHITE);
      }
      display.setCursor(8*(count-1),20);
      display.print(scan_num);
      display.display();


      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD

    }
  
}



void send_to_main() {

  if (p_phase == SETUP) {
    data_out.p2_is_active = true;
    data_out.p2_number = 0;
    data_out.p2_is_challenge = 0;
  }

  if (p_phase == PLAYING) {
    data_out.p2_is_active = false;
    data_out.p2_number = scan_number;
    data_out.p2_is_challenge = 0;
  }

  if (p_phase == CHALLENGE) {
    data_out.p2_is_active = false;
    data_out.p2_number = 0;
    data_out.p2_is_challenge = 1;
  }

  esp_err_t result_main = esp_now_send(Main_Address, (uint8_t *) &data_out, sizeof(data_out));

  if (result_main == ESP_OK) {
      Serial.println("Sent to Main with success");
  }
  else {
    Serial.println("Error sending the data to Main");
  }
}
