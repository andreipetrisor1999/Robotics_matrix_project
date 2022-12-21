#include <LiquidCrystal.h>
#include "LedControl.h"
#define MAIN_MENU_SIZE 5
#define HIGHSCORE_MENU_SIZE 5
#define ABOUT_MENU_SIZE 8
#define HOW_TO_PLAY_MENU_SIZE 8
#define SETTINGS_MENU_SIZE 6
#define SETTINGS_OPTIONS_MENU_SIZE 2
#define EASY 1200
#define MEDIUM 800
#define HARD 500
#define VERY_HARD 400

const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte matrixSize = 8;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);
byte matrixBrightness = 5;
bool inGame = false;

const int buzzerPin = 3;
int moveMenuBuzzerTone = 1000;
int pressButtonBuzzerTone = 1500;
int snakeEatsTone = 2000;
int gameOverTone = 800;
bool soundIsOn = true;

const int joyXPin = A0;
const int joyYPin = A1;
const int joySWPin = 2;
const int joyLeftThreshold = 400;
const int joyRightThreshold = 600;
const int joyDownThreshold = 400;
const int joyUpThreshold = 600;
bool joyIsNeutral = true;
byte lastSwState = LOW;
int buttonPressCount = 0;
 
const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
 
 
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

enum menuTypes {
  MAIN,
  HIGH_SCORES,
  SETTINGS,
  ABOUT,
  HOW_TO_PLAY,
  CHANGE_PLAYER_NAME,
  CHANGE_DIFFICULTY,
  CHANGE_GAME_BRIGHTNESS,
  SOUND  
};
menuTypes currentMenu = MAIN;
int currentMenuSize = MAIN_MENU_SIZE;
bool accesedSubMenu = false;
bool accessedSettingsMenu = false;
bool accesedSettingsSubMenu = false;
bool canMoveUD = true;
bool canMoveLR = false;

String alphabet = " ABCDEFGHIJKLMNOPQRSTUVWXYZ";
String currentPlayerName = "            ";
int currentScore = 0;
int highestScore = 0;

String mainMenuItems[MAIN_MENU_SIZE] = {" Start Game", " High Scores ", " Settings", " About", " How to play"};
String highScoresMenuItems[HIGHSCORE_MENU_SIZE];
String aboutMenuItems[ABOUT_MENU_SIZE] = {" THE COOL GAME", "This is a small", " project that", "is intended to", "  entertain", "   anyone.", "  CREATED BY", "PETRISOR ANDREI"};
String howToPlayMenuItems[HOW_TO_PLAY_MENU_SIZE] = {"  Use the joy", "  to move the", " snake around", "  the field.", "  Collect the", "   apples to", " increase your", "    score!"};
String settingsMenuItems[SETTINGS_MENU_SIZE] = {" Player name", " Difficulty" , " LCD brightness", " Game brightness", " Sound", " Save & exit"};


int alphabetPosition = 0;
int letterPosition = 0;
int cursorPosition = 0;
int savedCursor;
char cursor = '>';
byte upArrow[] = {
  B00000,
  B00000,
  B00100,
  B01110,
  B11111,
  B11111,
  B11111,
  B00000
};

byte downArrow[] = {
  B00000,
  B11111,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};


struct Snake{
  int head[2];     // the (row, column) of the snake head
  int body[40][2]; //An array that contains the (row, column) coordinates
  int len;         //The length of the snake 
  int dir[2];      //A direction to move the snake along
};

//Define The Apple as a Struct

struct Apple{
  int rPos; //The row index of the apple
  int cPos; //The column index of the apple
};

byte pic[matrixSize] = {0,0,0,0,0,0,0,0};
Snake snake = {{1,5},{{0,5}, {1,5}}, 2, {1,0}};//Initialize a snake object
Apple apple = {(int)random(0,8),(int)random(0,8)};//Initialize an apple object

//Variables To Handle The Game Time
float oldTime = 0;
float timer = 0;
float updateRate = 3;
float difficultyTimer = MEDIUM;

void setup() {
  Serial.begin(9600);
  pinMode (joySWPin, INPUT_PULLUP);
  lcd.begin(16, 2);
  lcd.createChar(0, upArrow);
  lcd.createChar(1, downArrow);

  lc.shutdown(0, false);
  lc.setIntensity(0, matrixBrightness);
  lc.clearDisplay(0);
}

