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

#define uS_TO_S_FACTOR 1000000ULL  //Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  36000        //Time ESP32 will go to sleep (in seconds)
int WIFI_RETRIES = 20;
const int AirValue = 4095;   //you need to replace this value with Value_1
const int WaterValue = 2736;  //you need to replace this value with Value_2
const char* SensorNumber = "3"; // Change for every Sensor


#define LED_PIN   2
#define ANALOG_PIN 36

int soilMoistureValue = 0;
int soilmoisturepercent=0;

char str[120];
char payload[200];
char cAirTemp[10];
char cAirMoist[10];
char cAirPressure[10];
String sPayload;

int conn_tries = 0;

const char* ssid = networkSSID;
const char* password = networkPASSWORD;
const char* mqttServer = mqttSERVER;
const char* mqttUsername = mqttUSERNAME;
const char* mqttPassword = mqttPASSWORD;

char pubTopic[] = "home/sensordata1";       //payload[0] will have ledState value

WiFiClient wifiClient;
PubSubClient client(wifiClient);

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

void WiFi_Off() {
  client.disconnect();
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  Serial.print("Wifi Disconnected");
}

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

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    conn_tries++;
    Serial.println(conn_tries);
    Serial.println(WIFI_RETRIES);
    if (conn_tries > WIFI_RETRIES)
    {
      Serial.print("Restart the WiFiModul due to lack of connection");https://www.youtube.com/feed/history
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
  client.setServer(mqttServer, 1883);
  delay(100);
}

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

void loop() 
{
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi_Start();
  }
  calcMoisture();
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();  
  client.publish(pubTopic, payload);
  Serial.println("disconnect");
  client.disconnect();
  delay(1000);
  WiFi_Off();
  
  //Set timer to 60 seconds
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");

  //Go to sleep now
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

void calcMoisture() 
{
  soilMoistureValue  = analogRead(ANALOG_PIN);
  Serial.print("Moisture from sensor: ");
  Serial.println(soilMoistureValue);
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);

  Serial.print("Moisture mappedr: ");
  Serial.println(soilmoisturepercent);
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
  
  dtostrf(bme.readTemperature(), 6, 2, cAirTemp);
  dtostrf(bme.readPressure() / 100.0F, 6, 2, cAirPressure);
  dtostrf(bme.readHumidity(), 6, 2, cAirMoist);

  sPayload += str;
  sPayload += ",";
  sPayload += SensorNumber;
  sPayload += ",";
  sPayload += cAirTemp;
  sPayload += ",";
  sPayload += cAirPressure;
  sPayload += ",";
  sPayload += cAirMoist;  
  
  sPayload.toCharArray(payload, 200);
  Serial.print("Payload char: ");
  Serial.print(payload);  
  Serial.println("\n");
}

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
