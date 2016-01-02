//Beehive monitor for wifi
//Based on v4 of the beehive monitor by Marc Curtis @exmonkey https://github.com/exmonkey206/beehive_monitor

//Importing various libraries

#include <Sleep_n0m1.h>
#include <CountingStream.h>
#include <Xively.h>
#include <XivelyClient.h>
#include <XivelyDatastream.h>
#include <XivelyFeed.h>
#include <b64.h>
#include <HttpClient.h>
#include <DHT.h>
#include <WiFi.h>
#include <SPI.h>

//Setup a DHT22 instance
#define DHTTYPE DHT22
#define DHTPIN 5

DHT dht(DHTPIN, DHTTYPE);

float temperature = 0;
float humidity = 0;
int wait = 225;
boolean notConnected = true;
unsigned long time;
int mins;
boolean working;

//sleep stuff
Sleep sleep;
unsigned long sleepTime; //how long you wna the arduino to sleep

char myTempStream[] = "temperature"; //Set stream name (need to match Xively name)
char myHumidityStream[] = "humidity"; //Set 2nd stream name

XivelyDatastream datastreams[] = {
  XivelyDatastream(myTempStream, strlen(myTempStream), DATASTREAM_FLOAT),
  XivelyDatastream(myHumidityStream, strlen(myHumidityStream), DATASTREAM_FLOAT),
};

#define FEED_ID 1234 //Set Xively Feed ID
char xivelyKey[] = "1234"; //Set Xively API key

XivelyFeed feed(FEED_ID, datastreams, 2); //Creating the feed, definining the two datastreams
WiFiClient client;
XivelyClient xivelyclient(client);

//connect to wifi
char ssid[] = "name";     //  your network SSID (name) 
char pass[] = "1234";    // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

void setup(void) {
  // dht sensor
  dht.begin();
  
  // wifi
  // initialize serial:
  Serial.begin(9600);

  // attempt to connect using WPA2 encryption:
  Serial.println("Attempting to connect to WPA network...");
  status = WiFi.begin(ssid, pass);

  // if you're not connected, stop here:
  if ( status != WL_CONNECTED) { 
    Serial.println("Couldn't get a wifi connection");
    while(true);
  } 
  // if you are connected, print out info about the connection:
  else {
    Serial.println("Connected to network");
  }
}

void loop() {
  if(working==false){
    working = true;
    Serial.print("allow the sensor to warm up");
    delay(5000);
  }
}

// Get the data from the DHT222 sensor
void get_data(){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)

  boolean gotReading = 0;

  while (gotReading==0) {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temperature) ){
      Serial.println("Failed to read from DHT sensor!");
      delay(1000);
    }
    else{
      gotReading = 1;
    }
  }

  // Compute heat index
  // Must send in temp in Fahrenheit!
  //float hi = dht.computeHeatIndex(f, h);

  Serial.print("Humidity: "); 
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: "); 
  Serial.print(temperature);
  Serial.print(" *C ");
  processData();
}

void processData(){
  Serial.println("process data"); 

  //add the results to the xively stream
  datastreams[0].setFloat(temperature); 
  datastreams[1].setFloat(humidity);
  //datastreams[2].setFloat(outputValue);

  int ret = xivelyclient.put(feed, xivelyKey);  // Send to Xively
  Serial.println("Ret: "); 
  Serial.println(ret); 

  if(ret == 200){
    Serial.println(ret);      
    delay(1000);
  }
  else{                                    
    delay(1000);
    Serial.println(ret);
  } 
}


