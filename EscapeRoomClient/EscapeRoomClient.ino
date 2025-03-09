#include <DHT_U.h>
#include <DHT.h>

#define FUN D1

#define DHTPIN D4
#define DHTTYPE DHT22

#define pinMuxA D5
#define pinMuxB D6
#define pinMuxC D7
#define pinMuxInOut A0

#define R_LED D1
#define G_LED D2
#define B_LED D3

#define R_BTN D5
#define G_BTN D6
#define B_BTN D7

DHT dht(DHTPIN, DHTTYPE);

extern int secretCode[4];
void SendData(int digit);
void SendCodeDigit(int puzzleNumber);
void puzzleSolved(int puzzleNumber);
void wifi_Setup();

int currentPuzzle = 0;
unsigned long currentLightMillis = 0;
unsigned long lastSensorReadMillis = 0;

//חידה 1 - חיישן אור
unsigned long lightCorrectStartTime = 0;
int maxLightLevel = 0;
bool inCorrectLightRange = false;
bool samplingMaxLight = true;
unsigned long samplingStartTime = 0;

//חידה 2 - טמפרטורה
float currentTemp = 0;
float targetTemp = 0;
bool tempPuzzleSolved = false;
bool tempInCorrectRange = false;
unsigned long tempCorrectStartTime = 0;
static unsigned long lastTempCheckMillis = 0;

//חידה 3 - משחק אורות
const int sequenceLength = 4;
int sequence[sequenceLength];
int leds[3] = { R_LED, G_LED, B_LED };
int buttons[3] = { R_BTN, G_BTN, B_BTN };
int currentStep = 0;
bool displayingSequence = true;
unsigned long lastChangeTime = 0;
int displayState = 0;

unsigned long buttonReleaseTime = 0;
bool waitingForButtonRelease = false;
int lastPressedButton = -1;

// משתנים לאפקטים
bool isBlinking = false;
int blinkCount = 0;
unsigned long blinkTime = 0;
int blinkState = 0;

bool showingVictory = false;
int victoryStep = 0;
unsigned long victoryTime = 0;

bool showingStartSequence = true;
int startSequenceCount = 0;
unsigned long startSequenceTime = 0;


void setup() {
  Serial.begin(9600);
  wifi_Setup();

  pinMode(FUN, OUTPUT);
  digitalWrite(FUN, HIGH);

  pinMode(A0, INPUT);
  pinMode(pinMuxA, OUTPUT);
  pinMode(pinMuxB, OUTPUT);
  pinMode(pinMuxC, OUTPUT);
  pinMode(pinMuxInOut, INPUT);

  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(B_LED, OUTPUT);
  pinMode(R_BTN, INPUT_PULLUP);
  pinMode(G_BTN, INPUT_PULLUP);
  pinMode(B_BTN, INPUT_PULLUP);

  dht.begin();

  digitalWrite(A0, LOW); 

  digitalWrite(R_LED, LOW);
  digitalWrite(G_LED, LOW);
  digitalWrite(B_LED, LOW);

  for (int i = 0; i < 10; i++) {
    analogRead(A0);
  }

  generateRandomSequence();

  samplingStartTime = millis();
  
  showingStartSequence = true;
  startSequenceCount = 0;
  startSequenceTime = millis();
}

void loop() {
  currentLightMillis = millis();
  static unsigned long statusPrintTimer = 0;
  if (currentLightMillis - statusPrintTimer >= 5000) {
    statusPrintTimer = currentLightMillis;
    Serial.print("מצב נוכחי: חידה ");
    Serial.println(currentPuzzle + 1);
  }

  if (currentLightMillis - lastSensorReadMillis >= 1000) {
    lastSensorReadMillis = currentLightMillis;
  }

  switch (currentPuzzle) {
    case 0:
      handleLightPuzzle();
      break;
    case 1:
      handleTemperaturePuzzle();
      break;
    case 2:
      handleLEDSequencePuzzle();
      break;
    case 3:
      handleDistancePuzzle();
      break;
  }
}

int ReadMuxChannel(byte chnl) {
  int a = (bitRead(chnl, 0) > 0) ? HIGH : LOW;
  int b = (bitRead(chnl, 1) > 0) ? HIGH : LOW;
  int c = (bitRead(chnl, 2) > 0) ? HIGH : LOW;

  digitalWrite(pinMuxA, a);
  digitalWrite(pinMuxB, b);
  digitalWrite(pinMuxC, c);

  int ret = analogRead(pinMuxInOut);
  return ret;
}

