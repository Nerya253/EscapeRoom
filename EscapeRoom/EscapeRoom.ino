#define LOCK_PIN D1   

void setup() {
  Serial.begin(9600);
  pinMode(LOCK_PIN, OUTPUT);
  digitalWrite(LOCK_PIN, LOW);  
  WiFi_setup();
}

void loop() {
  WiFi_loop();
}

void unlockDoor() {
  digitalWrite(LOCK_PIN, HIGH);
}