#include <esp_now.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>

//P1 status led
#define PIN_RED_1    23 // GIOP23
#define PIN_GREEN_1  27 // GIOP27
#define PIN_BLUE_1   33 // GIOP33
//P2 status led
#define PIN_RED_2    19 // GIOP19
#define PIN_GREEN_2  18 // GIOP18
#define PIN_BLUE_2   25 // GIOP25
//P3 status led
#define PIN_RED_3    17 // GIOP17
#define PIN_GREEN_3  16 // GIOP16
#define PIN_BLUE_3   4 // GIOP4


// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);


uint8_t sevenSegment_Address[] = {0x30, 0xAE, 0xA4, 0x1A, 0x28, 0x20};
uint8_t playerOne_Address[] = {0x3C, 0x61, 0x05, 0x03, 0xD5, 0x10};
uint8_t playerTwo_Address[] = {0x3C, 0x61, 0x05, 0x03, 0xCC, 0x8C};
uint8_t playerThree_Address[] = {0x24, 0x0A, 0xC4, 0x9A, 0xFC, 0x98};



// Structure example to receive data
// Must match the sender structure
typedef struct seven_segment_message_out {
  bool do_random;
} seven_segment_message_out;


typedef struct player_message_out {
  int round;
  int phase; //0 : setup, 1 : playing, 2 : sending, 3 : fastest, 4 : challenge, 5 : fastest challenge, 6 : endround, 7 : end
  int score;
} player_message_out;


typedef struct message_in {
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

} message_in;



// Create a sevenSegment_message called data_in & data_out
seven_segment_message_out segmentData_out;
player_message_out player1_Data_out;
player_message_out player2_Data_out;
player_message_out player3_Data_out;

message_in dataIn;


int p1_number = 0;
int p2_number = 0;
int p3_number = 0;
int mainphase = 0; //0 : setup, 1 : playing, 2 : sending, 3 : fastest, 4 : challenge, 5 : endround, 6 : end
int mainround = 0;
int p1_ready = 0;
int p2_ready = 0;
int p3_ready = 0;
int fastest_player = 0;
int fastest_challenge = 0;


esp_now_peer_info_t peerInfo;


// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&dataIn, incomingData, sizeof(dataIn));

  Serial.print("Is random: ");
  Serial.println(dataIn.is_random);

  Serial.print("Random number: ");
  Serial.println(dataIn.random_number);

  Serial.print("Color: ");
  Serial.println(dataIn.random_color);

  Serial.print("P1 active: ");
  Serial.println(dataIn.p1_is_active);

  Serial.print("P1 number: ");
  Serial.println(dataIn.p1_number);

  Serial.print("P2 active: ");
  Serial.println(dataIn.p2_is_active);

  Serial.print("P2 number: ");
  Serial.println(dataIn.p2_number);

  Serial.print("P3 active: ");
  Serial.println(dataIn.p3_is_active);

  Serial.print("P3 number: ");
  Serial.println(dataIn.p3_number);

  Serial.println();
  if (mainphase == 0 && dataIn.p1_is_active) {
    set_led(1, 0, 255, 0);
    p1_ready = 1;
  }

  // dataIn.p2_is_active = 1;
  if (mainphase == 0 && dataIn.p2_is_active) {
    set_led(2, 0, 255, 0);
    p2_ready = 1;
  }

  // dataIn.p3_is_active = 1;
  if (mainphase == 0 && dataIn.p3_is_active) {
    set_led(3, 0, 255, 0);
    p3_ready = 1;
  }

  if (p1_ready && p2_ready && p3_ready && mainphase == 0) {
    mainphase++;
    mainround++;
    
    fastest_player = 0;
    player1_Data_out.phase = mainphase;
    player1_Data_out.round = mainround;
    player1_Data_out.score = 0;
    esp_err_t result_p1 = esp_now_send(playerOne_Address, (uint8_t *) &player1_Data_out, sizeof(player1_Data_out));
    if (result_p1 == ESP_OK) {
      Serial.println("Sent to P1 with success");
    }
    else {
      Serial.println("Error sending the data to P1");
    }


    player2_Data_out.phase = mainphase;
    player2_Data_out.round = mainround;
    player2_Data_out.score = 0;
    esp_err_t result_p2 = esp_now_send(playerTwo_Address, (uint8_t *) &player2_Data_out, sizeof(player2_Data_out));
    if (result_p2 == ESP_OK) {
      Serial.println("Sent to P2 with success");
    }
    else {
      Serial.println("Error sending the data to P2");
    }

    player3_Data_out.phase = mainphase;
    player3_Data_out.round = mainround;
    player3_Data_out.score = 0;
    esp_err_t result_p3 = esp_now_send(playerThree_Address, (uint8_t *) &player3_Data_out, sizeof(player3_Data_out));
    if (result_p3 == ESP_OK) {
      Serial.println("Sent to P3 with success");
    }
    else {
      Serial.println("Error sending the data to P3");
    }

    delay(500);
    set_led(1, 0, 0, 0);
    set_led(2, 0, 0, 0);
    set_led(3, 0, 0, 0);
    lcd_countdown();
    sendDoRandom();
    lcd_PlayingPhase();
  }


  if (fastest_player == 0) {
    if (dataIn.p1_number != 0) {
      fastest_player = 1;
      lcd_ChallengePhase();
      player1_Data_out.phase = 3;
      player2_Data_out.phase = 4;
      player3_Data_out.phase = 4;
      esp_now_send(playerOne_Address, (uint8_t *) &player1_Data_out, sizeof(player1_Data_out));
      esp_now_send(playerTwo_Address, (uint8_t *) &player2_Data_out, sizeof(player2_Data_out));
      esp_now_send(playerThree_Address, (uint8_t *) &player3_Data_out, sizeof(player3_Data_out));      
      set_led(1, 255, 0, 0);
      // set_timer();
    }
    else if (dataIn.p2_number != 0) {
      fastest_player = 2;
      lcd_ChallengePhase();
      player1_Data_out.phase = 4;
      player2_Data_out.phase = 3;
      player3_Data_out.phase = 4;
      esp_now_send(playerOne_Address, (uint8_t *) &player1_Data_out, sizeof(player1_Data_out));
      esp_now_send(playerTwo_Address, (uint8_t *) &player2_Data_out, sizeof(player2_Data_out));
      esp_now_send(playerThree_Address, (uint8_t *) &player3_Data_out, sizeof(player3_Data_out));
      set_led(2, 255, 0, 0);
      // set_timer();

    }
    else if (dataIn.p3_number != 0) {
      fastest_player = 3;
      lcd_ChallengePhase();
      player1_Data_out.phase = 4;
      player2_Data_out.phase = 4;
      player3_Data_out.phase = 3;
      esp_now_send(playerOne_Address, (uint8_t *) &player1_Data_out, sizeof(player1_Data_out));
      esp_now_send(playerTwo_Address, (uint8_t *) &player2_Data_out, sizeof(player2_Data_out));
      esp_now_send(playerThree_Address, (uint8_t *) &player3_Data_out, sizeof(player3_Data_out));
      set_led(3, 255, 0, 0);
      // set_timer();
    }
  }
  if (fastest_challenge == 0) {
    if (dataIn.p1_is_challenge && fastest_player != 1)
    {
      set_led(1, 0, 0, 255);
      set_led(fastest_player, 0, 0, 255);
      player1_Data_out.phase = 5;
      fastest_challenge = 1;     
    }
    else if (dataIn.p2_is_challenge && fastest_player != 2)
    {
      set_led(2, 0, 0, 255);
      set_led(fastest_player, 0, 0, 255);
      player2_Data_out.phase = 5;
      fastest_challenge = 2;
    }
    else if (dataIn.p3_is_challenge && fastest_player != 3)
    {
      set_led(3, 0, 0, 255);
      set_led(fastest_player, 0, 0, 255);
      player3_Data_out.phase = 5;
      fastest_challenge = 3;
      
    }
  }


}




