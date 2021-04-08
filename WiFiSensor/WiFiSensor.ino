/****************************************************************************************************************************************************
 *  TITLE: Code for ESP32 with SoilMoisture Sensor and Barometer
 *  DESCRIPTION: Getting the Data every 6 Hours and send it via MQTT to an RabbitMQ instance.
 *
 *  By Martin Hüsgen
 ****************************************************************************************************************************************************/

#include <WiFi.h>
#include <PubSubClient.h>
#include "credentials.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme; // I2C


// Maybe a Config file would make sense for the basic stuff, instead of flooding the header here :)
//Conversion factor for micro seconds to seconds
#define uS_TO_S_FACTOR 1000000ULL  

//Time ESP32 will go to sleep (in seconds)
#define TIME_TO_SLEEP  36000  // 600 minutes = 10hours  

// trys to reconnect to the Wifi
#define WIFI_RETRIES = 20;

// "Max Values" for the Moisture Sensor 
// Test this with every Sensor, each behaves a littlebit diffrent
// Test it in the "air" for AirValue and put into water for Watervalue
const int AirValue = 4095;   // 0% Moisture Value
const int WaterValue = 2736;  // 100% Moisture Value

// The Number of the Sensor that is transmitted in the MQTT Payload
const char* SensorNumber = "3"; // Change for every Sensor

// Define the LED PIN and the PIN where the Analog values is sent to
// Choose freely for you device
#define LED_PIN   2
#define ANALOG_PIN 36

// Variables Init
int soilMoistureValue = 0;
int soilmoisturepercent=0;

// Payload Variables
char str[120];
char payload[200];
char cAirTemp[10];
char cAirMoist[10];
char cAirPressure[10];
String sPayload;

// Connection Tries to connect to the MQTT client, just a counter!
int conn_tries = 0;

// Connection Details taken from credtials.h
// Change the Values there, not here!
const char* ssid = networkSSID;
const char* password = networkPASSWORD;
const char* mqttServer = mqttSERVER;
const char* mqttUsername = mqttUSERNAME;
const char* mqttPassword = mqttPASSWORD;

// MQTT Topic definition
char pubTopic[] = "home/sensordata1";       //payload[0] will have ledState value


WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Init
void setup() 
{
  pinMode(LED_PIN, OUTPUT);     
  Serial.begin(115200);
  delay(1000);
  bool communication = bme.begin(0x76);
  if (!communication) {
    Serial.println("Could not find a valid BME280 sensor");
    Serial.println("check wiring, address, sensor ID!");
    Serial.print("SensorID was: 0x");
    Serial.println(bme.sensorID(), 16);
    Serial.println("ID of 0xFF probably means a bad address\n");
    while (true) { };
    delay(10);
  }
  else {
    Serial.println("Communication established!\n");
  }
  WiFi_Start();
  print_wakeup_reason();
}

// Discconect the WIFI
void WiFi_Off() {
  client.disconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  Serial.print("Wifi Disconnected");
}

// Start Wifi on the Chip and Connect to the WIFI
void WiFi_Start() 
{
  Serial.print("Start Wifi Connect");
  delay(10);
  WiFi.mode(WIFI_STA);
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Waiting for the Connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    conn_tries++;
    Serial.println(conn_tries);
    Serial.println(WIFI_RETRIES);
    if (conn_tries > WIFI_RETRIES)
    {
      // if the Connection "timed out" after given amount of Retries
      Serial.print("Restart the WiFiModul due to lack of connection");
      WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
      conn_tries = 0;
      WiFi_Start();
    }
  }

  randomSeed(micros());
  
  Serial.println("");
  Serial.println("WiFi connected");
  printWifiStatus();

  // Connect to the MQTT Server
  client.setServer(mqttServer, 1883);
  delay(100);
}

// if the Connection gets lost, we can use this reconnect function
// Maybe needs some rework, but works for now
void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ArduinoClient-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUsername, mqttPassword)) 
    {
      Serial.println("connected");
    } else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// restructure needed, sometimes unnecessary reconnects were made
void loop() 
{
  // Start the Wifi
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi_Start();
  }

  // Get the sensor Data and Calculates it
  calcMoisture();
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();  

  // Publishes the Payload on RabbitMQ (via mqtt)
  client.publish(pubTopic, payload);
  Serial.println("disconnect");
  client.disconnect();
  delay(1000);

  // Wifi disable to save batterly power
  WiFi_Off();
  
  // Set the Timer from TIME_TO_SLEEP on the esp32
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");

  //Go into hibernate mode for power saving
  //Will start up after TIME_TO_SLEEP is elapsed or a manual Reboot is done
  esp_deep_sleep_start();
}

//Function that prints the reason by which ESP32 has been awaken from sleep
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case 1  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : Serial.println("Wakeup caused by timer"); break;
    case 4  : Serial.println("Wakeup caused by touchpad"); break;
    case 5  : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break;
  }
}

// Calculates the Moisture Level from the Sensor
void calcMoisture() 
{
  soilMoistureValue  = analogRead(ANALOG_PIN);
  Serial.print("Moisture from sensor: ");
  Serial.println(soilMoistureValue);

  // maps for with the min and max value
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);

  Serial.print("Moisture mappedr: ");
  Serial.println(soilmoisturepercent);

  // if the values for min/max are not acurate enough we limit them to 0% or 100%
  if(soilmoisturepercent > 100)
  {
    sprintf(str, "%u", 100);
  } 
  else if(soilmoisturepercent <0)
  {
    sprintf(str, "%u", 0);
  }
  else if(soilmoisturepercent >=0 && soilmoisturepercent <= 100)
  {
    sprintf(str, "%u", soilmoisturepercent);
  }
  
  Serial.print("Moisture str: ");
  Serial.println(str);

  // getting the Value from the optional BME Sensor
  dtostrf(bme.readTemperature(), 6, 2, cAirTemp);
  dtostrf(bme.readPressure() / 100.0F, 6, 2, cAirPressure);
  dtostrf(bme.readHumidity(), 6, 2, cAirMoist);

  // Build the Payload string
  // defenitly not the best approach but it works
  sPayload += str;
  sPayload += ",";
  sPayload += SensorNumber;
  sPayload += ",";
  sPayload += cAirTemp;
  sPayload += ",";
  sPayload += cAirPressure;
  sPayload += ",";
  sPayload += cAirMoist;  

  // sending the payload via mqtt 
  sPayload.toCharArray(payload, 200);
  Serial.print("Payload char: ");
  Serial.print(payload);  
  Serial.println("\n");
}

// Just for Information in the Consol and Debugging
// can be removed in the Future
void printWifiStatus() {

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
