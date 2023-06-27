const char loginPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            text-align: center;
            background-color: #000;
            color: #fff;
        }

        .container {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
        }

        h2 {
            margin-bottom: 20px;
        }

        .form-group {
            margin-bottom: 20px;
        }

        .form-group label {
            display: block;
            margin-bottom: 5px;
        }

        .form-group input {
            width: 250px;
            padding: 10px;
            border: 1px solid #ccc;
            border-radius: 4px;
            font-size: 16px;
        }

        .form-group button {
            padding: 10px 20px;
            background-color: #4CAF50;
            color: #fff;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
        }
    </style>
</head>

<body>
    <div class="container">
        <h2>Wi-Fi Login</h2>
        <form id="login-form" method="POST">
            <div class="form-group">
                <label for="ssid">Wi-Fi SSID:</label>
                <input type="text" id="ssid" name="ssid" required>
            </div>
            <div class="form-group">
                <label for="password">Wi-Fi Password:</label>
                <input type="password" id="password" name="password" required>
            </div>
            <div class="form-group">
                <button type="submit">Connect</button>
            </div>
        </form>
        <p id="message"></p>
    </div>

    <script>
        document.getElementById("login-form").addEventListener("submit", function (event) {
            event.preventDefault();
            var ssid = document.getElementById("ssid").value;
            var password = document.getElementById("password").value;
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/login");
            xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
            xhr.onreadystatechange = function () {
                if (xhr.readyState === 4) {
                    if (xhr.status === 200) {
                        document.getElementById("message").textContent = "Connected to Wi-Fi successfully!";
                    } else {
                        document.getElementById("message").textContent = "Failed to connect to Wi-Fi. Please try again.";
                    }
                }
            };
            xhr.send("ssid=" + encodeURIComponent(ssid) + "&password=" + encodeURIComponent(password));
        });
    </script>
</body>

</html>
)=====";

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <ESP8266WebServer.h>

#define REPS 20

//WiFiServer server(80);
SoftwareSerial TOF_serial(D5, D0);  // RX, TX
/* Set these to your desired credentials. */
const char *ssid = "TOF";
const char *password = "12345678";
unsigned char TOF_data[32] = {0};
unsigned char TOF_length = 16;
unsigned char TOF_header[3] {0x57, 0x00, 0xFF};
unsigned long TOF_distance = 0;
int avg_distance;

ESP8266WebServer server(80); // ساخت یک شیء از کلاس ESP8266WebServer برای سرور وب

void TOF() {
  if (TOF_serial.available() >= 32) {
    for (int i = 0; i < 32; i++) {
      TOF_data[i] = TOF_serial.read();
    }
    for (int j = 0; j < 16; j++) {
      if ((TOF_data[j] == TOF_header[0] && TOF_data[j + 1] == TOF_header[1] && TOF_data[j + 2] == TOF_header[2])) {
        if (((TOF_data[j + 12]) | (TOF_data[j + 13] << 8)) == 0) {
          //          Serial.println("Out of range!");
        }
        else {
          TOF_distance = (TOF_data[j + 8]) | (TOF_data[j + 9] << 8) | (TOF_data[j + 10] << 16);
          TOF_distance = TOF_distance / 10;
        }
        // break;
      }
    }
  }
}

void handleRoot() {
    TOF();
    int meansonar = 0;
    for (int i = 0; i < REPS; i++) {
        meansonar += TOF_distance ;
        delay(50); // Wait for 50ms before next measurement
    }
    avg_distance = meansonar / REPS;
    String distance = (avg_distance == LOW) ? "?" : String(avg_distance);

    String html = "<!DOCTYPE HTML>";
    html += "<html>";
    html += "<head>";
    html += "<style>";
    html += "body {";
    html += "    font-family: Arial, sans-serif;";
    html += "    text-align: center;";
    html += "    background-color: #000;";
    html += "    color: #fff;";
    html += "}";

    html += ".container {";
    html += "    display: flex;";
    html += "    flex-direction: column;";
    html += "    align-items: center;";
    html += "    justify-content: center;";
    html += "    height: 100vh;";
    html += "}";

    html += "h1 {";
    html += "    color: #333333;";
    html += "}";

    html += ".distance {";
    html += "    font-size: 48px;";
    html += "    margin-top: 20px;";
    html += "}";

    html += ".link {";
    html += "    margin-top: 40px;";
    html += "    color: #4CAF50;";
    html += "    font-size: 16px;";
    html += "    text-decoration: none;";
    html += "}";
    html += "</style>";
    html += "</head>";
    html += "<body>";
    html += "<h1>TOF</h1>";
    html += "<div class=\"container\">";
    html += "<div class=\"distance\">Distance Is: " + distance + " CM</div>";
    html += "<a class=\"link\" href=\"/loginPage\">Connection WIFI</a>";
    html += "</div>";
    html += "</body>";
    html += "</html>";

    server.send(200, "text/html", html);
}

void handleGetDistance() {
  // اینجا کدی که باید در صورت درخواست /getdistance اجرا شود را قرار دهید
  server.send(200, "text/plain", String(avg_distance));
}

void handleloginPage() {
  server.send(200, "text/html", loginPage);
}

void handleLogin() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    // اتصال به شبکه Wi-Fi با استفاده از اطلاعات ورودی کاربر
    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    
    // ارسال پاسخ موفقیت آمیز به کاربر
    server.send(200, "text/plain", "Connected to Wi-Fi successfully!");
  } else {
    // ارسال پاسخ خطا به کاربر
    server.send(400, "text/plain", "Invalid request");
    server.send(200, "text/html", loginPage);
  }
}

void setup() {

  delay(10);
  Serial.begin(115200);
  TOF_serial.begin(115200);
  Serial.println();
  
  // تنظیم مقادیر IP، Gateway و Subnet برای ESP8266
  IPAddress localIP(192, 168, 1, 1);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(localIP, gateway, subnet);
  
  WiFi.softAP(ssid,password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("IP address: ");
  Serial.println(myIP);

  server.on("/", handleRoot); // درخواست روت صفحه
  server.on("/getdistance", handleGetDistance); // درخواست صفحه /getdistance
  server.on("/loginPage", handleloginPage);
  server.on("/login", handleLogin);
  server.begin();
}

void loop() {
TOF();
 int meansonar = 0;
  for (int i = 0; i < REPS; i++) {
   meansonar +=TOF_distance ;
   delay(50); // Wait for 50ms before next measurement
  }
 int avg_distance = meansonar / REPS;
 Serial.print(avg_distance);
 Serial.println("cm");
 server.handleClient(); // پردازش درخواست‌ها
}
