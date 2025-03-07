#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "Project";
const char* pswd = "88888888";

WiFiClient client;
int server_port = 80;

int secretCode[4];

unsigned long previousWifiMillis = 0;

void generateRandomCode() {
  randomSeed(analogRead(A0));
  for (int i = 0; i < 4; i++) {
    secretCode[i] = random(0, 10);
  }
}

void wifi_Setup() {
  Serial.println("wifiSetup");
  WiFi.begin(ssid, pswd);
  previousWifiMillis = millis(); 

  // while (WiFi.status() != WL_CONNECTED) {
  //   Serial.println("trying ...");
  //   if (millis() - previousWifiMillis >= 100) {
  //     previousWifiMillis = millis();
  //   }
  // }

  Serial.println("Connected to network");

  generateRandomCode();
}

void SendData(int digit) {
  HTTPClient http;
  String url = "http://55.55.55.55/addDigit?digit=" + String(digit);
  http.begin(client, url);
  int httpCode = http.GET();
  String payload = http.getString();
  http.end();
}

void SendCodeDigit(int puzzleNumber) {
  if (puzzleNumber >= 0 && puzzleNumber < 4) {
    int digit = secretCode[puzzleNumber];
    SendData(digit); 
    Serial.print("שליחת ספרה ");
    Serial.print(puzzleNumber + 1);
    Serial.print(" מהקוד: ");
    Serial.println(digit);
  }
}

void puzzleSolved(int puzzleNumber) {
  Serial.print("חידה מספר ");
  Serial.print(puzzleNumber + 1);
  Serial.println(" נפתרה!");

  SendCodeDigit(puzzleNumber);
}