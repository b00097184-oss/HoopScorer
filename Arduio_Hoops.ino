#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <string.h>


// LCD CONFIGURATION
LiquidCrystal_I2C lcd(0x27, 16, 2);  



SoftwareSerial zigbee(A2, A0);  // RX, TX


// PINS
#define BTN_PIN A3      
#define BTN_LED A1      


#define TRIG1 2
#define ECHO1 3
#define LED1 9    


#define TRIG2 5
#define ECHO2 4
#define LED2 11    


#define TRIG3 7
#define ECHO3 6
#define LED3 10    


// BUZZER
#define BUZZER 12



#define HOOP1_THRESHOLD 10  
#define HOOP2_THRESHOLD 5  
#define HOOP3_THRESHOLD 3.5  




unsigned long gameStartTime;
bool gameRunning = false;
unsigned long lastLCDTime = 0;



int roundDurationSec = 30;



int h1Score = 0;
int h2Score = 0;
int h3Score = 0;



bool hoop1Active = false;
bool hoop2Active = false;
bool hoop3Active = false;



char player1[17] = "Player1";   
char player2[17] = "Player2";   
int numPlayers = 0;        
int currentPlayer = 1;         



int roundsPerPlayer = 1;
int remainingRoundsP1 = 0;
int remainingRoundsP2 = 0;
bool gameFinished = false;



char zigbeeBuffer[64];
byte zigbeeIndex = 0;



float readUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 15000);
  if (duration == 0) return 999;
  return (duration * 0.0343) / 2;
}



char* ltrim(char* s) {
  while (*s == ' ' || *s == '\t') s++;
  return s;
}



const char* getCurrentPlayerName() {
  if (currentPlayer == 1) return player1;
  if (currentPlayer == 2 && numPlayers >= 2) return player2;
  return "Player";
}


// print player line on LCD 
void printPlayerLine() {
  const char* name = getCurrentPlayerName();
  lcd.setCursor(0, 0);
  lcd.print("P");
  lcd.print(currentPlayer);
  lcd.print(": ");
  // Print name and pad to clear rest of line
  int i = 0;
  while (name[i] != '\0' && i < 12) { // 16 cols: "P#:" + space + 12 chars = 15
    lcd.print(name[i]);
    i++;
  }
  while (i < 12) {
    lcd.print(" ");
    i++;
  }
}


void processZigbeeCommand(char* cmd) {


  Serial.print("ZB CMD: ");
  Serial.println(cmd);


  if (strncmp(cmd, "SETUP:", 6) != 0) {
    return;
  }


  char* p = cmd + 6;


  // Tokenize by ':'
  char* t1 = strtok(p, ":");      
  char* t2 = strtok(NULL, ":");   
  char* t3 = strtok(NULL, ":");   
  char* t4 = strtok(NULL, ":");   


  // Reset state when new setup arrives
  gameRunning = false;
  gameFinished = false;
  numPlayers = 0;
  currentPlayer = 1;
  remainingRoundsP1 = 0;
  remainingRoundsP2 = 0;
  roundsPerPlayer = 1;
  roundDurationSec = 30; 



  if (t1 && t2 && t3 && t4) {
    int timeFlag = atoi(t1);
    roundsPerPlayer = atoi(t2);
    if (roundsPerPlayer <= 0) roundsPerPlayer = 1;



    if (timeFlag == 0) roundDurationSec = 15;
    else roundDurationSec = 30;


    char* mode = t3;
    char* players = t4;


    if (strcmp(mode, "MULTI") == 0) {
      char* comma = strchr(players, ',');
      if (!comma) return;


      *comma = '\0';
      char* p1 = ltrim(players);
      char* p2 = ltrim(comma + 1);


      strncpy(player1, p1, 16);
      player1[16] = '\0';
      strncpy(player2, p2, 16);
      player2[16] = '\0';


      numPlayers = 2;
      currentPlayer = 1;
      remainingRoundsP1 = roundsPerPlayer;
      remainingRoundsP2 = roundsPerPlayer;


      Serial.print("Setup NEW MULTI T=");
      Serial.print(roundDurationSec);
      Serial.print(" R=");
      Serial.print(roundsPerPlayer);
      Serial.print(" P1=");
      Serial.print(player1);
      Serial.print(" P2=");
      Serial.println(player2);
    } else if (strcmp(mode, "SINGLE") == 0) {
      char* p1 = ltrim(players);
      strncpy(player1, p1, 16);
      player1[16] = '\0';


      numPlayers = 1;
      currentPlayer = 1;
      remainingRoundsP1 = roundsPerPlayer;


      Serial.print("Setup NEW SINGLE T=");
      Serial.print(roundDurationSec);
      Serial.print(" R=");
      Serial.print(roundsPerPlayer);
      Serial.print(" P1=");
      Serial.println(player1);
    }
  }

  else if (t1 && t2 && !t3) {

    char* mode = t1;
    char* data = t2;


    roundsPerPlayer = 1;
    roundDurationSec = 30;


    if (strcmp(mode, "SINGLE") == 0) {
      strncpy(player1, data, 16);
      player1[16] = '\0';
      numPlayers = 1;
      currentPlayer = 1;
      remainingRoundsP1 = 1;
    } else if (strcmp(mode, "MULTI") == 0) {
      char* comma = strchr(data, ',');
      if (!comma) return;


      *comma = '\0';
      char* p1 = ltrim(data);
      char* p2 = ltrim(comma + 1);


      strncpy(player1, p1, 16);
      player1[16] = '\0';
      strncpy(player2, p2, 16);
      player2[16] = '\0';


      numPlayers = 2;
      currentPlayer = 1;
      remainingRoundsP1 = 1;
      remainingRoundsP2 = 1;
    }
  } else {

  }


  // Show setup result on LCD
  lcd.clear();
  if (numPlayers == 0) {
    lcd.setCursor(0,0);
    lcd.print("SETUP ERROR     ");
    lcd.setCursor(0,1);
    lcd.print("Check format    ");
  } else {
    printPlayerLine();
    lcd.setCursor(0, 1);
    lcd.print("R:");
    if (currentPlayer == 1) lcd.print(remainingRoundsP1);
    else lcd.print(remainingRoundsP2);
    lcd.print(" T:");
    lcd.print(roundDurationSec);
  }
}


