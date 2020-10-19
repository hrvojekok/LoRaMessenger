#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include <WiFi.h>
#include <string>
#include <sstream>

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()
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

//webpage begin
/*const char Web_page[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html>
  <style>
    .displayobject{
       font-family: sans-serif;
       margin: auto;
       text-align: center;
       width: 50%;
       border: 3px solid #000000;
       padding: 10px;
       background: #558ED5;
    }
    h1 {
      font-size: 36px;
      color: white;
    }
    h4 {
      font-size: 30px;
      color: yellow;
    }
  </style>
  <body>
     <div class = "displayobject">
       <h1>Update only webpage Values - no refresh needed!</h1><br>
       <h4>Temperature reading: <span id="TEMPvalue">0</span>&deg</h4>
       <h4>Humidity reading: <span id="HUMIvalue">0</span>%</h4>
       <h4>Pressure reading: <span id="PRESvalue">0</span>hPa</h4><br>
     </div>
     <script>
       setInterval(function() {getSensorData();}, 1000); // Call the update function every set interval e.g. 1000mS or 1-sec
  
       function getSensorData() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("TEMPvalue").innerHTML = this.responseText;
          }
        };
        xhttp.open("GET", "TEMPread", true);
        xhttp.send();
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("HUMIvalue").innerHTML = this.responseText;
          }
        };
        xhttp.open("GET", "HUMIread", true);
        xhttp.send();
        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("PRESvalue").innerHTML = this.responseText;}
        };  
        xhttp.open("GET", "PRESread", true);
        xhttp.send(); 
      }
    </script>
  </body>
</html>
)=====";
//webpage end

void handleRoot() {
  //String s = Web_page;             //Display HTML contents
  server.send(200, "text/html", Web_page); //Send web page
}

void handleTEMP() { // This function is called by the script to update the sensor value, in this example random data!
  // temp = String(bme.readTemperature(),2);
  // server.send(200, "text/plain", temp));
  server.send(200, "text/plain", String((float)random(195,209)/10,2)); //Send sensor reading when there's a client ajax request
}

void handleHUMI() { // This function is called by the script to update the sensor value, in this example random data!
  server.send(200, "text/plain", String((float)random(501,523)/10,2)); //Send sensor reading when there's a client ajax request
}
*/


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

  //start
  //server.on("/", handleRoot);         // This displays the main webpage, it is called when you open a client connection on the IP address using a browser
  //server.on("/TEMPread", handleTEMP); // To update Temperature called by the function getSensorData
  //server.on("/HUMIread", handleHUMI); // To update Humidity called by the function getSensorData
  //server.on("/PRESread", handlePRES); // To update Pressure called by the function getSensorData
  //----------------------------------------------------------------
 //end
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

String convertAsciiToString(int ascii){
  String stringFromAscii;
  stringFromAscii = (char) ascii;
  return stringFromAscii;
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
            Serial.println(c);
            header += c;
            //end of added

              //read char by char HTTP request
              if (helper.length() < 100) {
      
                //store characters to string
                helper += c;
                Serial.println(c);
                Serial.println(header);
              }


  if(client.find("message=")){
    
    //char* temp;
    Serial.println("found message");
    int index = 0;
    do { 
      index = client.read();
      //itoa(index, temp, 10);
      //Serial.println(index);

      //converting int to string
      std::string stringForSending = SSTR("i is: " << index);
      Serial.println(SSTR(index).c_str());

      Serial.println(convertAsciiToString(index));
      sendMessage(convertAsciiToString(index));

      //if(client.find("&submit")){
      //  break;
      //}
      
      stringForSending = "";
      index = 0;      
    } while (client.read() != -1);

   
    
    //int index = client.read();
    //Serial.println(index);
    //Serial.println(temp);
  }


  
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println("Connection: close");
      client.println();
      


          client.println("<html>");
          client.println("<body>");

          //client.println("<h1>HTML form GET example</H1>");

          client.println("<form action='/' method=get >"); //uses IP/port of web page

          client.println("Your message: <input type=text name='message' value='' size='25' ><br>");





          client.println("<script type=\"text/javascript\"> function insertString() { let origString = message; let stringToAdd = \"#\"; for(let i = 0; i < origString.length(); i++){ if(i % 2 === 0){ newString = origString.slice(0, i) + stringToAdd + origString.slice(i); } } } </script> ");

          //client.println(" <script type=\"text/javascript\"> function insertString() { let origString = \"GeeksGeeks\"; let stringToAdd = \"For\"; let indexPosition = 5; newString = origString.slice(0, indexPosition) + stringToAdd + origString.slice(indexPosition); document.querySelector('.outputString').textContent = newString; } </script> ");


          //client.println("<script>var url = new URL(url_string); url.searchParams.get(\"c\");</script>");

          //client.println("<script> function getQueryVariable(variable){ var query = window.location.search.substring(1); var vars = query.split(\"&\"); for (var i=0;i<vars.length;i++) { var pair = vars[i].split(\"=\"); if(pair[0] == variable){return pair[1];} } return(false); } </script>");
          //client.println("<script> getQueryVariable(\"message\"); if(document.getElementById('number1').checked) { document.write(\"<h1>Hello member</h1>\"); }  </script>");

          
          //client.println("var url = new URL(url_string);");
          //client.println("url.searchParams.get(\"c\");");
          
          client.println("<input type=submit name='submit' onclick=\"insertString()\">");

          client.println("</form>");

          client.println("<br>");

          client.println("</body>");
          client.println("</html>");

          delay(1);


char help;
if(header.length() >= 1){
  help = client.read();
  Serial.print(help);
  Serial.println("help");
}
          
      
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
