#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFiNINA.h>
#include <Wire.h>


#define uvPin 3
#define TempPin 2
#define DHTTYPE DHT11

Adafruit_BMP280 bmp; // I2C Communication
DHT_Unified dht(TempPin, DHTTYPE);
// Network information
char* ssid = "EagleNet";//"MCTTexas";////"TP-Link_AF06";
const char* password = "";//"1A2B345CD";


// ThingSpeak settings
char server[] = "api.thingspeak.com";
String writeAPIKey = "518JQL1XY96JNR9V";

// Constants
unsigned long postingInterval = 15 * 1000;

// Global variables
unsigned long lastConnectionTime = 0;
int measurementNumber = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();
    pinMode(TempPin,INPUT);
    pinMode(uvPin, INPUT);
    connectWiFi();
    if (!bmp.begin()) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }

}

void loop() {
  unsigned long timems = millis();

  if((timems-lastConnectionTime)>postingInterval){
    sensors_event_t event;
    double currentTemp = readTemp(event);
    double currentHumidity = readHum(event);
    long pressure = bmp.readPressure()/3386.39; //[in-Hg]
    double uvIndex = analogRead(uvPin)/0.1;
    Serial.println(currentTemp);
    Serial.println(currentHumidity);
    Serial.println(pressure);
    Serial.println(uvIndex);
    httpRequest(currentTemp,currentHumidity,pressure,uvIndex);
  }
    

}


void connectWiFi(){

    while (WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password);
        delay(3000);
    }

    // Display a notification that the connection is successful. 
    Serial.println("Connected");
    
}

void httpRequest(double field1Data, double field2Data, long field3Data, double field4Data) {

    WiFiClient client;
    
    if (!client.connect(server, 80)){
      
        Serial.println("Connection failed");
        lastConnectionTime = millis();
        client.stop();
        return;     
    }
    
    else{
        
        // Create data string to send to ThingSpeak.
        String data = "field1=" + String(field1Data) + "&field2=" + String(field2Data) + "&field3=" + String(field3Data) + "&field4=" + String(field4Data); //shows how to include additional field data in http post
        
        // POST data to ThingSpeak.
        if (client.connect(server, 80)) {
          
            client.println("POST /update HTTP/1.1");
            client.println("Host: api.thingspeak.com");
            client.println("Connection: close");
            client.println("User-Agent: ESP32WiFi/1.1");
            client.println("X-THINGSPEAKAPIKEY: "+writeAPIKey);
            client.println("Content-Type: application/x-www-form-urlencoded");
            client.print("Content-Length: ");
            client.print(data.length());
            client.print("\n\n");
            client.print(data);
            
            Serial.println("RSSI = " + String(field1Data));
            lastConnectionTime = millis();   
            delay(250);
        }
    }
    client.stop();
}

double readTemp(sensors_event_t event)
{
  dht.temperature().getEvent(&event);
  double temp = event.temperature;

  return(temp);
}

double readHum(sensors_event_t event)
{
  dht.humidity().getEvent(&event);
  double humidity = event.relative_humidity;

  return(humidity);
}
