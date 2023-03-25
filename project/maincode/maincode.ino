#include <esp_now.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <ezButton.h>

#define DEBOUNCE_TIME 50
ezButton button(5);

//P1 status led
#define PIN_RED_1        23 // GIOP23
#define PIN_GREEN_1    27 // GIOP27
#define PIN_BLUE_1     33 // GIOP33
//P2 status led
#define PIN_RED_2        19 // GIOP19
#define PIN_GREEN_2    18 // GIOP18
#define PIN_BLUE_2     25 // GIOP25
//P3 status led
#define PIN_RED_3        17 // GIOP17
#define PIN_GREEN_3    16 // GIOP16
#define PIN_BLUE_3     4 // GIOP4


// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;


// Phase
const int SETUP = 0;
const int PLAYING = 1;
const int SENDING = 2;
const int FASTEST = 3;
const int CHALLANGE = 4;
const int FASTEST_CHALLENGE = 5;
const int WAIT_CHALLENGE = 6;
const int END_ROUND = 7;
const int END = 8;

const int RESET = -1;


// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

// E8:68:E7:22:B6:A8
uint8_t sevenSegment_Address[] = {0x30, 0xAE, 0xA4, 0x1A, 0x28, 0x20};
uint8_t playerOne_Address[] = {0xE8, 0x68, 0xE7, 0x22, 0xB6, 0xA8};
uint8_t playerTwo_Address[] = {0x3C, 0x61, 0x05, 0x03, 0xCC, 0x8C};
uint8_t playerThree_Address[] = {0x24, 0x0A, 0xC4, 0x9A, 0xFC, 0x98};



// Structure to receive data
// Must match the sender structure
typedef struct seven_segment_message_out {
    bool do_random;
} seven_segment_message_out;


typedef struct player_message_out {
    int round;
    int phase;
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



int mainphase = SETUP;
int mainround = 0;
int random_number = 0;

int selecter_player_number;

int p1_ready = 0;
int p2_ready = 0;
int p3_ready = 0;

int fastest_player = 0;
int fastest_challenge_player = 0;
int winner = 0;

int player_score[4];



// About Time;
double t1;
double t2;
double t;


esp_now_peer_info_t peerInfo;




// Sent & Recv ===============================================================================

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    memcpy(&dataIn, incomingData, sizeof(dataIn));

    Serial.print("Is random: ");        Serial.println(dataIn.is_random);
    Serial.print("Random number: ");    Serial.println(dataIn.random_number);
    Serial.print("Color: ");            Serial.println(dataIn.random_color);
    Serial.print("P1 active: ");        Serial.println(dataIn.p1_is_active);
    Serial.print("P1 number: ");        Serial.println(dataIn.p1_number);
    Serial.print("P2 active: ");        Serial.println(dataIn.p2_is_active);
    Serial.print("P2 number: ");        Serial.println(dataIn.p2_number);
    Serial.print("P3 active: ");        Serial.println(dataIn.p3_is_active);
    Serial.print("P3 number: ");        Serial.println(dataIn.p3_number);
    Serial.println();
}

// ===========================================================================================




// LCD Function ==============================================================================

void lcd_countdown(int x) {
    for (int i = x; i > 0; i--) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(i);
        delay(1000);
    }
}

void lcd_ManualCountdown(int x, String message) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
    lcd.setCursor(0, 1);
    lcd.print(x);
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
    lcd.print("?    ");
}

void lcd_NumberMatch() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Challanger wrong :(");
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("-1 challanger");
    lcd.setCursor(0, 1);
    lcd.print("+1 to right.");
}

void lcd_NumberWrong() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Challanger right !");
    delay(3000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("+1 challanger score");
    lcd.setCursor(0, 1);
    lcd.print("-1 to lie.");
}

void lcd_TimeoutChallenge() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Challenge");
    lcd.setCursor(0, 1);
    lcd.print("Timeout..");
}

void lcd_EndRound() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("End Round/Score ");
    lcd.print(mainround);
    lcd.setCursor(0, 1);
    lcd.print("P1:");
    lcd.print(player_score[1]);
    lcd.print(" P2:");
    lcd.print(player_score[2]);
    lcd.print(" P3:");
    lcd.print(player_score[3]);
}

void lcd_EndGame() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Game End");
    lcd.setCursor(0, 1);
    lcd.print("Winner is P");
    lcd.print(winner);
    lcd.print(" :)");
}

// ===========================================================================================




// LED Function ==============================================================================

void set_led(int player, int red, int green, int blue) {
    if (player == 1) {
        //P1 status led
        analogWrite(PIN_RED_1, red);
        analogWrite(PIN_GREEN_1, green);
        analogWrite(PIN_BLUE_1, blue);
    } else if (player == 2) {
        //P2 status led
        analogWrite(PIN_RED_2, red);
        analogWrite(PIN_GREEN_2, green);
        analogWrite(PIN_BLUE_2, blue);
    } else if (player == 3) {
        //P3 status led
        analogWrite(PIN_RED_3, red);
        analogWrite(PIN_GREEN_3, green);
        analogWrite(PIN_BLUE_3, blue);
    }
}

