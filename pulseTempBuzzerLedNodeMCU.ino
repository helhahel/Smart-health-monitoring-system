#define USE_ARDUINO_INTERRUPTS true // Set-up low-level interrupts for most acurate BPM math.
#include <PulseSensorPlayground.h> // Includes the PulseSensorPlayground Library.
#include <Wire.h>
#include <SeeedGrayOLED.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

#include <ThingSpeak.h>
#include <ESP8266WiFi.h>

String apiKey = "RTH0H0QB34EIYWA7";     //  Enter your Write API key from ThingSpeak
 
const char *ssid =  "Home@Wifi";     // replace with your wifi ssid and wpa2 key
const char *pass =  "home@2022";
const char* server = "api.thingspeak.com";
#define DHTPIN 5
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
// Variables
const int PulseWire = D0; // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
const int led = 13; // The on-board Arduino LED, close to PIN 13.
const int ledAlert = 12;
const int buzzer = 11;
int Threshold = 550; // Determine which Signal to "count as a beat" and which to ignore.
 
PulseSensorPlayground pulseSensor; // Creates an instance of the PulseSensorPlayground object called "pulseSensor"

WiFiClient client;
unsigned long myChannelNumber = 1730307 ; //Channel Number


void setup() {

   Wire.begin();
   Serial.begin(9600);
   SeeedGrayOled.init(SH1107G);                 // initialize SEEED OLED display
   //SeeedGrayOled.setInverseDisplay();    // Set display to inverse mode
   SeeedGrayOled.clearDisplay();         // Clear Display.
   SeeedGrayOled.setVerticalMode();      // Set to vertical mode for displaying text
       

  Serial.println("DHT11 test!");
  dht.begin();
  
  pulseSensor.analogInput(PulseWire);
  pulseSensor.blinkOnPulse(led); //auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold);
  
  if (pulseSensor.begin()) {
       Serial.println("We created a pulseSensor Object !"); //This prints one time at Arduino power-up, or on Arduino reset.
          }
          
  SeeedGrayOled.setTextXY(1, 0); //set Cursor to ith line, 0th column
  SeeedGrayOled.putString("HEALTH MONITORIN");
  SeeedGrayOled.setTextXY(4, 3);
  SeeedGrayOled.putString("Heart Beat!");
  SeeedGrayOled.setTextXY(6, 3);
  SeeedGrayOled.putString("Body Temperatu!");
  SeeedGrayOled.setTextXY(8, 3);
  SeeedGrayOled.putString("Alcohol Level!");
  delay(3000);

       Serial.println("Connecting to ");
       Serial.println(ssid);
 
 
       WiFi.begin(ssid, pass);
 
      while (WiFi.status() != WL_CONNECTED) 
     {
            delay(500);
            Serial.print(".");
     }
      Serial.println("");
      Serial.println("WiFi connected");
}

void loop() {

  pinMode(buzzer, OUTPUT);
  pinMode(ledAlert, OUTPUT);

  //SeeedGrayOled.clearDisplay();
  
  // Double-check the "pulseSensor" object was created and "began" seeing a signal.
  int BPM = pulseSensor.getBeatsPerMinute(); // Calls function on our pulseSensor object that returns BPM as an "int".
      // "BPM" hold this BPM value now.
  if (pulseSensor.sawStartOfBeat()) {
     SeeedGrayOled.clearDisplay();
     heartBeat();
     //temperature();
  }
     float h = dht.readHumidity();
     float t = dht.readTemperature();
     float f = dht.readTemperature(true);

                           if (client.connect(server,80))   //   "184.106.153.149" or api.thingspeak.com
                      {  
                            
                             String postStr = apiKey;
                             postStr +="&field1=";
                             postStr += String(BPM);
                             postStr +="&field2=";
                             postStr += String(t);
                             //postStr +="&field3=";
                             //postStr += String(ratio);
                             postStr += "\r\n\r\n";
 
                             client.print("POST /update HTTP/1.1\n");
                             client.print("Host: api.thingspeak.com\n");
                             client.print("Connection: close\n");
                             client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
                             client.print("Content-Type: application/x-www-form-urlencoded\n");
                             client.print("Content-Length: ");
                             client.print(postStr.length());
                             client.print("\n\n");
                             client.print(postStr);
 
                             //Serial.print("Temperature: ");
                             //Serial.print(t);
                             //Serial.print(" degrees Celcius, Humidity: ");
                             //Serial.print(h);
                             Serial.println("%. Send to Thingspeak.");
                        }
          client.stop();
 
          Serial.println("Waiting...");
  
  // thingspeak needs minimum 15 sec delay between updates
  delay(1000);
  
}
  
