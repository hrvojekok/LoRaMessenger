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


byte localAddress = 0xAA;
byte destinationAddress = 0xBB;
long lastSendTime = 0;
int interval = 2000;
int count = 0;
String incoming = "";
String helper = "";

// Replace with your network credentials
const char* ssid     = "ESP32 AccessPointNodeA";
const char* password = "nodeAnodeA";

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

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();

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

  incoming = "";

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

            //added
            char c = client.read();
            //end of added

              //read char by char HTTP request
              if (helper.length() < 100) {
      
                //store characters to string
                helper += c;
                Serial.print(c);
              }

  
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println("Connection: close");
      client.println();
      


          client.println("<html>");
          client.println("<body>");

          //client.println("<h1>HTML form GET example</H1>");

          client.println("<form action='/' method=post >"); //uses IP/port of web page

          client.println("Your message: <input type=text name='message' value='' size='25' ><br>");

          //client.println("<script>var url = new URL(url_string); url.searchParams.get(\"c\");</script>");

          client.println("<script> function getQueryVariable(variable){ var query = window.location.search.substring(1); var vars = query.split(\"&\"); for (var i=0;i<vars.length;i++) { var pair = vars[i].split(\"=\"); if(pair[0] == variable){return pair[1];} } return(false); } </script>");
          client.println("<script> getQueryVariable(\"message\"); if(document.getElementById('number1').checked) { document.write(\"<h1>Hello member</h1>\"); }  </script>");

          
          //client.println("var url = new URL(url_string);");
          //client.println("url.searchParams.get(\"c\");");
          
          client.println("<input type=submit name='submit' value='Send'>");

          client.println("</form>");

          client.println("<br>");

          client.println("</body>");
          client.println("</html>");

          delay(1);


          
      
//      // Web Page Heading
//      client.println("<html>");
//      client.println("<body><h1>ESP32 Web Server Node A</h1>");
//      client.println("<h2>" + incoming + "</h2>");
//      //client.write("<h2>asd" + client.read() + "</h2>");
//      client.write("<form>");
//      client.write("<input type=\"text\"><br><br>");
//      client.write("<input type=\"submit\" value=\"Submit\">");
//      client.write("</form>");
//      //client.write(client.read(client.println("sd")));
//  
//      client.println("</body></html>");
      
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