// ===========================================================================================




// Send ESP-Now Function =====================================================================

void SendDoRandom() {
    segmentData_out.do_random = true;
    esp_err_t result_segment = esp_now_send(sevenSegment_Address, (uint8_t *) &segmentData_out, sizeof(segmentData_out));
    if (result_segment == ESP_OK) {
        Serial.println("Sent with success");
    } else {
        Serial.println("Error sending the data");
    }
}


void SendUpdateDataToPlayer() {
    esp_now_send(playerOne_Address, (uint8_t *) &player1_Data_out, sizeof(player1_Data_out));
    esp_now_send(playerTwo_Address, (uint8_t *) &player2_Data_out, sizeof(player2_Data_out));
    esp_now_send(playerThree_Address, (uint8_t *) &player3_Data_out, sizeof(player3_Data_out));
}

// ===========================================================================================




// Setup  ====================================================================================

void setup() {
    // initialize LCD
    lcd.init();
    // turn on LCD backlight                                            
    lcd.backlight();
    
    // Initialize Serial Monitor
    Serial.begin(9600);

    segmentData_out.do_random = false;

    button.setDebounceTime(DEBOUNCE_TIME);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Numerito Game");

    lcd.setCursor(0, 1);
    lcd.print("Wait to start...");

    //P1 status led
    pinMode(PIN_RED_1,     OUTPUT);
    pinMode(PIN_GREEN_1, OUTPUT);
    pinMode(PIN_BLUE_1,    OUTPUT);
    
    //P2 status led
    pinMode(PIN_RED_2,     OUTPUT);
    pinMode(PIN_GREEN_2, OUTPUT);
    pinMode(PIN_BLUE_2,    OUTPUT);
    
    //P1 status led
    pinMode(PIN_RED_3,     OUTPUT);
    pinMode(PIN_GREEN_3, OUTPUT);
    pinMode(PIN_BLUE_3,    OUTPUT);

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);
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

    for (int i = 0; i<4 ; i++){
        player_score[i] = 0 ;
    }

    player1_Data_out.phase = RESET;
    player2_Data_out.phase = RESET;
    player3_Data_out.phase = RESET;

    SendUpdateDataToPlayer();
}

// ===========================================================================================




// Loop ======================================================================================

