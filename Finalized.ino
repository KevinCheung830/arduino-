#include  "WiFiEsp.h"
#include "ThingSpeak.h"
#include<SPI.h>
#include<Wire.h>
#include <SoftwareSerial.h>
#include<Adafruit_GFX.h>
#include<Adafruit_SSD1306.h>
#include "DHT.h"                //DHT sensor
#include <Adafruit_NeoPixel.h>  //set of LEDs
#define ESP_BAUDRATE 115200

//OLED 
#define scrw 128
#define scrh 64
#define OLEDreset -1
#define scradd 0x3C
//Adafruit_SSD1306 display(scrw,scrh,&Wire,OLEDreset);
Adafruit_SSD1306 display(128,64);

//redtx bugback
//HardWare connection fo BT05 RX=10 TX=11
SoftwareSerial BTSerial(11,10); 

//DHT11
#define DHTPIN 8
#define DHTTYPE DHT11
//Humidity DHT11 intial
DHT dht(DHTPIN, DHTTYPE);

//set of LEDs setting
#define LED_PIN 12
#define LED2_PIN 13

#define LED_Count 22
#define LED2_Count 30
Adafruit_NeoPixel  strip = Adafruit_NeoPixel(LED_Count,LED_PIN,NEO_GRB + NEO_KHZ800); //set of LEDs setting
Adafruit_NeoPixel  strip2 = Adafruit_NeoPixel(LED2_Count,LED2_PIN,NEO_GRB + NEO_KHZ800); //set of LEDs setting

//Waterlv
#define Waterlv_sensorPower 5
#define Waterlv_sensorPin A13

//WIFI and THINGSPEAK SETTING
char ssid[]="EE3070_P1615_1";
char pass[]="EE3070P1615";
int status = WL_IDLE_STATUS;
unsigned long ChannelID = 2049527;
const char*ReadAPIKey = "0FCYCVNKHCX7GBK2";
const char*WriteAPIKey = "L1MXYXEJSJ8H31NZ";
WiFiEspClient client;

//Water level setting
int lowerThreshold = 420;
int upperThreshold = 520;
// Value for storing water level
int val = 0;
// Declare pins to which LEDs are connected water level
int redLED = 2;
int greenLED =3;
int blueLED = 4;

//Relay count
int count=0;
int counter =0;
bool signal= false;
const int RELAY_PIN = A5;

void setup() {

  pinMode(A15, INPUT); // photo sensor
  pinMode(RELAY_PIN, OUTPUT);  
  //bluetooth
   BTSerial.begin(9600);

//WIFI configuration
  // initialize serial for debugging
  Serial.begin(115200); 
  Serial1.begin(ESP_BAUDRATE);
  // initialize ESP module
  WiFi.init(&Serial1);

  //DHT11 initialization
  dht.begin();  

  //OLED initialization
  if(!display.begin(SSD1306_SWITCHCAPVCC,0X3C)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  //start
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(42, 26);
    display.print("System_Start");
    display.display();

  //LEDs initialization
  strip.begin();

  // WATER LEVEL SETTING
  pinMode(Waterlv_sensorPower, OUTPUT);
  digitalWrite(Waterlv_sensorPower, LOW);
  // Initially turn off all LEDs
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, LOW);
  digitalWrite(blueLED, LOW);


  // Connect or reconnect to WiFi
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    if(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected");
    ThingSpeak.begin(client); 
  }
  ThingSpeak.begin(client); 

}

