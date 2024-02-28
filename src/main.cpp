#include <Arduino.h>
#include <WiFiS3.h>
#include <secrets.h>
#include <Wire.h>
#include <Adafruit_PM25AQI.h>
#include <SPI.h>

#define LED_GROEN 4
#define LED_GEEL 2
#define LED_ROOD 3

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
int status = WL_IDLE_STATUS;

// Millis
unsigned long millisDataSturen = 0;
unsigned long millisTime = 0;


// Function to initialize Led's
void initLed()
{
  pinMode(LED_GROEN, OUTPUT);
  pinMode(LED_GEEL, OUTPUT);
  pinMode(LED_ROOD, OUTPUT);

  digitalWrite(LED_GROEN, LOW); // Initialize LED_GROEN as off
  digitalWrite(LED_GEEL, LOW);  // Initialize LED_GEEL as off
  digitalWrite(LED_ROOD, LOW);  // Initialize LED_ROOD as off
}

void printWifiStatus()
{
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("Signal Strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

// Function that collects data from the sensor and sends it to sheets
void sendDataSheets()
{
  digitalWrite(LED_ROOD, HIGH);

  WiFiSSLClient client;

  // Get the values from the sensor
  PM25_AQI_Data data;
  
  if (! aqi.read(&data)) {
    Serial.println("Could not read from AQI");
    return;
  }
  Serial.println("AQI reading success");

  String PM_10 = String(data.pm10_standard);
  String PM_25 = String(data.pm25_standard);
  String PM_100 = String(data.pm100_standard);
  String particles_03um = String(data.particles_03um);
  String particles_05um = String(data.particles_05um);
  String particles_10um = String(data.particles_10um);
  String particles_25um = String(data.particles_25um);
  String particles_50um = String(data.particles_50um);
  String particles_100um = String(data.particles_100um);

  // Construct the final URL for the Google Sheets script
  String urlFinal = GOOGLE_APPS_SCRIPT_URL + GOOGLE_SCRIPT_ID + "/exec?pm10=" + PM_10 + "&pm25=" + PM_25 + "&pm100=" + PM_100 + "&um03=" + particles_03um + "&um05=" + particles_05um + "&um10=" + particles_10um + "&um25=" + particles_25um + "&um50=" + particles_50um + "&um100=" + particles_100um;
  if (client.connect("script.google.com", 443))
  {
    Serial.println("connected to server");

    // Send HTTP GET request
    client.println("GET " + urlFinal + " HTTP/1.1");
    client.println("Host: script.google.com");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
    client.stop();
  }
  else
  {
    client.stop();
    Serial.println("No client connect");
  }
  digitalWrite(LED_ROOD, LOW);
}

void setup()
{
  initLed();
  Serial.begin(9600);

  digitalWrite(LED_GROEN, HIGH);

  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(1000);
    digitalWrite(LED_GEEL, HIGH);
  }

  printWifiStatus();

  Wire1.begin(); // Join I2C bus
  if (! aqi.begin_I2C()) {      // connect to the sensor over I2C
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }

  Serial.println("\nStarting connection to server...");
}

void loop()
{
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(2000);
    digitalWrite(LED_GEEL, HIGH);
  }

  Serial.println("still running 1");
  
  if (millis() >= millisDataSturen + (60000 * 15))
  {
    Serial.println("still running 2");
    millisDataSturen = millis();
    sendDataSheets();
  }
  Serial.println("still running 3");
}
