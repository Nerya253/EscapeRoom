#include <DHT_U.h>
#include <DHT.h>

#define FUN D4

#define DHTPIN D1
#define DHTTYPE DHT11

#define pinMuxA D5
#define pinMuxB D6
#define pinMuxC D7
#define pinMuxInOut A0

DHT dht(DHTPIN, DHTTYPE);

extern int secretCode[4];
void SendData(int digit);
void SendCodeDigit(int puzzleNumber);
void puzzleSolved(int puzzleNumber);
void wifi_Setup();


//חידת אור
unsigned long currentLightMillis = 0;
unsigned long lastSensorReadMillis = 0;
unsigned long lightCorrectStartTime = 0;
int maxLightLevel = 0;
int currentPuzzle = 0;
bool inCorrectLightRange = false;
bool samplingMaxLight = true;
unsigned long samplingStartTime = 0;


void setup() {
  Serial.begin(9600);
  wifi_Setup();
  pinMode(pinMuxA, OUTPUT);
  pinMode(pinMuxB, OUTPUT);
  pinMode(pinMuxC, OUTPUT);
  pinMode(pinMuxInOut, INPUT);
  pinMode(FUN, OUTPUT);
  digitalWrite(FUN, HIGH);
  dht.begin();

  samplingStartTime = millis();
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
  static unsigned long lastLightCheckMillis = 0;

  int lightLevel = ReadMuxChannel(0);

  if (samplingMaxLight) {
    if (lightLevel > maxLightLevel) {
      maxLightLevel = lightLevel;
    }

    if (currentLightMillis - samplingStartTime >= 5000) {
      samplingMaxLight = false;
      Serial.print("דגימת האור הסתיימה. רמת אור מקסימלית: ");
      Serial.println(maxLightLevel);
      Serial.print("רמת אור יעד (80%): ");
      Serial.println(maxLightLevel * 0.8);
    }
    return;
  }

  if (lightLevel > maxLightLevel) {
    maxLightLevel = lightLevel;
  }

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

//חידת טמפרטורה
float currentTemp = 0;
float targetTemp = 0;
bool tempPuzzleSolved = false;
bool tempInCorrectRange = false;
unsigned long tempCorrectStartTime = 0;
static unsigned long lastTempCheckMillis = 0;

void handleTemperaturePuzzle() {
  if (currentLightMillis - lastTempCheckMillis >= 2000) {
    lastTempCheckMillis = currentLightMillis;

    float t = dht.readTemperature();

    currentTemp = t;
    Serial.print("Temperature = ");
    Serial.println(currentTemp);

    if (targetTemp == 0) {
      targetTemp = currentTemp - 0.2;
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

void handleLEDSequencePuzzle() {
  Serial.println("בדיקת מנורות");
  delay(5000);
}

void handleDistancePuzzle() {
  Serial.println("בדיקת מרחק");
  delay(5000);
}
