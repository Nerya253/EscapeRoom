#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DIYables_4Digit7Segment_74HC595.h>

#define SCLK D5  
#define RCLK D6 
#define DIO D7  

#define LOCKING_PIN D4  

DIYables_4Digit7Segment_74HC595 display(SCLK, RCLK, DIO);

IPAddress apIP(12, 34, 56, 78);

const char* ssid = "Project";
const char* password = "88888888";

String gamePassword = "";
unsigned long wifiMillisTime = 0;

ESP8266WebServer server(80);

void handleNotFound() {
  String message = "File Not Found \n \n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}


void handleMissionComplited() {
Serial.println("GET");
  if(server.hasArg("gameOn")){
    digitalWrite(LOCKING_PIN, LOW);
  }
  if (server.hasArg("missionCode") && gamePassword.length() < 4) {
    if (server.arg("missionCode").length() == 1) {
      gamePassword = gamePassword + server.arg("missionCode");
      display.printInt(gamePassword.toInt(), true);
      server.send(200, "text/plain", "ok");
    }
    server.send(404, "text/plain", "password is too long");
  } else {
    server.send(404, "text/plain", "error");
  }
}

void wifiSetup() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);

  server.on("/", handleRoot);
  server.on("/api", handleMissionComplited);
  server.onNotFound(handleNotFound);

  Serial.print("AP IP address: ");
  Serial.println(apIP);
  pinMode(LOCKING_PIN, OUTPUT);
  wifiMillisTime = millis();
  display.clear();
  display.printInt(0, true);
  server.begin();
}

void wifiLoop() {
  if (millis() - wifiMillisTime >= 10) {
    wifiMillisTime = millis();
    server.handleClient();
  }
  display.loop();
}