void heartBeat() {
       int BPM = pulseSensor.getBeatsPerMinute(); // Calls function on our pulseSensor object that returns BPM as an "int".
      
      Serial.print("♥ BPM: "); // Print phrase "BPM: "
      Serial.println(BPM); // Print the value inside of BPM.
      SeeedGrayOled.setTextXY(3, 5); //set Cursor to ith line, 0th column
      SeeedGrayOled.putString("HeartBeat!"); // If test is "true", print a message "a heartbeat happened".
      SeeedGrayOled.setTextXY(5, 3); //set Cursor to ith line, 0th column
      SeeedGrayOled.putString("♥ BPM:"); // Print phrase "BPM: "
      SeeedGrayOled.putNumber(BPM); // Print the value inside of BPM.
      //}
      delay(1000); // considered best practice in a simple sketch.

      if ((BPM >= 100) && (BPM <= 250)) {
        
         Serial.println("Heart is Normal");
         SeeedGrayOled.setTextXY(9, 3); //set Cursor to ith line, 0th column
         SeeedGrayOled.putString("Heart Normal!");
         delay(2000);
         
        }
        
      else {
        
         Serial.println("Heart is Abnormal");
         SeeedGrayOled.setTextXY(7, 3); //set Cursor to ith line, 0th column
         SeeedGrayOled.putString("Abnormal Heart!");
         digitalWrite(ledAlert, HIGH);
         tone(buzzer,10000);
         delay(3000);
         noTone(buzzer);
         temperature();       
        }
      }
      
void temperature(){
   
     float h = dht.readHumidity();
     float t = dht.readTemperature();
     float f = dht.readTemperature(true);
     
     //check if any reads failes
     if (isnan(h) || isnan(t)|| isnan(f)) {
         Serial.println("Failed to read from DHT sensor!");
         return;
        }
      else{
              
              Serial.print("Humidity: ");
              Serial.print(h);
              Serial.print(" %\t");
              Serial.print("Temperature: ");
              Serial.print(t);
              Serial.println(" *C ");
              
        //SeeedGrayOled.clearDisplay();
        SeeedGrayOled.setTextXY(9, 1); //set Cursor to ith line, 0th column
        SeeedGrayOled.putString(" Temperature:");
        SeeedGrayOled.setTextXY(11, 3);
        SeeedGrayOled.putNumber(t);
        SeeedGrayOled.putString(" °C ");
      }
      
      if ((t <= 20)||(t >= 29)) {
        Serial.println("Abnormal Temperature!");
        Serial.print("Temperature: ");
        Serial.print(t);
        Serial.println(" °C ");
        
        //SeeedGrayOled.clearDisplay();
        //SeeedGrayOled.setTextXY(7, 1); //set Cursor to ith line, 0th column
        //SeeedGrayOled.putNumber(t);
        //SeeedGrayOled.putString(" C ");
        SeeedGrayOled.setTextXY(13, 1);
        SeeedGrayOled.putString("Abnormal Temp ");
        
        digitalWrite(ledAlert, HIGH);
        tone(buzzer,10000);
        delay (3000);
        noTone(buzzer);
        digitalWrite(ledAlert, LOW);
        delay (3000);
        alcoholDetector();
      }
        
     if(20 < t < 27){
 
     //Start display temperature
     Serial.println("Perfect temperature!");
     Serial.print("Temperature: ");
     Serial.print(t);
     Serial.println(" °C ");
     
     //SeeedGrayOled.clearDisplay();
     //SeeedGrayOled.setTextXY(7, 1); //set Cursor to ith line, 0th column
     //SeeedGrayOled.putNumber(t);
     //SeeedGrayOled.putString(" C ");
     SeeedGrayOled.setTextXY(13, 1);
     SeeedGrayOled.putString("Normal Temp ");

     //digitalWrite(buzzer, LOW);
     noTone(buzzer);
     digitalWrite(ledAlert, LOW);
    
   //End display temperature
    delay(1000);
    }

  }
  
  void alcoholDetector(){
    
  }
 