void loop() {
  //reset button
  button.loop(); // MUST call the loop() function first

  if (button.isPressed())
    ESP.restart();

  // if (button.isReleased())
  //   Serial.println("The button is released");

    if (mainphase == SETUP && dataIn.p1_is_active) {
        set_led(1, 0, 255, 0);
        p1_ready = 1;
    }

    // dataIn.p2_is_active = 1;
    if (mainphase == SETUP && dataIn.p2_is_active) {
        set_led(2, 0, 255, 0);
        p2_ready = 1;
    }

    // dataIn.p3_is_active = 1;
    if (mainphase == SETUP && dataIn.p3_is_active) {
        set_led(3, 0, 255, 0);
        p3_ready = 1;
    }


    // All player ready to play.
    if ( mainphase == SETUP && p1_ready && p2_ready && p3_ready ) {
        mainphase = PLAYING;
        mainround += 1;
        
        fastest_player = 0;
        player1_Data_out.phase = mainphase;
        player1_Data_out.round = mainround;
        player1_Data_out.score = player_score[1]; // Just test !!!

        player2_Data_out.phase = mainphase;
        player2_Data_out.round = mainround;
        player2_Data_out.score = player_score[2]; // Just test !!!

        player3_Data_out.phase = mainphase;
        player3_Data_out.round = mainround;
        player3_Data_out.score = player_score[3]; // Just test !!!
       
        SendUpdateDataToPlayer();

        delay(500);
        set_led(1, 0, 0, 0);
        set_led(2, 0, 0, 0);
        set_led(3, 0, 0, 0);
        lcd_countdown(5);
        SendDoRandom();     
        lcd_PlayingPhase();
    }

    if (dataIn.random_number != 0 && random_number != dataIn.random_number) {
        random_number = dataIn.random_number;
        Serial.printf("random_number: %d\n", random_number);
    }


    // Fastest player found.
    if ( mainphase == PLAYING && fastest_player == 0 ) {
        if (dataIn.p1_number != 0) {
            fastest_player = 1;
            selecter_player_number = dataIn.p1_number;

            mainphase = CHALLANGE;
            lcd_ChallengePhase();
            t1 = millis();

            player1_Data_out.phase = FASTEST;
            player2_Data_out.phase = CHALLANGE;
            player3_Data_out.phase = CHALLANGE;
            SendUpdateDataToPlayer();          
            set_led(1, 255, 0, 0);
           
        } else if (dataIn.p2_number != 0) {
            fastest_player = 2;
            selecter_player_number = dataIn.p2_number;

            mainphase = CHALLANGE;
            lcd_ChallengePhase();
            t1 = millis();

            player1_Data_out.phase = CHALLANGE;
            player2_Data_out.phase = FASTEST;
            player3_Data_out.phase = CHALLANGE;
            SendUpdateDataToPlayer();
            set_led(2, 255, 0, 0);

        } else if (dataIn.p3_number != 0) {
            fastest_player = 3;
            selecter_player_number = dataIn.p3_number;

            mainphase = CHALLANGE;
            lcd_ChallengePhase();
            t1 = millis();

            player1_Data_out.phase = CHALLANGE;
            player2_Data_out.phase = CHALLANGE;
            player3_Data_out.phase = FASTEST;
            SendUpdateDataToPlayer();
            set_led(3, 255, 0, 0);
            // set_timer();
        }
    }

    
    if ( mainphase == CHALLANGE ) {
        t2 = millis();
        t = t2 - t1;

        // If having challanger !
        if (t <= 10000) {
            lcd_ManualCountdown(10 - t/1000, "Challenging time!");
            if (dataIn.p1_is_challenge && fastest_player != 1) {
                fastest_challenge_player = 1;
                
                set_led(1, 0, 0, 255);
                set_led(fastest_player, 0, 0, 255);

                player1_Data_out.phase = FASTEST_CHALLENGE;
                if (fastest_player == 2)
                    player3_Data_out.phase = WAIT_CHALLENGE;
                else if (fastest_player == 3)
                    player2_Data_out.phase = WAIT_CHALLENGE;

                SendUpdateDataToPlayer();

                mainphase = FASTEST_CHALLENGE;
                        
            } else if (dataIn.p2_is_challenge && fastest_player != 2) {
                fastest_challenge_player = 2;
                
                set_led(2, 0, 0, 255);
                set_led(fastest_player, 0, 0, 255);

                player2_Data_out.phase = FASTEST_CHALLENGE;
                if (fastest_player == 1)
                    player3_Data_out.phase = WAIT_CHALLENGE;
                else if (fastest_player == 3)
                    player1_Data_out.phase = WAIT_CHALLENGE;

                SendUpdateDataToPlayer();

                mainphase = FASTEST_CHALLENGE;
                
            } else if (dataIn.p3_is_challenge && fastest_player != 3) {
                fastest_challenge_player = 3;

                set_led(3, 0, 0, 255);
                set_led(fastest_player, 0, 0, 255);

                player3_Data_out.phase = FASTEST_CHALLENGE;
                if (fastest_player == 2)
                    player1_Data_out.phase = WAIT_CHALLENGE;
                else if (fastest_player == 1)
                    player2_Data_out.phase = WAIT_CHALLENGE;
                
                SendUpdateDataToPlayer();

                mainphase = FASTEST_CHALLENGE;
            }

        } else {
            lcd_TimeoutChallenge();
            player_score[fastest_player] += 1;

            
            delay(5000);
            
            mainphase = END_ROUND;
        }
    }


    if ( mainphase == FASTEST_CHALLENGE ) {

        Serial.printf("player: %d, correct: %d\n", selecter_player_number,random_number);

        if (selecter_player_number == random_number) {
            player_score[fastest_player] += 1;
            Serial.printf("fastest: %d, score: %d\n", fastest_player, player_score[fastest_player]);

            if(player_score[fastest_challenge_player] > 0) 
                player_score[fastest_challenge_player] -= 1;

            lcd_NumberMatch();
            delay(3000);
        } else {
            player_score[fastest_challenge_player] += 1;

            if(player_score[fastest_player] > 0) 
                player_score[fastest_player] -= 1;

            lcd_NumberWrong();
            delay(3000);
        }

        player1_Data_out.score = player_score[1];
        player2_Data_out.score = player_score[2];
        player3_Data_out.score = player_score[3];

        player1_Data_out.phase = END_ROUND;
        player2_Data_out.phase = END_ROUND;
        player3_Data_out.phase = END_ROUND;

        SendUpdateDataToPlayer();

        mainphase = END_ROUND;
    }


    if ( mainphase == END_ROUND ) {
        lcd_EndRound();
        delay(5000);

        for (int i=1 ; i<4 ; i++) {
            if (player_score[i] >= 3) {
                player1_Data_out.phase = END;
                player2_Data_out.phase = END;
                player3_Data_out.phase = END;

                SendUpdateDataToPlayer();
                winner = i;
                mainphase = END;

                break;

            } else {
                player1_Data_out.phase = SETUP;
                player2_Data_out.phase = SETUP;
                player3_Data_out.phase = SETUP;

                SendUpdateDataToPlayer();

                fastest_player = 0;
                fastest_challenge_player = 0;

                mainphase = SETUP;
            }
        }

        
    }

    if ( mainphase == END ){
        lcd_EndGame();
        delay(10000);
        ESP.restart();
    }

    
    delay(100);
}