void handleLightPuzzle() {
  digitalWrite(FUN, HIGH);
  pinMode(A0, INPUT);
  pinMode(pinMuxA, OUTPUT);
  pinMode(pinMuxB, OUTPUT);
  pinMode(pinMuxC, OUTPUT);


  static unsigned long lastLightCheckMillis = 0;

  int lightLevel = ReadMuxChannel(0);

  if (samplingMaxLight) {
    if (lightLevel > maxLightLevel) {
      maxLightLevel = lightLevel;
    }

    if (currentLightMillis - samplingStartTime >= 5000) {
      samplingMaxLight = false;
    }
    return;
  }

  if (lightLevel > maxLightLevel) {
    maxLightLevel = lightLevel;
  }
  
  Serial.print("רמת אור: ");
  Serial.print(lightLevel);
  Serial.print(" רמת אור מקסימלית: ");
  Serial.print(maxLightLevel);
  Serial.print(" רמת אור יעד (80%): ");
  Serial.println(maxLightLevel * 0.8);
  
  if (lightLevel <= maxLightLevel * 0.8) {
    if (!inCorrectLightRange) {
      inCorrectLightRange = true;
      lightCorrectStartTime = currentLightMillis;
      Serial.println("נכנס לטווח האור המתאים");
    } else if (currentLightMillis - lightCorrectStartTime >= 2000) {
      Serial.println("החידה נפתרה! האור היה בטווח הנכון למשך 2 שניות");
      puzzleSolved(0);
      currentPuzzle++;
      inCorrectLightRange = false;
      maxLightLevel = 0;
      digitalWrite(FUN, LOW);
    }
  } else {
    if (inCorrectLightRange) {
      Serial.println("יצא מטווח האור המתאים - מתחיל מחדש");
      inCorrectLightRange = false;
    }
  }
}

void handleTemperaturePuzzle() {
  pinMode(FUN, OUTPUT);

  if (currentLightMillis - lastTempCheckMillis >= 2000) {
    lastTempCheckMillis = currentLightMillis;

    float t = dht.readTemperature();

    currentTemp = t;
    Serial.print("Temperature = ");
    Serial.println(currentTemp);

    if (targetTemp == 0) {
      targetTemp = currentTemp - 0.1;
      Serial.print("טמפרטורת יעד: ");
      Serial.println(targetTemp);
    }

    if (currentTemp <= targetTemp) {
      if (!tempInCorrectRange) {
        tempInCorrectRange = true;
        tempCorrectStartTime = currentLightMillis;
        Serial.println("הגיע לטמפרטורה הרצויה!");
      } else if (currentLightMillis - tempCorrectStartTime >= 2000) {
        Serial.println("החידה נפתרה! הטמפרטורה ירדה מספיק למשך 2 שניות");
        puzzleSolved(1);
        digitalWrite(FUN, HIGH);
        currentPuzzle++;
        tempPuzzleSolved = true;
      }
    } else {
      if (tempInCorrectRange) {
        Serial.println("הטמפרטורה עלתה מעל היעד - מתחיל מחדש");
        tempInCorrectRange = false;
      }
    }
  }
}

