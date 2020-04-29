#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <WiFi.h>
#include "SSD1306.h" 


#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND    915E6 //915E6 or 433E6 - frequency of LoRa 433MHz/915MHz


byte localAddress = 0xBB;
byte destinationAddress = 0xAA;
long lastSendTime = 0;
int interval = 2000;
int count = 0;

// Replace with your network credentials
const char* ssid     = "ESP32 AccessPointNodeB";
const char* password = "nodeBnodeB";

WiFiServer server(80);
String header;

void setup() {
  Serial.begin(9600);

  while (!Serial);
  Serial.println();
  Serial.println("LoRa Receiver Callback");
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
   //LoRa.onReceive(cbk);
  LoRa.receive();
  Serial.println("Initialized");

  
   // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);
  Serial.println(ssid);
  Serial.println(password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();

}

void loop() {
   if (millis() - lastSendTime > interval) {
      String sensorData = String(count++);
      sendMessage(sensorData);
  
      Serial.print("Sending data " + sensorData);
      Serial.print(" from 0x" + String(localAddress, HEX));
      Serial.println(" to 0x" + String(destinationAddress, HEX));
  
      lastSendTime = millis();
      interval = random(2000) + 1000;
    }
  
    receiveMessage(LoRa.parsePacket());

    WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
      Serial.println("New Client.");          // print a message out in the serial port
      String currentLine = "";                // make a String to hold incoming data from the client
  
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println("Connection: close");
      client.println();
      
  
      
      // Web Page Heading
      client.println("<body><h1>ESP32 Web Server Node A</h1>");
      
  
      client.println("</body></html>");
      
      // The HTTP response ends with another blank line
      client.println();
  
      
      while (client.connected()) {            // loop while the client's connected
  
      // Clear the header variable
      header = "";
      // Close the connection
      client.stop();
      Serial.println("Client disconnected.");
      Serial.println("");
    }
  }
}


void sendMessage(String outgoing) {
  LoRa.beginPacket();
  LoRa.write(destinationAddress);
  LoRa.write(localAddress);
  LoRa.write(outgoing.length());
  LoRa.print(outgoing);
  LoRa.endPacket();
}

void receiveMessage(int packetSize) {
  if (packetSize == 0) return;

  int recipient = LoRa.read();
  byte sender = LoRa.read();
  byte incomingLength = LoRa.read();

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {
    Serial.println("Error: Message length does not match length");
    return;
  }

  if (recipient != localAddress) {
    Serial.println("Error: Recipient address does not match local address");
    return;
  }

  Serial.print("Received data " + incoming);
  Serial.print(" from 0x" + String(sender, HEX));
  Serial.println(" to 0x" + String(recipient, HEX));
}