void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="he">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>חדר בריחה</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Heebo:wght@300;400;500;700&display=swap');
        
        :root {
            --primary: #11a8cd;
            --primary-dark: #0c85a3;
            --accent: #ff5722;
            --error: #f44336;
            --success: #00c853;
            --dark: #121212;
            --light-text: #ffffff;
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            margin: 0;
            background-color: #000;
            font-family: 'Heebo', sans-serif;
            overflow: hidden;
            position: relative;
        }
        
        .background-container {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            z-index: -1;
            overflow: hidden;
        }
        
        .background-overlay {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0, 0, 0, 0.6);
            z-index: 1;
        }
        
        .background-image {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            filter: blur(3px);
            transform: scale(1.1);
            z-index: 0;
            background-color: #0d1117;
        }
        
        .cyberpunk-grid {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background-image: linear-gradient(rgba(18, 16, 16, 0) 2px, transparent 2px),
                              linear-gradient(90deg, rgba(18, 16, 16, 0) 2px, transparent 2px);
            background-size: 40px 40px;
            background-position: -2px -2px;
            z-index: 1;
            opacity: 0.4;
        }
        
        .container {
            position: relative;
            width: 90%;
            max-width: 500px;
            z-index: 10;
        }
        
        .game-console {
            background: rgba(10, 10, 10, 0.85);
            backdrop-filter: blur(15px);
            border-radius: 12px;
            border: 1px solid rgba(17, 168, 205, 0.3);
            box-shadow: 0 20px 50px rgba(0, 0, 0, 0.8),
                        0 0 0 1px rgba(255, 255, 255, 0.1),
                        inset 0 0 30px rgba(17, 168, 205, 0.2);
            padding: 2.5rem;
            position: relative;
            overflow: hidden;
        }
        
        .game-console::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 5px;
            background: linear-gradient(90deg, var(--primary), var(--accent));
            z-index: 2;
        }
        
        .glowing-edges {
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            border: 1px solid rgba(17, 168, 205, 0.2);
            border-radius: 12px;
            box-shadow: 0 0 15px rgba(17, 168, 205, 0.3);
            pointer-events: none;
            z-index: -1;
        }
        
        .header {
            text-align: center;
            margin-bottom: 2rem;
            position: relative;
        }
        
        h2 {
            color: var(--light-text);
            font-size: 1.8rem;
            font-weight: 500;
            text-shadow: 0 0 10px rgba(17, 168, 205, 0.7);
            margin-bottom: 0.5rem;
            letter-spacing: 1px;
        }
        
        .subtitle {
            color: rgba(255, 255, 255, 0.7);
            font-size: 0.9rem;
            font-weight: 300;
            margin-top: 0.5rem;
        }
        
        .lock-icon {
            font-size: 2.5rem;
            color: var(--primary);
            margin-bottom: 1rem;
            animation: glow 2s infinite alternate;
        }
        
        @keyframes glow {
            from {
                text-shadow: 0 0 5px rgba(17, 168, 205, 0.5),
                            0 0 10px rgba(17, 168, 205, 0.5);
            }
            to {
                text-shadow: 0 0 10px rgba(17, 168, 205, 0.8),
                            0 0 20px rgba(17, 168, 205, 0.8),
                            0 0 30px rgba(17, 168, 205, 0.5);
            }
        }
        
        .code-form {
            position: relative;
            margin: 2rem 0;
        }
        
        .input-group {
            position: relative;
            margin-bottom: 1.5rem;
        }
        
        input[type='text'] {
            width: 100%;
            background-color: rgba(255, 255, 255, 0.07);
            color: var(--light-text);
            border: 2px solid rgba(17, 168, 205, 0.3);
            border-radius: 8px;
            font-size: 1.3rem;
            letter-spacing: 3px;
            text-align: center;
            padding: 16px 20px;
            transition: all 0.3s ease;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2),
                        inset 0 2px 5px rgba(0, 0, 0, 0.2);
        }
        
        input[type='text']:focus {
            outline: none;
            border-color: var(--primary);
            box-shadow: 0 0 0 3px rgba(17, 168, 205, 0.25),
                        0 5px 15px rgba(0, 0, 0, 0.2),
                        inset 0 2px 5px rgba(0, 0, 0, 0.2);
        }
        
        input[type='text']::placeholder {
            color: rgba(255, 255, 255, 0.3);
        }
        
        .input-decoration {
            position: absolute;
            top: -10px;
            right: -10px;
            width: 20px;
            height: 20px;
            border-top: 2px solid var(--primary);
            border-right: 2px solid var(--primary);
            opacity: 0.6;
        }
        
        button {
            width: 100%;
            background: linear-gradient(45deg, var(--primary), var(--primary-dark));
            color: white;
            border: none;
            border-radius: 8px;
            padding: 15px 20px;
            font-size: 1.1rem;
            font-weight: 500;
            letter-spacing: 1px;
            cursor: pointer;
            transition: all 0.3s ease;
            margin-top: 1rem;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.3),
                        0 0 0 1px rgba(255, 255, 255, 0.05);
            position: relative;
            overflow: hidden;
            text-transform: uppercase;
        }
        
        button:hover {
            background: linear-gradient(45deg, var(--primary-dark), var(--primary));
            transform: translateY(-3px);
            box-shadow: 0 7px 20px rgba(0, 0, 0, 0.4),
                        0 0 0 1px rgba(255, 255, 255, 0.07);
        }
        
        button:active {
            transform: translateY(1px);
            box-shadow: 0 3px 10px rgba(0, 0, 0, 0.3),
                        0 0 0 1px rgba(255, 255, 255, 0.05);
        }
        
        button::before {
            content: '';
            position: absolute;
            top: 0;
            left: -100%;
            width: 100%;
            height: 100%;
            background: linear-gradient(
                90deg,
                transparent,
                rgba(255, 255, 255, 0.2),
                transparent
            );
            transition: all 0.5s;
        }
        
        button:hover::before {
            left: 100%;
        }
        
        .button-icon {
            margin-left: 10px;
        }
        
        .feedback {
            margin-top: 1.5rem;
            padding: 1rem;
            border-radius: 8px;
            font-weight: 500;
            text-align: center;
            display: flex;
            align-items: center;
            justify-content: center;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
        }
        
        .fail {
            background-color: rgba(244, 67, 54, 0.15);
            color: #ff6b6b;
            border: 1px solid rgba(244, 67, 54, 0.3);
            animation: shake 0.5s cubic-bezier(.36,.07,.19,.97) both;
        }
        
        @keyframes shake {
            0%, 100% { transform: translateX(0); }
            10%, 30%, 50%, 70%, 90% { transform: translateX(-6px); }
            20%, 40%, 60%, 80% { transform: translateX(6px); }
        }
        
        .currect {
            background-color: rgba(0, 200, 83, 0.15);
            color: #69f0ae;
            border: 1px solid rgba(0, 200, 83, 0.3);
            animation: success-pulse 2s infinite;
            font-size: 1.6rem;
            padding: 1.5rem;
        }
        
        @keyframes success-pulse {
            0% { box-shadow: 0 0 0 0 rgba(0, 200, 83, 0.4); }
            70% { box-shadow: 0 0 0 15px rgba(0, 200, 83, 0); }
            100% { box-shadow: 0 0 0 0 rgba(0, 200, 83, 0); }
        }
        
        .feedback-icon {
            margin-left: 10px;
            font-size: 1.2rem;
        }
        
        .scan-line {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 8px;
            background: linear-gradient(
                to bottom,
                rgba(255, 255, 255, 0.1),
                transparent
            );
            animation: scan 8s linear infinite;
            opacity: 0.5;
            pointer-events: none;
        }
        
        @keyframes scan {
            0% { top: 0; }
            100% { top: 100%; }
        }
        
        .status-lights {
            position: absolute;
            top: 15px;
            right: 15px;
            display: flex;
            gap: 8px;
        }
        
        .status-light {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            animation: blink 3s infinite;
        }
        
        .status-light:nth-child(1) {
            background-color: var(--primary);
            animation-delay: 0s;
        }
        
        .status-light:nth-child(2) {
            background-color: var(--accent);
            animation-delay: 1s;
        }
        
        .status-light:nth-child(3) {
            background-color: var(--error);
            animation-delay: 2s;
        }
        
        @keyframes blink {
            0%, 50%, 100% { opacity: 1; }
            25%, 75% { opacity: 0.5; }
        }
        
        @media (max-width: 768px) {
            .container {
                width: 95%;
            }
            
            h2 {
                font-size: 1.5rem;
            }
            
            .subtitle {
                font-size: 0.8rem;
            }
            
            input[type='text'] {
                font-size: 1.1rem;
            }
            
            button {
                font-size: 1rem;
            }
        }
    </style>
