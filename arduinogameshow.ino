/*
  WiFi Web Server LED game

  A web game to remember the led flashing sequence.
  This will create a new access point.
  When a device connects to the acess point, a flashing sequence occurs.
  It will then launch a new server and print out the IP address
  to the Serial Monitor. From there, you can open that address in a web browser
  to guess the led flashing sequence.

  James Walter
 */

#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <LiquidCrystal.h>

char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;           // your network key index number (needed only for WEP)
const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int led = LED_BUILTIN;
int led1 = 14;
int led2 = 15;
int led3 = 16;
int led4 = 17;
int led5 = 18;
int redPin = 19;    // Red RGB pin -> D3

String rS = "";
int status = WL_IDLE_STATUS;
WiFiServer server(80);
int correct = 0;

void setup() {
  //Initialize serial and wait for port to open:
  lcd.begin(16, 2);
  Serial.begin(9600);
  randomSeed(analogRead(0));
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Access Point Web Server");

  pinMode(led, OUTPUT);  // set the LED pin mode
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(redPin, OUTPUT);


  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // by default the local IP address will be 192.168.4.1
  // you can override it with the following:
  // WiFi.config(IPAddress(10, 0, 0, 1));

  // print the network name (SSID);
  Serial.print("Creating access point named: ");
  Serial.println(ssid);

  // Create open network. Change this line if you want to create an WEP network:
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // don't continue
    while (true)
      ;
  }

  // wait 10 seconds for connection:
  delay(10000);

  // start the web server on port 80
  server.begin();

  // you're connected now, so print out the status
  printWiFiStatus();
}


void loop() {

  // compare the previous status to the current status
  if (status != WiFi.status()) {
    // it has changed update the variable
    status = WiFi.status();

    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      Serial.println("Device connected to AP");
      rS = randomSequence(10);
      correct = 0;
      game(rS);
      Serial.println("New random " + rS);
    } else {
      // a device has disconnected from the AP, back to listening mode
      Serial.println("Device disconnected from AP");
    }
  }

  WiFiClient client = server.available();  // listen for incoming clients

  if (client) {                    // if you get a client,
    Serial.println("new client");  // print a message out the serial port
    String currentLine = "";       // make a String to hold incoming data from the client
    // String for user's LED sequence
    String ledSequence = "";
    
    while (client.connected()) {  // loop while the client's connected
      delayMicroseconds(20);                                 
      if (client.available()) {                              // if there's bytes to read from the client,
        char c = client.read();                              // read a byte, then
        ledSequence += c;
        Serial.write(c);  // print it out to the serial monitor

        if (c == '\n' && currentLine.length() == 0) {
          // An HTTP request ends with a blank line

          handleHTTPRequest(client, ledSequence, rS);
          ledSequence = "";
          break;
        }
        if (c == '\n') {
          currentLine = "";
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void sendHttpResponse(WiFiClient client) {
  // Send an HTTP response with a form to enter an LED sequence
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  client.println("<html><body>");
  client.println("<h1>Guess the LED Sequence</h1>");
  client.println("<form method=\"GET\">");
  client.println("Enter LED sequence: <input type=\"text\" name=\"led_sequence\"><br>");
  client.println("<input type=\"submit\" value=\"Submit\">");
  client.println("</form>");
  client.println("</body></html>");
}

void handleHTTPRequest(WiFiClient &client, String &request, String &rS) {
  // Check if the HTTP GET request is for the LED sequence

  int idx = request.indexOf("/?led_sequence=");
  if (idx != -1) {
    int endIdx = request.indexOf(' ', idx);
    if (endIdx != -1) {
      String sequence = request.substring(idx + 15, endIdx);
      compareSequence(rS, sequence);
      Serial.println("comparing " + rS + " and " + sequence);
    }
  }

  sendHttpResponse(client);
}

void compareSequence(String rS, String sequence) {
  int counter = 0;
  for (int i = 0; i < sequence.length(); i++) {
    if (rS[i] == sequence[i]) {
      counter += 1;
    }
  }
  if (counter != 0 && counter == sequence.length()) { //for winner buzzer
    for (int i = 0; i < 10; i++) {
      digitalWrite(redPin, HIGH);  // GET /H turns the LED on
      delay(random(200));
      digitalWrite(redPin, LOW);  // GET /L turns the LED off
      delay(random(200));
    }
    lcd.setCursor(0, 0);
    lcd.print("Correct Sequence:");
    lcd.setCursor(0, 1);
    lcd.print(rS);
    correct = 1;
  } else {
    if (correct != 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Try Again");
    }

  }
  
}



String randomSequence(int length) {
  char rS[length] = {'\0'};
  int r = random(5);
  for (int i = 0; i < length; i++) {
    switch (r) {
        case 0:
          rS[i] = 'W';
          break;
        case 1:
          rS[i] = 'R';
          break;
        case 2:
          rS[i] = 'Y';
          break;
        case 3:
          rS[i] = 'G';
          break;
        case 4:
          rS[i] = 'B';
          break;
    }
    r = random(5);
  }
  return rS;
}

void show(char x) {
  int y = 300;
  if (x == 'W') {
    digitalWrite(led1, HIGH);  // GET /H turns the LED on
    digitalWrite(led, HIGH);  // GET /H turns the LED on
    delay(y);
    digitalWrite(led1, LOW);  // GET /L turns the LED off
    digitalWrite(led, LOW);  // GET /H turns the LED on
    delay(y);
  }
  if (x == 'R') {
    digitalWrite(led2, HIGH);  // GET /H turns the LED on
    digitalWrite(led, HIGH);  // GET /H turns the LED on
    delay(y);
    digitalWrite(led2, LOW);  // GET /L turns the LED off
    digitalWrite(led, LOW);  // GET /H turns the LED on
    delay(y);
  }
  if (x == 'Y') {
    digitalWrite(led3, HIGH);  // GET /H turns the LED on
    digitalWrite(led, HIGH);  // GET /H turns the LED on
    delay(y);
    digitalWrite(led3, LOW);  // GET /L turns the LED off
    digitalWrite(led, LOW);  // GET /H turns the LED on
    delay(y);
  }
  if (x == 'G') {
    digitalWrite(led4, HIGH);  // GET /H turns the LED on
    digitalWrite(led, HIGH);  // GET /H turns the LED on
    delay(y);
    digitalWrite(led4, LOW);  // GET /L turns the LED off
    digitalWrite(led, LOW);  // GET /H turns the LED on
    delay(y);
  }
  if (x == 'B') {
    digitalWrite(led5, HIGH);  // GET /H turns the LED on
    digitalWrite(led, HIGH);  // GET /H turns the LED on
    delay(y);
    digitalWrite(led5, LOW);  // GET /L turns the LED off
    digitalWrite(led, LOW);  // GET /H turns the LED on
    delay(y);
  }
}

void game(String ret) {
  for (int i = 0; i < ret.length(); i++) {
    show(ret[i]);
  }
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
