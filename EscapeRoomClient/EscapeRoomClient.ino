#define LIGHT_SENSOR A0


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
  pinMode(LIGHT_SENSOR, INPUT); 

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

void handleLightPuzzle() {
  static unsigned long lastLightCheckMillis = 0;
  
  int lightLevel = analogRead(LIGHT_SENSOR);
  
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
    } 
    else if (currentLightMillis - lightCorrectStartTime >= 2000) {
      Serial.println("החידה נפתרה! האור היה בטווח הנכון למשך 2 שניות");
      puzzleSolved(0);
      currentPuzzle++;
      inCorrectLightRange = false;
      maxLightLevel = 0; 
    }
  } 
  else {
    if (inCorrectLightRange) {
      Serial.println("יצא מטווח האור המתאים - מתחיל מחדש");
      inCorrectLightRange = false;
    }
  }
}

void handleTemperaturePuzzle() {
  Serial.println("בדיקת טמפרטורה");
}

void handleLEDSequencePuzzle() {
  Serial.println("בדיקת מנורות");
}

void handleDistancePuzzle() {
  Serial.println("בדיקת מרחק");
}
