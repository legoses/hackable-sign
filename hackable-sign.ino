#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>



// LED Matrix Display configuration
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 15

/*
  PA_PRINT,
  PA_SCAN_HORIZ,
  PA_SCROLL_LEFT,
  PA_WIPE,
  PA_SCROLL_UP_LEFT,
  PA_SCROLL_UP,
  PA_OPENING_CURSOR,
  PA_GROW_UP,
  PA_MESH,
  PA_SCROLL_UP_RIGHT,
  PA_BLINDS,
  PA_CLOSING,
  PA_RANDOM,
  PA_GROW_DOWN,
  PA_SCAN_VERT,
  PA_SCROLL_DOWN_LEFT,
  PA_WIPE_CURSOR,
  PA_DISSOLVE,
  PA_OPENING,
  PA_CLOSING_CURSOR,
  PA_SCROLL_DOWN_RIGHT,
  PA_SCROLL_RIGHT,
  PA_SLICE,
  PA_SCROLL_DOWN,
*/


MD_Parola Display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Default text displayed
String text = "~~ HACK THE PLANET!!!";

// WiFi credentials for accessing the web server
const char* http_username = "admin";
const char* http_password = "31337";


ESP8266WebServer server(80);

// WiFi network settings for the Access Point (AP) mode
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 9);
IPAddress subnet(255, 255, 255, 0);


// Track the number of unique connections since last reset
int unique_connections = 0; 
bool new_device_connected = false;

// Track the unique MAC addresses of connected clients
const int max_clients = 10;
String unique_macs[max_clients];
int num_unique_macs = 0;



// Function to check for new device connections and update the unique connections count
void checkForNewConnections() {
  int current_clients = WiFi.softAPgetStationNum();

  // Check if there's a new device connection and update the unique_connections count
  if (current_clients > num_unique_macs) {
    new_device_connected = true;
    unique_connections++;
    num_unique_macs = current_clients;
  }
}


// Handler for the root page
void handleRoot() {
  // Check for authentication, and send an authentication request if user hasn't authenticated
  if (!server.authenticate(http_username, http_password)) {
    return server.requestAuthentication();
  }

  if (server.hasArg("newtext")) {
    text = server.arg("newtext");
    Display.displayClear(); // Clear the display
    Display.displayScroll(text.c_str(), PA_RIGHT, PA_SCROLL_LEFT, 50); // Scroll the new text
  }

  String webpage = "<html><body>";
  webpage += "<h1>WELCOME TO THE AZCWR WIRELESS SIGN</h1>";
  webpage += "<form action='/' method='post'>"; 
  webpage += "New Text: <input type='text' name='newtext' value='" + text + "'>";
  webpage += "<input type='submit' value='Change'>";
  webpage += "</form>";
  webpage += "<p>Current text displayed: " + text + "</p>";
  webpage += "<!-- made by p0st with <3 -->";
  webpage += "</body></html>";

  // Send the HTML page as the response
  server.send(200, "text/html", webpage);
}


// Setup function runs once on device startup
void setup() {
  const char* ssid = "hack-this-sign"; // WiFi network name (SSID)
  const int max_connections = 4; // Maximum number of simultaneous connections

  // Set the ESP8266 in Access Point (AP) mode
  // WiFi.softAP(ssid, passwd); // If you'd like to include a password for the AP

  // Initialize Serial communication for debugging
  Serial.begin(9600);
  Serial.println();

  // Configure the Access Point settings
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  // Start the Access Point
  Serial.print("Setting soft-AP ... ");
  Serial.println(WiFi.softAP(ssid) ? "Ready" : "Failed!");

  // Print the Access Point IP address to the Serial Monitor
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

  // Initialize LED matrix display
  Display.begin();
  Display.setIntensity(15); // Change brightness from 0 to 15
  Display.displayClear();
  Display.displayScroll(text.c_str(), PA_RIGHT, PA_SCROLL_LEFT, 50);


  // ROUTES:

  // Route for serving the website homepage
  server.on("/", handleRoot);

  // Start the web server
  server.begin();

}



// Loop function runs repeatedly after setup
void loop() {

  // Check for new device connections and update the unique connections count
  checkForNewConnections();

  // Handle incoming client requests
  server.handleClient();

  // If the LED matrix animation is finished, reset it
  if (Display.displayAnimate()) {
    Display.displayReset();
  }
}