</head>
<body dir="rtl">
    <div class="background-container">
        <div class="background-image"></div>
        <div class="background-overlay"></div>
        <div class="cyberpunk-grid"></div>
    </div>
    
    <div class="container">
        <div class="game-console">
            <div class="glowing-edges"></div>
            <div class="status-lights">
                <div class="status-light"></div>
                <div class="status-light"></div>
                <div class="status-light"></div>
            </div>
            
            <div class="scan-line"></div>)rawliteral";

  String form = R"rawliteral(
            <div class="header">
                <h2>מערכת אבטחה</h2>
                <p class="subtitle">הכנס את הקוד הסודי כדי לפתוח את הדלת</p>
            </div>
            
            <form method="get" class="code-form">
                <div class="input-group">
                    <div class="input-decoration"></div>
                    <input type="text" name="password" placeholder="* * * *" maxlength="4" autocomplete="off">
                </div>
                <button type="submit">
                    פתח נעילה
                </button>)rawliteral";

  if (server.hasArg("password")) {
    if (server.arg("password").length() != 4) {
      form += R"rawliteral(
                <div class="feedback fail">
                    <span>הקוד חייב להיות 4 ספרות</span>
                    <span class="feedback-icon">!</span>
                </div>)rawliteral";
    } else {
      if (server.arg("password") == gamePassword) {
        digitalWrite(LOCKING_PIN, HIGH);
        form = R"rawliteral(
            <div class="header">
                <h2>מערכת אבטחה</h2>
            </div>
            
            <form method="get" class="code-form">
                <div class="currect">
                    הצלחת לצאת מהחדר!
                    <br>
                    ניפגש בפעם הבאה
                </div>)rawliteral";
      } else {
        form += R"rawliteral(
                <div class="feedback fail">
                    <span>גישה נדחתה - קוד שגוי</span>
                    <span class="feedback-icon">!</span>
                </div>)rawliteral";
      }
    }
  }

  form += R"rawliteral(
            </form>
        </div>
    </div>
    
    <script>
        document.addEventListener('DOMContentLoaded', function() {
            const codeInput = document.querySelector('input[name="password"]');
            
            if (codeInput) {
                codeInput.addEventListener('focus', function() {
                    document.querySelector('.game-console').style.boxShadow = '0 20px 50px rgba(0, 0, 0, 0.8), 0 0 0 1px rgba(255, 255, 255, 0.1), inset 0 0 30px rgba(17, 168, 205, 0.3)';
                });
                
                codeInput.addEventListener('blur', function() {
                    document.querySelector('.game-console').style.boxShadow = '0 20px 50px rgba(0, 0, 0, 0.8), 0 0 0 1px rgba(255, 255, 255, 0.1), inset 0 0 30px rgba(17, 168, 205, 0.2)';
                });
            }
        });
    </script>)rawliteral";

  html += form;
  html += "</body>";
  html += "</html>";
  server.send(200, "text/html", html);
}