void loop() {
  Serial.print(accessedSettingsMenu);
  Serial.print(" | ");
  Serial.print(accesedSettingsSubMenu);
  Serial.print(" | ");
  Serial.print(cursorPosition);
  Serial.print(" | ");
  Serial.print(buttonPressCount);
  Serial.print('\n');
  if(millis() < 1000){
    lcd.setCursor(0, 0);
    lcd.print(" THE COOL GAME ");
    lcd.setCursor(0, 1);
    lcd.print(" welcome player ");
  }else {
    if(millis() < 1200) {
      moveMainMenu(cursorPosition);
    } 
    else {
      if(inGame){  
        playTheGame();
      } else {
        readAxisAndButton(currentMenu, currentMenuSize); 
        for (int row = 0; row < matrixSize; row++) {
          for (int col = 0; col < matrixSize; col++) {
            lc.setLed(0, row, col, false);
          }
        } 
      }
    }
  }
}

void readAxisAndButton(enum menuTypes menuType, int menuSize){
  byte swState = digitalRead(joySWPin);
  int joyX = analogRead(joyXPin);
  int joyY = analogRead(joyYPin);
  if(joyX < joyDownThreshold && joyIsNeutral && (cursorPosition < menuSize - 1 || accesedSettingsSubMenu) && canMoveUD){
    if(soundIsOn) tone(buzzerPin, moveMenuBuzzerTone, 100);
    joyIsNeutral = false;
    cursorPosition ++;
    switch (menuType){
      case MAIN:
        if(cursorPosition != menuSize - 1){
          moveMainMenu(cursorPosition);
        } else {
            for(int i=0; i < menuSize; i++){
              mainMenuItems[i].setCharAt(0,' ');
            }
            lcd.clear();
            mainMenuItems[cursorPosition].setCharAt(0,cursor);
            lcd.print(mainMenuItems[cursorPosition-1]);
            lcd.setCursor(0, 1);
            lcd.print(mainMenuItems[cursorPosition]);
          }
      break;

      case HIGH_SCORES:
      break;
      
      case SETTINGS:
        if(cursorPosition != menuSize - 1){
          moveSettingsMenu(cursorPosition);
        } else {
            for(int i=0; i < menuSize; i++){
              settingsMenuItems[i].setCharAt(0,' ');
            }
            lcd.clear();
            settingsMenuItems[cursorPosition].setCharAt(0,cursor);
            lcd.print(settingsMenuItems[cursorPosition-1]);
            lcd.setCursor(0, 1);
            lcd.print(settingsMenuItems[cursorPosition]);
          }
      break;

      case ABOUT:
        if(cursorPosition > ABOUT_MENU_SIZE - 2){
          cursorPosition = ABOUT_MENU_SIZE - 2;
          noTone(buzzerPin);
        }
        moveAboutMenu(cursorPosition);
      break;
      
      case HOW_TO_PLAY:
        if(cursorPosition > HOW_TO_PLAY_MENU_SIZE - 2){
          cursorPosition = HOW_TO_PLAY_MENU_SIZE - 2;
          noTone(buzzerPin);
        }
        moveHowToPlayMenu(cursorPosition);
      break;

      case CHANGE_PLAYER_NAME:
        alphabetPosition ++;
        if(alphabetPosition > 26) {
          alphabetPosition = 0;
        }
        changePlayerName(letterPosition, alphabetPosition);
      break;
    } 
  }

  if(joyX > joyUpThreshold && joyIsNeutral && (cursorPosition > 0 || accesedSettingsSubMenu) && canMoveUD){
    if(soundIsOn) tone(buzzerPin, moveMenuBuzzerTone, 100);
    joyIsNeutral = false;
    cursorPosition --;
    switch (menuType){
      case MAIN:
        moveMainMenu(cursorPosition);
      break;
      
      case HIGH_SCORES:
      break;

      case SETTINGS:
        moveSettingsMenu(cursorPosition);
      break;
      
      case ABOUT:
        moveAboutMenu(cursorPosition);
      break;

      case HOW_TO_PLAY:
        moveHowToPlayMenu(cursorPosition);
      break;
      
      case CHANGE_PLAYER_NAME:        
        alphabetPosition --;
        if(alphabetPosition < 0) {
          alphabetPosition = 26;
        }
        changePlayerName(letterPosition, alphabetPosition);
      break;
    }
  }

  if(joyY < joyLeftThreshold && joyIsNeutral && canMoveLR){
    if(soundIsOn) tone(buzzerPin, moveMenuBuzzerTone, 100);
    joyIsNeutral = false;
    switch(menuType){
      case CHANGE_PLAYER_NAME:
        letterPosition --;
        if(letterPosition < 0) {
          letterPosition = 0;
          noTone(buzzerPin);
        }
        if(currentPlayerName[letterPosition] != alphabet[0]) alphabetPosition = alphabet.indexOf(currentPlayerName[letterPosition]);
        else alphabetPosition = 0;
        changePlayerName(letterPosition, alphabetPosition);
      break;

      case CHANGE_DIFFICULTY:
        if(difficultyTimer == MEDIUM){
          difficultyTimer = EASY;
          lcd.clear();
          lcd.print("Coose difficulty");
          lcd.setCursor(0, 1);
          lcd.print("  <EASY>");          
        }
        
        if(difficultyTimer == HARD){
          difficultyTimer = MEDIUM;
          lcd.clear();
          lcd.print("Coose difficulty");
          lcd.setCursor(0, 1);
          lcd.print("  <MEDIUM>");          
        }
        
        if(difficultyTimer == VERY_HARD){
          difficultyTimer = HARD;
          lcd.clear();
          lcd.print("Coose difficulty");
          lcd.setCursor(0, 1);
          lcd.print("  <HARD>");          
        }
      break;

      case SOUND:
        if(soundIsOn) {
          soundIsOn = false;
          lcd.clear();
          lcd.print(" Choose option: ");
          lcd.setCursor(0, 1);
          lcd.print("     < OFF >    ");
        } else {
          soundIsOn = true;
          lcd.clear();
          lcd.print(" Choose option: ");
          lcd.setCursor(0, 1);
          lcd.print("     < ON >     ");
        }
      break;

      case CHANGE_GAME_BRIGHTNESS:
        if(matrixBrightness < 1){
          matrixBrightness = 0;
          noTone(buzzerPin);
        } else {
          matrixBrightness --;
        }
        lc.setIntensity(0, matrixBrightness);
        lcd.clear();
        lcd.print("Brightness level");
        lcd.setCursor(0, 1);
        String brightnessMenuItem = "      <" + String(matrixBrightness) + ">";
        lcd.print(brightnessMenuItem);
      break;

    }
  }

  if(joyY > joyRightThreshold && joyIsNeutral && canMoveLR){
    if(soundIsOn) tone(buzzerPin, moveMenuBuzzerTone, 100);
    joyIsNeutral = false;
    switch(menuType){
      case CHANGE_PLAYER_NAME:
        if(currentPlayerName[letterPosition] != alphabet[0]){
          letterPosition ++;
          alphabetPosition = alphabet.indexOf(currentPlayerName[letterPosition]);
          if(letterPosition > 15) {
            letterPosition = 15;
            noTone(buzzerPin);
          }
          changePlayerName(letterPosition, alphabetPosition);          
        }
        else {
          noTone(buzzerPin);
        }
      break;

      case CHANGE_DIFFICULTY:

        if(difficultyTimer == HARD){
          difficultyTimer = VERY_HARD;
          lcd.clear();
          lcd.print("Coose difficulty");
          lcd.setCursor(0, 1);
          lcd.print("  <VERY HARD>");          
        }
        
        if(difficultyTimer == MEDIUM){
          difficultyTimer = HARD;
          lcd.clear();
          lcd.print("Coose difficulty");
          lcd.setCursor(0, 1);
          lcd.print("  <HARD>");          
        }
        
        if(difficultyTimer == EASY){
          difficultyTimer = MEDIUM;
          lcd.clear();
          lcd.print("Coose difficulty");
          lcd.setCursor(0, 1);
          lcd.print("  <MEDIUM>");          
        }
      break;
      
      case SOUND:
        if(soundIsOn) {
          soundIsOn = false;
          lcd.clear();
          lcd.print(" Choose option: ");
          lcd.setCursor(0, 1);
          lcd.print("     < OFF >    ");
        } else {
          soundIsOn = true;
          lcd.clear();
          lcd.print(" Choose option: ");
          lcd.setCursor(0, 1);
          lcd.print("     < ON >     ");
        }
      break;

      case CHANGE_GAME_BRIGHTNESS:
        matrixBrightness ++;
        if(matrixBrightness > 15){
          matrixBrightness = 15;
          noTone(buzzerPin);
        }
        lc.setIntensity(0, matrixBrightness);
        lcd.clear();
        lcd.print("Brightness level");
        lcd.setCursor(0, 1);
        String brightnessMenuItem = "      <" + String(matrixBrightness) + ">";
        lcd.print(brightnessMenuItem);
      break;
    }
  }

  if(swState != lastSwState){
    if(swState == LOW){
      buttonPressCount ++;
      if(soundIsOn) tone(buzzerPin, pressButtonBuzzerTone, 100);
      if(!accesedSubMenu && !accessedSettingsMenu && buttonPressCount == 1){
        switch(cursorPosition){
          case 1:
            accesedSubMenu = true;
            currentMenu = HIGH_SCORES;
            if(highScoresMenuItems[0].length() == 0){
              canMoveUD = false;
              canMoveLR = false;
              lcd.clear();
              lcd.print(" NO HIGH SCORES ");
              lcd.setCursor(0, 1);
              lcd.print("   AVAILABLE   ");
            }
          break;

          case 2:
            accessedSettingsMenu = true;
            canMoveUD = true;
            canMoveLR = false;            
            currentMenu = SETTINGS;
            currentMenuSize = SETTINGS_MENU_SIZE;
            cursorPosition = 0;
            moveSettingsMenu(cursorPosition);
          break;

          case 3:
            accesedSubMenu = true;
            canMoveUD = true;
            canMoveLR = false;            
            currentMenu = ABOUT;
            currentMenuSize = ABOUT_MENU_SIZE;
            cursorPosition = 0;
            moveAboutMenu(cursorPosition);
          break;

          case 4:
            accesedSubMenu = true;
            canMoveUD = true;
            canMoveLR = false; 
            currentMenu = HOW_TO_PLAY;
            currentMenuSize = HOW_TO_PLAY_MENU_SIZE;
            cursorPosition = 0;
            moveHowToPlayMenu(cursorPosition);
          break;
          
          case 0:
            buttonPressCount = 0;
            String score = "SCORE:" + String(currentScore);
            lcd.clear();
            lcd.print(score);
            lcd.setCursor(0, 1);
            String highScore = "HIGHSCORE:" + String (highestScore);
            lcd.print(highScore);
            inGame = true;
          break;
        }
      }
      if ((accesedSubMenu || (accessedSettingsMenu && cursorPosition == 5)) && buttonPressCount == 2){
        buttonPressCount = 0;
        accesedSubMenu = false;
        accessedSettingsMenu = false;
        canMoveUD = true;
        canMoveLR = false;         
        currentMenu = MAIN;
        currentMenuSize = MAIN_MENU_SIZE;
        cursorPosition = 0;
        moveMainMenu(cursorPosition); 
      }

      if (accesedSettingsSubMenu && buttonPressCount == 3){
        buttonPressCount = 1;
        letterPosition = 0;
        alphabetPosition = 0;
        accesedSettingsSubMenu = false;
        canMoveUD = true;
        canMoveLR = false; 
        currentMenu = SETTINGS;
        currentMenuSize = SETTINGS_MENU_SIZE;
        cursorPosition = 0;
        moveSettingsMenu(cursorPosition);
      }

      if (accessedSettingsMenu && buttonPressCount == 2) {        
        switch (cursorPosition){
          case 0 :
            accesedSettingsSubMenu = true;
            canMoveUD = true;
            canMoveLR = true;
            currentMenu = CHANGE_PLAYER_NAME;
            currentMenuSize = SETTINGS_OPTIONS_MENU_SIZE;
            cursorPosition = 0; 
            changePlayerName(cursorPosition, alphabet.indexOf(currentPlayerName[cursorPosition]));                  
          break;

          case 1 :
            accesedSettingsSubMenu = true;
            canMoveUD = false;
            canMoveLR = true; 
            currentMenu = CHANGE_DIFFICULTY;
            currentMenuSize = SETTINGS_OPTIONS_MENU_SIZE;
            cursorPosition = 0;
            if(difficultyTimer == EASY){
              lcd.clear();
              lcd.print("Coose difficulty");
              lcd.setCursor(0, 1);
              lcd.print("  <EASY>");          
            }
            if(difficultyTimer == MEDIUM){
              lcd.clear();
              lcd.print("Coose difficulty");
              lcd.setCursor(0, 1);
              lcd.print("  <MEDIUM>");          
            }
            if(difficultyTimer == HARD){
              lcd.clear();
              lcd.print("Coose difficulty");
              lcd.setCursor(0, 1);
              lcd.print("  <HARD>");          
            }
            if(difficultyTimer == VERY_HARD){
              lcd.clear();
              lcd.print("Coose difficulty");
              lcd.setCursor(0, 1);
              lcd.print("  <VERYHARD>");          
            }
          break;

          case 2:
            buttonPressCount = 1;
          break;

          case 4 :
            accesedSettingsSubMenu = true;
            canMoveUD = false;
            canMoveLR = true; 
            currentMenu = SOUND;
            currentMenuSize = SETTINGS_OPTIONS_MENU_SIZE;
            cursorPosition = 0; 
            if(soundIsOn){
              lcd.clear();
              lcd.print(" Choose option: ");
              lcd.setCursor(0, 1);
              lcd.print("     < ON >     ");
            } else {
              lcd.clear();
              lcd.print(" Choose option: ");
              lcd.setCursor(0, 1);
              lcd.print("     < OFF >    ");
            }
          break;

          case 3 :
            accesedSettingsSubMenu = true;
            canMoveUD = false;
            canMoveLR = true; 
            currentMenu = CHANGE_GAME_BRIGHTNESS;
            currentMenuSize = SETTINGS_OPTIONS_MENU_SIZE;
            cursorPosition = 0;
            lcd.clear();
            lcd.print("Brightness level");
            lcd.setCursor(0, 1);
            String brightnessMenuItem = "      <" + String(matrixBrightness) + ">";
            lcd.print(brightnessMenuItem);
          break;          
        }        
      }       
    }
    lastSwState = swState;
  }
   
  if(joyDownThreshold <= joyX && joyX <= joyUpThreshold && joyLeftThreshold <= joyY && joyY <= joyRightThreshold){
   joyIsNeutral = true;
  }
}

