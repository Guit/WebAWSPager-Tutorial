#include <Arduino.h>
#include <LiquidCrystal.h>
#include <WiFi.h> // In order to manage the connection to the wifi network, and the wifi access point mode
#include <ESPAsyncWebServer.h> // The webserver we will use to display the wifi selection web page
#include <SPIFFS.h> // In Order to use Serial Peripheral Interface Flash File System
const int rs = 23, en = 22, d4 = 5, d5 = 18 , d6 = 19, d7 = 21;
LiquidCrystal lcd(rs,en,d4,d5,d6,d7);
// We create a function in order to easily write on the screen

void writeOnLCD(String line1, String line2)
{
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}


/**
 * Access Point
 * */
String defaultSSID = "MyESP32"; // Name Of the wifi network of the esp32
String defaultPassword = "123456789"; // password of the network
const int ACCESSPOINT = 0; // Mode Access Point
const int WIFICLIENT = 1; // Mode Client
const int MAXWIFIATTEMPT = 5; // Number of attempt in case of difficulties to connect

/**
 * This function is used to start the wifi :
 * - at the beginning as a wifi accessPoint,
 * - when the user as choosen a wifi network, as a client
 * */
bool setupWIFI(String ssid, String password,int MODE)
{
  WiFi.disconnect();
  IPAddress IP;
  String error="";
  if (MODE == ACCESSPOINT){ // access Point Mode
    WiFi.mode(WIFI_MODE_AP);
    WiFi.disconnect();
    WiFi.softAP(ssid.c_str(), password.c_str()); // Use the Wifi name and password to create a network.
    IP = WiFi.softAPIP();
  }else
  {
    writeOnLCD("Trying to connect on", ssid);
    WiFi.begin(ssid.c_str(), password.c_str());
    uint8_t i = 0;
    
    while (WiFi.status() != WL_CONNECTED && i<MAXWIFIATTEMPT) // 5 attent to connect
    {
      Serial.print('.');
      delay(2000);
      i++;  
    }
    /**
     * 
     * */
    if (i==MAXWIFIATTEMPT){
      writeOnLCD("Connexion Failed","AP mode in 5s");
      delay(5000);
      setupWIFI(defaultSSID,defaultPassword,ACCESSPOINT);
      return false;
    }
    // TODO: Add EEPROM Storage : We don't store the wifi network / password inside the ESP32 Memory

    IP = WiFi.localIP();
  }

  char ipChar[16];
  sprintf(ipChar, "%d.%d.%d.%d", IP[0], IP[1], IP[2], IP[3]); // We get the IP adress of the ESP32
  writeOnLCD(ssid, ipChar); // We write it on the LCD Screen
  return true;
}
  /**
 * this function is used to scan all the network available in order to display it on the web page.
 * */
String *availableWifi;
int nbAvailableWifi = 0;
void initAvailableWIFI()
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  nbAvailableWifi = WiFi.scanNetworks();
  availableWifi = new String[nbAvailableWifi];
  for (int i = 0; i < nbAvailableWifi; i++)
  {
    availableWifi[i] = WiFi.SSID(i);
    Serial.println(WiFi.SSID(i));
  }
}
   
/*
 Now it's time to start the webserver in order to be able to serve the page that will allow us to connect to the right wifi
 */

/**
 * Simple HTML Server
 * */
AsyncWebServer server(80); // The server will run on the 80 port, the standard HTTP port

/**
 * The processor function is USE by ESP Async Server :
 * The list of available wifi network will be discover at the runtime. 
 * So when the Webserver will display the page to the user, it will process the HTML page and replace the %SSID% 
 * variable by the list of wifi. 
 * */
String processor(const String &var)
{
  Serial.println(var);
  String toReturn = "";
  if (var == "SSID") // the variable to replace by the list
  {
    for (int i = 0; i < nbAvailableWifi; i++)
    {
      toReturn = toReturn + "<OPTION>" + availableWifi[i] + "</OPTION>"; // we use the html syntax to complete the list.
    }

    Serial.print(toReturn);
    return toReturn;
  }
  return String();
}
/** Server start
 * we will use the SPIFFS storage to store the HTML File we have to serve.
 * in order to deploy the HTML in the SPIFFS we create a data folder in the project, and put the file inside
 * */
void startServer()
{
  if (!SPIFFS.begin(true)) 
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { // when we arrive on the root of the server we will serve the index.html file
    Serial.println("Received request");
    request->send(SPIFFS, "/index.html", String(), false, processor); // the server send the content of the index.html file, from the SPIFFS,
                                                                      // and use the processor function to complete the wifi list before sending the result.
  });
  server.on("/setSSID", HTTP_POST, [](AsyncWebServerRequest *request) { // this endpoint is called by the index.html file, by the form.
                                                                        // it is this endpoint that take all the parameters to initiate a Wifi connection
    Serial.println("Received submit");
    
    if (request->hasParam("ssid", true) && request->hasParam("password", true))
    {
      AsyncWebParameter *ssid = request->getParam("ssid", true); // this method allows us to get the parameters send by the HTML Forms
      AsyncWebParameter *password = request->getParam("password", true);
      Serial.println(password->value());
      Serial.println(ssid->value());
      if (setupWIFI(ssid->value(),password->value(),WIFICLIENT)){ //we take all the parameters to initiate a new wifi connection.
             Serial.println("Success"); // 
      };
      request->send(SPIFFS, "/index.html", String(), false, processor);

    }
    
  });
  server.begin();
}

void setup() {
  Serial.begin(115200); // define the speed in order to be able to read the monitoring
  writeOnLCD("Scanning", "Available WIFI");
  initAvailableWIFI();
  writeOnLCD("Starting the", "Access Point Mode");
  setupWIFI(defaultSSID, defaultPassword, ACCESSPOINT); // start the access point mode
  startServer();//start the server
}

void loop() {
  // put your main code here, to run repeatedly:
}