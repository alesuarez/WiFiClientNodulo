#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <ESP8266HTTPClient.h>
#define ESP8266_LED_GPIO2   2 // Blue LED.
#define ESP8266_RELAY_GPIO4 4 // Relay control.
#define ESP8266_OPTO_GPIO5  5 // Optocoupler input.
#define LED_PIN             ESP8266_LED_GPIO2
#define LED_ON              LOW
#define LED_OFF             HIGH
#define FIELD_ALARM         2
#define ALARM_OFF           0
#define timeSeconds         40

/* Set these to your desired credentials. */
const char *ssid = "Hidden";
const char *password = "123456@A";
int interruptRising = 0;
int interruptFalling = 0;
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change :
const long interval = 5000;           // interval at which to blink (milliseconds)
int ledState = LOW;             // ledState used to set the LED

unsigned long myChannelNumber = 951326;
const char * myWriteAPIKey = "QN5G2HIYA4VH0X2R";
const char * myReadAPIKey = "3X9TSMYX2NTI70OH";
WiFiClient  client;
int alarmField = 0;
unsigned long lastTrigger = 0;
boolean startTimer = false;
boolean alarmStatus = false;
boolean trigger = false;
void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  Serial.println("Configuring GPIO...");  
  pinMode( ESP8266_RELAY_GPIO4, OUTPUT );       // Relay control pin.
  pinMode( ESP8266_OPTO_GPIO5, INPUT_PULLUP ); // Input pin.
  pinMode( LED_PIN, OUTPUT );             // ESP8266 module blue LED.
  digitalWrite( ESP8266_RELAY_GPIO4, LOW );       // Set relay control pin low.
  digitalWrite( LED_PIN, HIGH );          // Turn off LED.
  Serial.println("Finish to config GPIO.");
    
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    
    alarmField = readChannelField(FIELD_ALARM);
    
    Serial.print("Lectura Alarma Field2: ");
    Serial.println(alarmField);
  }
  
  alarmStatus = intToBoolean(alarmField);
  
  if (alarmStatus) {
    if (!trigger) {
      turnOnRelay(true);
      startTimer = true;
      lastTrigger = millis();
      trigger = true;
    }
  } else {
    turnOnRelay(false);
    startTimer = false;
    trigger = false;
  }
  
  currentMillis = millis();
  
  if(startTimer && (currentMillis - lastTrigger > (timeSeconds*1000))) {
    Serial.println("Motion stopped...");
    turnOnRelay(false);
    startTimer = false;
    trigger = false;
    writeChannelField(FIELD_ALARM, ALARM_OFF);
    alarmField = 0;
  }
}

void turnOnRelay(boolean turnOn) {
  if (turnOn) {
   
    digitalWrite(LED_PIN, LED_ON);
    digitalWrite( ESP8266_RELAY_GPIO4, HIGH );
  } else {
    
    digitalWrite( ESP8266_RELAY_GPIO4, LOW );
    digitalWrite(LED_PIN, LED_OFF);
  }
}

boolean intToBoolean(int i) {
  return i > 0 ? true : false;
}

int readChannelField(int field) {
  return ThingSpeak.readIntField(myChannelNumber, field, myReadAPIKey);
}

int writeChannelField(int field, int value) {
  int x = ThingSpeak.writeField(myChannelNumber, field, value, myWriteAPIKey);
  
  if(x == 200){
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  
  return x;
}