void moveMainMenu(int position){
  for(int i=0; i< MAIN_MENU_SIZE; i++){
    mainMenuItems[i].setCharAt(0,' ');
  }
  lcd.clear();
  mainMenuItems[position].setCharAt(0,cursor);
  lcd.print(mainMenuItems[position]);
  lcd.setCursor(0, 1);
  lcd.print(mainMenuItems[position+1]);
}

void moveSettingsMenu(int position){
  for(int i=0; i< SETTINGS_MENU_SIZE; i++){
    settingsMenuItems[i].setCharAt(0,' ');
  }
  lcd.clear();
  settingsMenuItems[position].setCharAt(0,cursor);
  lcd.print(settingsMenuItems[position]);
  lcd.setCursor(0, 1);
  lcd.print(settingsMenuItems[position+1]);
}

void moveAboutMenu(int position){
    lcd.clear();
    lcd.write(byte(0));
    lcd.print(aboutMenuItems[position]);
    lcd.setCursor(0, 1);
    lcd.write(byte(1));
    lcd.print(aboutMenuItems[position+1]);
}

void moveHowToPlayMenu(int position){
    lcd.clear();
    lcd.write(byte(0));
    lcd.print(howToPlayMenuItems[position]);
    lcd.setCursor(0, 1);
    lcd.write(byte(1));
    lcd.print(howToPlayMenuItems[position+1]);
}