void loop(){
   
  
   //*********************** Brightness()****************************
    int sensorValue = analogRead(A15);
    Serial.print("Brightness: ");
    Serial.println(sensorValue);

    if(sensorValue>=0 && sensorValue<=199){
      Sun(128, 0, 128); // set all LEDs to purple
      strip.show(); // update the LED strip with the new colors
      Sun2(0, 0, 128); // set all LEDs to purple
      strip2.show(); // update the LED strip with the new colors
      
    }else if(sensorValue>=200 && sensorValue<=600){
      Sun(0, 0, 0); // set all LEDs to purple
      strip.show(); // update the LED strip with the new colors
       Sun2(0, 0, 0); // set all LEDs to purple
      strip2.show(); // update the LED strip with the new colors
    }

     
    
    //*********************** Brightness()****************************

   //*********************** HDT11_Humidity_tempretrue()****************************
    float humidity = dht.readHumidity();   //humidity
    float temp     = dht.readTemperature();  //temperature C

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" *C ");

    //*********************** HDT11_Humidity_tempretrue()**************************** 

    
    //*********************** WATER_LEVEL************************************* 
    int level = readSensor();

    if (level == 0) {
      Serial.println("Water Level: Empty");
      digitalWrite(redLED, LOW);
      digitalWrite(greenLED, LOW);
      digitalWrite(blueLED, LOW);                
    }
    else if (level > 0 && level <= lowerThreshold) {
      Serial.println("Water Level: Low");
      digitalWrite(redLED, HIGH);
      digitalWrite(greenLED, LOW);
      digitalWrite(blueLED, LOW);
    }
    else if (level > lowerThreshold && level <= upperThreshold) {
      Serial.println("Water Level: Medium");
      digitalWrite(redLED, LOW);
      digitalWrite(greenLED, HIGH);
      digitalWrite(blueLED, LOW);
    }
    else if (level > upperThreshold) {
      Serial.println("Water Level: High");
      digitalWrite(redLED, LOW);
      digitalWrite(greenLED, LOW);
      digitalWrite(blueLED, HIGH);
    }

    //*********************** WATER_LEVEL*************************************//

    
    //*********************** UPLOAD_DATA_TO_THINGSPEAK****************************
    if(counter== 5 || counter == 0 ){
    ThingSpeak.setField(2,sensorValue);   //brightness
    ThingSpeak.setField(3,humidity); //waterlevel
    ThingSpeak.setField(1,temp);     //temperature
    
    if (level == 0) {
      ThingSpeak.setField(6,0);          
      }
      else if (level > 0 && level <= lowerThreshold) {
      ThingSpeak.setField(6,1);  
      }
      else if (level > lowerThreshold && level <= upperThreshold) {
      ThingSpeak.setField(6,2); 
      } 
      else if (level > upperThreshold) {
      ThingSpeak.setField(6,3); 
      }
      int x = ThingSpeak.writeFields(ChannelID,WriteAPIKey);
      if(x==200){
       Serial.println("upload sucess!");
      }
      else{
      Serial.println("upload failed!");
      }   
    }
    //*********************** UPLOAD_DATA_TO_THINGSPEAK****************************
    


    //***********************OLED_DISPLAY************************************* 
    if(counter==2){
      display.clearDisplay();
      display.setCursor(2, 2);
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.println("WaterLv: ");
      if(level==0){display.println("No water");}
      if(level > 0 && level <= lowerThreshold){display.println("water lv low");}
      if(level > lowerThreshold && level <= upperThreshold){display.println("too much water");}

      display.setCursor(0, 32);
      display.print("Brightness: ");
      display.println(sensorValue);
      display.display();
    }
  
    //*********************** BT05*************************************
    
    //bluetooth
   Serial.println("Connecting BlueTooth ");
   if(BTSerial.available()>0){// Check if data is available from BT05
      char data = BTSerial.read(); 
      //int BT_datain=BTSerial.parseInt();
      BTSerial.println("*********************");
      BTSerial.println("InstantReport");
      BTSerial.println("Go and Check out on ThingSpeak Cloud");
      BTSerial.println("");
    
      Serial.print("BT05_Connected");
      Serial.print("Received data: ");
      Serial.println(data); // Print received data to serial for debugging
      BTSerial.println("BT05_Connected");
      BTSerial.println("");
      BTSerial.print("Brightness:");
      BTSerial.println(sensorValue);
      BTSerial.println("");
      BTSerial.print("temp:");
      BTSerial.println(temp);
      BTSerial.println("");
      BTSerial.print("humi:");
      BTSerial.println(humidity);
      BTSerial.println("");
      BTSerial.print("WaterLv: ");
      if(level==0){BTSerial.println("No water");}
      if(level > 0 && level <= lowerThreshold){BTSerial.println("Medium");}
      if(level > lowerThreshold && level <= upperThreshold){BTSerial.println("Water Overflow");}
      BTSerial.println("");
      if(sensorValue>=0 && sensorValue<=199){
        BTSerial.println("Light is ON");
        BTSerial.println("It is kinda dark here");
      }
      else if(sensorValue>=200 && sensorValue<=1000){
        BTSerial.println("Light is OFF");
        BTSerial.println("It is kinda bright here");
        }
      if(data=='1'){
        signal=true; //power the pump
        BTSerial.println("The bump is ON");
        BTSerial.println("FullAuto...........Mode.ON");
      } 
      else if(data=='0'){
        Serial.println("the bump is OFF");    
        signal=false; //off the pump
        BTSerial.println("FullAuto...........Mode.OFF");
        Sun(0, 0, 0); // turn off all LEDs
        strip.show(); // update the LED strip with the new colors
        digitalWrite(RELAY_PIN, LOW);
        }
      BTSerial.println("End OF instant Report");
      BTSerial.println("*********************");
   }
   
    //*********************** BT05*************************************
    

    

    //***********************OLED_DISPLAY************************************* 
    
   if(counter==4){
    display.clearDisplay();
    display.setCursor(2, 2);
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.print("Humidity: ");
    display.println(humidity);
  
 
    display.setCursor(0, 32);
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.print("tempretrue: ");
    display.println(temp);
    display.display();
   
   }
    //***********************OLED_DISPLAY*************************************//

    //***********************PumpControl*************************************//
   if(counter==2 && signal==true){
      Serial.println("BumpStatus:ON"); 
      digitalWrite(RELAY_PIN, HIGH); // turn on pump 10 seconds
      delay(10000);  
      digitalWrite(RELAY_PIN, LOW);   
      Serial.print("...........ONNONONONOON");
   }

    //***********************Counter*************************************//
    counter++;
    if(counter ==15){
      counter=0;
    }
    delay(1000);
}

int readSensor() {
  digitalWrite(Waterlv_sensorPower, HIGH);
  delay(10);
  val = analogRead( Waterlv_sensorPin);
  digitalWrite(Waterlv_sensorPower, LOW);
  return val;
}

void printWifiData(){
    // print your WiFi shield's IP address
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
}


void Sun(uint8_t r, uint8_t g, uint8_t b) {
  for(int i = 0; i < LED_Count; i++) {
    strip.setPixelColor(i, r, g, b); // set the color of the LED
  }
}

void Sun2(uint8_t r, uint8_t g, uint8_t b) {
  for(int i = 0; i < LED2_Count; i++) {
    strip2.setPixelColor(i, r, g, b); // set the color of the LED
  }
}