void generateRandomSequence() {
  long randSeed = millis() * analogRead(A0);
  randomSeed(randSeed);

  for (int i = 0; i < sequenceLength; i++) {
    sequence[i] = random(0, 3);
  }

  Serial.println("New random sequence:");
  for (int i = 0; i < sequenceLength; i++) {
    Serial.print(sequence[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void handleLEDSequencePuzzle() {
  pinMode(R_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(B_LED, OUTPUT);
  pinMode(R_BTN, INPUT_PULLUP);
  pinMode(G_BTN, INPUT_PULLUP);
  pinMode(B_BTN, INPUT_PULLUP);

  if (showingStartSequence) {
    handleStartSequence();
    return;
  }
  
  if (isBlinking) {
    handleErrorBlink();
    return;
  }
  
  if (showingVictory) {
    handleVictoryAnimation();
    return;
  }
  
  if (waitingForButtonRelease) {
    if (digitalRead(buttons[lastPressedButton]) == HIGH) {
      digitalWrite(leds[lastPressedButton], LOW); 
      waitingForButtonRelease = false;
      buttonReleaseTime = millis();
    }
    return;
  }
  
  if (lastPressedButton != -1 && millis() - buttonReleaseTime < 200) {
    return;
  }
  
  lastPressedButton = -1;

  if (displayingSequence) {
    unsigned long currentTime = millis();
    if (currentTime - lastChangeTime > 400) {
      lastChangeTime = currentTime;

      digitalWrite(R_LED, LOW);
      digitalWrite(G_LED, LOW);
      digitalWrite(B_LED, LOW);

      if (displayState == 0) {
        if (currentStep < sequenceLength) {
          digitalWrite(leds[sequence[currentStep]], HIGH);
        }
        displayState = 1;
      } else {
        currentStep++;
        displayState = 0;

        if (currentStep >= sequenceLength) {
          displayingSequence = false;
          currentStep = 0;
        }
      }
    }
  }
  else {
    for (int i = 0; i < 3; i++) {
      if (digitalRead(buttons[i]) == LOW) {
        digitalWrite(leds[i], HIGH);
        lastPressedButton = i;
        waitingForButtonRelease = true;

        if (i == sequence[currentStep]) {
          currentStep++;
          if (currentStep >= sequenceLength) {
            Serial.println("החידה נפתרה!");
            showingVictory = true;
            victoryStep = 0;
            victoryTime = millis();
            puzzleSolved(2);
            // הסרתי את העלאת מספר החידה מכאן
            // currentPuzzle++;
          }
        } else {
          isBlinking = true;
          blinkCount = 0;
          blinkState = 0;
          blinkTime = millis();
        }
        
        break;
      }
    }
  }
}

void handleStartSequence() {
  if (millis() - startSequenceTime > 300) {
    startSequenceTime = millis();
    
    if (blinkState == 0) {
      digitalWrite(R_LED, HIGH);
      digitalWrite(G_LED, HIGH);
      digitalWrite(B_LED, HIGH);
      blinkState = 1;
    } else {
      digitalWrite(R_LED, LOW);
      digitalWrite(G_LED, LOW);
      digitalWrite(B_LED, LOW);
      blinkState = 0;
      startSequenceCount++;
      
      if (startSequenceCount >= 3) {
        showingStartSequence = false;
        displayingSequence = true;
        currentStep = 0;
        displayState = 0;
        lastChangeTime = millis();
      }
    }
  }
}

void handleErrorBlink() {
  if (millis() - blinkTime > 200) {
    blinkTime = millis();
    
    if (blinkState == 0) {
      digitalWrite(R_LED, HIGH);
      digitalWrite(G_LED, HIGH);
      digitalWrite(B_LED, HIGH);
      blinkState = 1;
    } else {
      digitalWrite(R_LED, LOW);
      digitalWrite(G_LED, LOW);
      digitalWrite(B_LED, LOW);
      blinkState = 0;
      blinkCount++;
      
      if (blinkCount >= 3) {
        isBlinking = false;
        currentStep = 0;
        displayingSequence = true;
        displayState = 0;
        lastChangeTime = millis();
      }
    }
  }
}

void handleVictoryAnimation() {
  if (millis() - victoryTime > 150) {
    victoryTime = millis();
    
    digitalWrite(R_LED, LOW);
    digitalWrite(G_LED, LOW);
    digitalWrite(B_LED, LOW);
    
    if (victoryStep < 9) {
      int ledIndex = victoryStep % 3;
      digitalWrite(leds[ledIndex], HIGH);
    }
    
    victoryStep++;
    
    if (victoryStep >= 9) {
      showingVictory = false;
      
      digitalWrite(R_LED, LOW);
      digitalWrite(G_LED, LOW);
      digitalWrite(B_LED, LOW);
      
      generateRandomSequence();
      
      showingStartSequence = true;
      startSequenceCount = 0;
      startSequenceTime = millis();
      
      // הוספתי את העלאת מספר החידה כאן, בסוף אנימציית הניצחון
      currentPuzzle++;
    }
  }
}

void handleDistancePuzzle() {
  Serial.println("בדיקת מרחק");
  delay(5000);
}