void set_lcd() {
  lcd.clear();
  // set cursor to first column, first row
  lcd.setCursor(0, 0);
  // print message
  lcd.print(dataIn.random_number);

  lcd.setCursor(0, 1);
  lcd.print(dataIn.p1_number);
  
}

void set_led(int player, int red, int green, int blue) {
  if (player == 1) {
    //P1 status led
    analogWrite(PIN_RED_1,red);
    analogWrite(PIN_GREEN_1,green);
    analogWrite(PIN_BLUE_1,blue);
  }
  else if (player == 2) {
    //P2 status led
    analogWrite(PIN_RED_2,red);
    analogWrite(PIN_GREEN_2,green);
    analogWrite(PIN_BLUE_2,blue);
  }
  else if (player == 3) {
    //P3 status led
    analogWrite(PIN_RED_3,red);
    analogWrite(PIN_GREEN_3,green);
    analogWrite(PIN_BLUE_3,blue);
  }
}
 


void setup() {
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
  
  // Initialize Serial Monitor
  Serial.begin(9600);

  segmentData_out.do_random = false;

  lcd.clear();
  lcd.setCursor(0, 0);
  // print message
  lcd.print("Numerito Game");

  lcd.setCursor(0, 1);
  lcd.print("Wait to start...");

  //P1 status led
  pinMode(PIN_RED_1,   OUTPUT);
  pinMode(PIN_GREEN_1, OUTPUT);
  pinMode(PIN_BLUE_1,  OUTPUT);
  
  //P2 status led
  pinMode(PIN_RED_2,   OUTPUT);
  pinMode(PIN_GREEN_2, OUTPUT);
  pinMode(PIN_BLUE_2,  OUTPUT);
  
  //P1 status led
  pinMode(PIN_RED_3,   OUTPUT);
  pinMode(PIN_GREEN_3, OUTPUT);
  pinMode(PIN_BLUE_3,  OUTPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Register peer & Add peer
  memcpy(peerInfo.peer_addr, sevenSegment_Address, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  memcpy(peerInfo.peer_addr, playerOne_Address, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  memcpy(peerInfo.peer_addr, playerTwo_Address, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  memcpy(peerInfo.peer_addr, playerThree_Address, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  
  
  
}


void loop() {
  
  if (mainphase == 4)
  {
    esp_err_t result_p1 = esp_now_send(playerOne_Address, (uint8_t *) &player1_Data_out, sizeof(player1_Data_out));
    if (result_p1 == ESP_OK) {
      Serial.println("Sent to P1 with success");
    }
    else {
      Serial.println("Error sending the data to P1");
    }
  }
  // esp_err_t result_p2 = esp_now_send(playerTwo_Address, (uint8_t *) &player2_Data_out, sizeof(player2_Data_out));

  // esp_err_t result_p3 = esp_now_send(playerThree_Address, (uint8_t *) &player3_Data_out, sizeof(player3_Data_out));
  // if (fastest_player != 0) {
  //   int start_time = millis();
  //   for (int i=0; i < millis()-start_time; i++) {
      
  //   }
  //   set_led(fastest_player, 0, 255 ,0);
  // }
  delay(100);
}

void lcd_countdown() {
  for (int i = 5; i > 0; i--) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(i);
    delay(1000);
  }
}

void lcd_PlayingPhase() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Round : ");
  lcd.print(mainround);
  lcd.setCursor(0, 1);
  lcd.print("Phase : Playing");
}

void lcd_ChallengePhase() {
  lcd.setCursor(0, 1);
  lcd.print("Challenge P");
  lcd.print(fastest_player);
  lcd.print("?  ");
}

void sendDoRandom() {
  segmentData_out.do_random = true;
  esp_err_t result_segment = esp_now_send(sevenSegment_Address, (uint8_t *) &segmentData_out, sizeof(segmentData_out));
  if (result_segment == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
}