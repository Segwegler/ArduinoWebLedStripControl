/*
 * ESP8266 NodeMCU LED Control over WiFi Demo
 * Using Static IP Address for ESP8266
 * https://circuits4you.com
 */

#include <Adafruit_NeoPixel.h>
#define LED_PIN    14
#define LED_COUNT 100
#define STATICIP 1

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

//colors
//Green
const uint32_t green = strip.Color( 0 ,255, 0 );
const uint32_t  red  = strip.Color(255, 0 , 0 );
const uint32_t blue = strip.Color( 0 , 0 ,255);


#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include <FS.h>

//ESP Web Server Library to host a web page
#include <ESP8266WebServer.h>

const char* file = "/index.html";

//Static IP address configuration
#ifdef STATICIP
IPAddress staticIP(192, 168, 227, 100); //ESP static ip
IPAddress gateway(192, 168, 227, 1);   //IP Address of your WiFi Router (Gateway)
IPAddress subnet(255, 255, 255, 0);  //Subnet mask
IPAddress dns(8, 8, 8, 8);  //DNS

#endif


const char* deviceName = "LED";

//On board LED Connected to GPIO2
#define LED 2  

//SSID and Password of your WiFi router
const char* ssid = "ssid";
const char* password = "password";

//Declare a global object variable from the ESP8266WebServer class.
ESP8266WebServer server(80); //Server on port 80

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
String readFile(String fName){
 String s = "";
 File f = SPIFFS.open(file, "r");
 while(f.available()){
  s += char(f.read());
 }
 f.close();
  return s;
}

void sendMainPage(String m=""){
  
 String s = readFile("/index.html");
 if(m){
  s+= "<script>console.log("+m+");</script>";
 }
 server.send(200, "text/html", s); //Send web page
 
}

void handleRoot() {
 Serial.println("You called root page");
 sendMainPage();
 
}

void handleLEDon() { 
 Serial.println("LED on page");
 digitalWrite(LED,LOW); //LED is connected in reverse
 sendMainPage();
 strip.fill(green, 0, LED_COUNT); // Green
 strip.show();
}

void handleLEDoff() { 
 Serial.println("LED off page");
 digitalWrite(LED,HIGH); //LED off
 sendMainPage();
 strip.clear();
 strip.show();
}

void handleColor(){
  String s = "";

  if(server.args() < 3){

    sendMainPage("bad args");
    return;
  }
  //pulls the values sent from the webpage
  for(int i=0; i< server.args(); i++){
    s+= "Arg: " + (String)i + " -> ";
    s+= server.argName(i)+": ";
    s+= server.arg(i) +"\n";
    
  }
  //sets color values
  int r = server.arg(0).toInt();
  int g = server.arg(1).toInt();
  int b = server.arg(2).toInt();

  strip.fill(strip.Color(r,g,b), 0, LED_COUNT);
  strip.show();
  String msg = 'r: '+server.arg(0)+' g: '+server.arg(1)+' b: '+server.arg(2);
  sendMainPage(msg);
  Serial.println("Color: \n" + s);
}

void handleNotFound(){
  String s = "";

  for(int i=0; i< server.args(); i++){
    s+= "Arg: " + (String)i + " -> ";
    s+= server.argName(i)+": ";
    s+= server.arg(i) +"\n";
    
  }
  sendMainPage();
  Serial.println("Not Found: \n" + s);
}

//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  Serial.begin(115200);

  SPIFFS.begin();
  Serial.println("File System Initialized!");  
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  //Onboard LED port Direction output
  pinMode(LED,OUTPUT); 
  //Power on LED state off
  digitalWrite(LED,HIGH);

  WiFi.disconnect();  //Prevent connecting to wifi based on previous configuration
  WiFi.hostname(deviceName);      // DHCP Hostname (useful for finding device for static lease)
  
  #ifdef STATICIP
  WiFi.config(staticIP, subnet, gateway, dns);
  #endif
  
  WiFi.begin(ssid, password);

  WiFi.mode(WIFI_STA); //WiFi mode station (connect to wifi router only
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
 
  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.on("/on", handleLEDon); //as Per  <a href="ledOn">, Subroutine to be called
  server.on("/off", handleLEDoff);
  server.on("/color", handleColor);
  server.onNotFound(handleNotFound);
  server.begin();                  //Start server
  Serial.println("HTTP server started");


    strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  //strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(100); // Set BRIGHTNESS to about 1/5 (max = 255)
}
//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  server.handleClient();          //Handle client requests
}


// Fill strip pixels one after another with a color. Strip is NOT cleared
// first; anything there will be covered pixel by pixel. Pass in color
// (as a single 'packed' 32-bit value, which you can get by calling
// strip.Color(red, green, blue) as shown in the loop() function above),
// and a delay time (in milliseconds) between pixels.
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}