void changePlayerName(int letterPos, int alphabetPos){
  lcd.clear();
  lcd.setCursor(letterPos, 0);
  lcd.write(byte(1));
  lcd.setCursor(0, 1);
  currentPlayerName.setCharAt(letterPos, alphabet[alphabetPos]);
  lcd.print(currentPlayerName);
}

void playTheGame(){
  int joystickX = analogRead(joyXPin);
  int joystickY = analogRead(joyYPin);

  float deltaTime = calculateDeltaTime();
  timer += deltaTime;
  
  if(joystickY < 400 && snake.dir[1]==0){
    if(soundIsOn) tone(buzzerPin, moveMenuBuzzerTone, 100);
    snake.dir[0] = 0;
    snake.dir[1] = -1;
  }else if(joystickY > 600 && snake.dir[1]==0){
    if(soundIsOn) tone(buzzerPin, moveMenuBuzzerTone, 100);
    snake.dir[0] = 0;
    snake.dir[1] = 1;
  }else if(joystickX > 600 && snake.dir[0]==0){
    if(soundIsOn) tone(buzzerPin, moveMenuBuzzerTone, 100);
    snake.dir[0] = -1;
    snake.dir[1] = 0;
  }else if(joystickX < 400 && snake.dir[0]==0){
    if(soundIsOn) tone(buzzerPin, moveMenuBuzzerTone, 100);
    snake.dir[0] = 1;
    snake.dir[1] = 0;
  }
  
  //Update
  if(timer > difficultyTimer/updateRate){
    timer = 0;
    Update();
  }
  
  //Render
  Render(); 
} 

