void setup() {
  Serial.begin(9600);
  wifiSetup();
}

void loop() {
  wifiLoop();
}