void handleZigbeeInput() {
  while (zigbee.available()) {
    char c = zigbee.read();
    if (c == '\r') continue;


    if (c == '\n') {
      zigbeeBuffer[zigbeeIndex] = '\0';
      if (zigbeeIndex > 0) {
        processZigbeeCommand(zigbeeBuffer);
      }
      zigbeeIndex = 0;
    } else {
      if (zigbeeIndex < sizeof(zigbeeBuffer) - 1) {
        zigbeeBuffer[zigbeeIndex++] = c;
      } else {

        zigbeeIndex = 0;
      }
    }
  }
}


void setup() {
  Serial.begin(9600);   
  zigbee.begin(9600);   
 
  lcd.init();
  lcd.backlight();


  pinMode(TRIG1, OUTPUT); pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT); pinMode(ECHO2, INPUT);
  pinMode(TRIG3, OUTPUT); pinMode(ECHO3, INPUT);


  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
 
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, HIGH); 
 
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BTN_LED, OUTPUT);


  lcd.setCursor(0,0);
  lcd.print("ARCADE HOOPS");
 
  Serial.println("SYS:READY");
  zigbee.println("SYS:READY");   
}


void loop() {
  // Always check for incoming ZigBee setup messages
  handleZigbeeInput();



  if (!gameRunning) {
    digitalWrite(BTN_LED, HIGH);


    if (numPlayers == 0) {
      // No setup yet
      lcd.setCursor(0, 0);
      lcd.print("WAITING SETUP   ");
      lcd.setCursor(0, 1);
      lcd.print("via ZigBee      ");
      return;
    }



    bool p1Done = (remainingRoundsP1 <= 0);
    bool p2Done = (numPlayers == 1) ? true : (remainingRoundsP2 <= 0);


    if (p1Done && p2Done) {
      gameFinished = true;
    }


    if (gameFinished) {
      lcd.setCursor(0,0);
      lcd.print("GAME COMPLETE   ");
      lcd.setCursor(0,1);
      lcd.print("Well Played !!  ");
      return;  // ignore button presses
    }



    if (currentPlayer == 1 && remainingRoundsP1 <= 0 && numPlayers == 2 && remainingRoundsP2 > 0) {
      currentPlayer = 2;
    } else if (currentPlayer == 2 && remainingRoundsP2 <= 0 && remainingRoundsP1 > 0) {
      currentPlayer = 1;
    }



    printPlayerLine();
    lcd.setCursor(0, 1);
    lcd.print("R:");
    if (currentPlayer == 1) lcd.print(remainingRoundsP1);
    else lcd.print(remainingRoundsP2);
    lcd.print(" T:");
    lcd.print(roundDurationSec);
    lcd.print(" Press");


    if (digitalRead(BTN_PIN) == LOW) {
      h1Score = 0;
      h2Score = 0;
      h3Score = 0;
     
      gameRunning = true;
      gameStartTime = millis();
     
      digitalWrite(BTN_LED, LOW);
      // Start Beep
      digitalWrite(BUZZER, LOW);
      delay(300);
      digitalWrite(BUZZER, HIGH);
     
      lcd.clear();
      printPlayerLine();
      lcd.setCursor(0,1);
      lcd.print("T:");
      lcd.print(roundDurationSec);
      lcd.print("  Score:0   ");
    }
    return;
  }



  unsigned long currentTime = millis();
  int timeRemaining = roundDurationSec - ((currentTime - gameStartTime) / 1000);


  if (timeRemaining <= 0) {
    gameRunning = false;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("   GAME OVER!   ");


    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER, LOW);
      delay(300);
      digitalWrite(BUZZER, HIGH);
      delay(200);
    }
   
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);


    int totalScore = h1Score + h2Score + h3Score;
    int playerNum = currentPlayer;


    // SEND FINAL STATS TO JAVA VIA ZIGBEE
    // Format: playerNumber,hoop1,hoop2,hoop3
    zigbee.print(playerNum);
    zigbee.print(",");
    zigbee.print(h1Score);
    zigbee.print(",");
    zigbee.print(h2Score);
    zigbee.print(",");
    zigbee.println(h3Score);


    Serial.print("P");
    Serial.print(playerNum);
    Serial.print(" -> ");
    Serial.print(h1Score); Serial.print(",");
    Serial.print(h2Score); Serial.print(",");
    Serial.println(h3Score);



    if (currentPlayer == 1) {
      if (remainingRoundsP1 > 0) remainingRoundsP1--;
    } else if (currentPlayer == 2) {
      if (remainingRoundsP2 > 0) remainingRoundsP2--;
    }


    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Score: ");
    lcd.print(totalScore);
    lcd.setCursor(0,1);
    lcd.print("Round complete  ");
    delay(3000);


    // Decide next player
    if (numPlayers == 2) {
      if (currentPlayer == 1 && remainingRoundsP2 > 0) {
        currentPlayer = 2;
      } else if (currentPlayer == 2 && remainingRoundsP1 > 0) {
        currentPlayer = 1;
      }
    } else {
      currentPlayer = 1; // single player
    }


    return;
  }


  // ULTRASONICS 
  float d1 = readUltrasonic(TRIG1, ECHO1);
  float d2 = readUltrasonic(TRIG2, ECHO2);
  float d3 = readUltrasonic(TRIG3, ECHO3);


  // HOOP 1
  if (d1 < HOOP1_THRESHOLD && !hoop1Active) {
      hoop1Active = true;
      digitalWrite(LED1, HIGH);
      h1Score++;               // Count a shot
      delay(250);              // Simple debounce
  } else if (d1 > HOOP1_THRESHOLD) {
      hoop1Active = false;
      digitalWrite(LED1, LOW);
  }


  // HOOP 2
  if (d2 < HOOP2_THRESHOLD && !hoop2Active) {
      hoop2Active = true;
      digitalWrite(LED2, HIGH);
      h2Score++;
      delay(250);
  } else if (d2 > HOOP2_THRESHOLD) {
      hoop2Active = false;
      digitalWrite(LED2, LOW);
  }


  // HOOP 3 
  if (d3 < HOOP3_THRESHOLD && !hoop3Active) {
      hoop3Active = true;
      digitalWrite(LED3, HIGH);
      h3Score++;
      delay(250);
  } else if (d3 > HOOP3_THRESHOLD) {
      hoop3Active = false;
      digitalWrite(LED3, LOW);
  }



  if (currentTime - lastLCDTime >= 200) {
      int totalScore = h1Score + h2Score + h3Score;


      printPlayerLine();
      lcd.setCursor(0, 1);
      lcd.print("T:");
      if (timeRemaining < 0) timeRemaining = 0;
      lcd.print(timeRemaining);
      lcd.print("  Score:");
      lcd.print(totalScore);
      lcd.print("   ");  


      lastLCDTime = currentTime;
  }
}