float calculateDeltaTime(){
  float currentTime = millis();
  float dt = currentTime - oldTime;
  oldTime = currentTime;
  return dt;
}

void reset(){
  for(int j=0;j<8;j++){
    pic[j] = 0;
  }
}

void Update(){
  reset();//Reset (Clear) the 8x8 LED matrix
  
  int newHead[2] = {snake.head[0]+snake.dir[0], snake.head[1]+snake.dir[1]};

  //Handle Borders
  if(newHead[0]==8){
    newHead[0]=0;
  }else if(newHead[0]==-1){
    newHead[0] = 7;
  }else if(newHead[1]==8){
    newHead[1]=0;
  }else if(newHead[1]==-1){
    newHead[1]=7;
  }
  
  //Check If The Snake hits itself
   for(int j=0;j<snake.len;j++){
    if(snake.body[j][0] == newHead[0] && snake.body[j][1] == newHead[1]){
      //Pause the game for 2 sec then go to main menu
      if(soundIsOn) tone(buzzerPin, gameOverTone, 2000);
      delay(2000);
      moveMainMenu(0);
      inGame = false;      
      resetScore();
      snake = {{1,5},{{0,5}, {1,5}}, 2, {1,0}};//Reinitialize the snake object
      apple = {(int)random(0,8),(int)random(0,8)};//Reinitialize an apple object
      return;
    }
  }

  //Check if The snake ate the apple
  if(newHead[0] == apple.rPos && newHead[1] ==apple.cPos){
    if(soundIsOn) tone(buzzerPin, snakeEatsTone, 100);
    increaseCurrentScore();
    snake.len = snake.len+1;
    apple.rPos = (int)random(0,8);
    apple.cPos = (int)random(0,8);
  }else{
    removeFirst();//Shifting the array to the left
  }
  
  snake.body[snake.len-1][0]= newHead[0];
  snake.body[snake.len-1][1]= newHead[1];
  
  snake.head[0] = newHead[0];
  snake.head[1] = newHead[1];
  
  //Update the pic Array to Display(snake and apple)
  for(int j=0;j<snake.len;j++){
    pic[snake.body[j][0]] |= 128 >> snake.body[j][1];
  }
  pic[apple.rPos] |= 128 >> apple.cPos;
  
}

void Render(){
  
   for(int i=0;i<8;i++){
    lc.setRow(0,i,pic[i]);
   }
}

void removeFirst(){
  for(int j=1;j<snake.len;j++){
    snake.body[j-1][0] = snake.body[j][0];
    snake.body[j-1][1] = snake.body[j][1];
  }
}

void increaseCurrentScore(){
  currentScore += 1;
  if(currentScore > highestScore) highestScore = currentScore;
  String score = "SCORE:" + String(currentScore);
  String highScore = "HIGHSCORE:" + String (highestScore);
  lcd.clear();
  lcd.print(score);
  lcd.setCursor(0, 1);
  lcd.print(highScore);
}

void resetScore(){
  currentScore = 0;
  if(currentScore > highestScore) highestScore = currentScore;
  String score = "SCORE:" + String(currentScore);
  String highScore = "HIGHSCORE:" + String (highestScore);
  lcd.clear();
  lcd.print(score);
  lcd.setCursor(0, 1);
  lcd.print(highScore);
}
