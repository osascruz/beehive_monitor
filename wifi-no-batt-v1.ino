//Beehive monitor for wifi
//Based on v4 of the beehive monitor by Marc Curtis @exmonkey https://github.com/exmonkey206/beehive_monitor

//Importing various libraries


#include <CountingStream.h>
#include <Adafruit_MQTT_Client.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_SleepyDog.h>
#include <b64.h>
#include <HttpClient.h>
#include <DHT.h>
#include <WiFi.h>
#include <SPI.h>

//Setup a DHT22 instance
#define DHTTYPE DHT22
#define DHTPIN 5

DHT dht(DHTPIN, DHTTYPE);

float temp = 0;
float humidity = 0;

//Adafruit IO Setup

#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "petespeller"
#define AIO_KEY "02964e15ff8269d41755cff6ff944c0c3753b536"

// Store the MQTT server, client ID, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[] PROGMEM = AIO_SERVER;

// Set a unique MQTT client ID using the AIO key + the date and time the sketch
// was compiled (so this should be unique across multiple devices for a user,
// alternatively you can manually set this to a GUID or other random value).
const char MQTT_CLIENTID[] PROGMEM  = __TIME__ AIO_USERNAME;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

//connect to wifi
char ssid[] = "VM264773-2G";     //  your network SSID (name) 
char pass[] = "gtypgcjt";    // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

//Setup feed called 'temperature' for publishing changes
const char TEMPERATURE_FEED[] PROGMEM = AIO_USERNAME "/feeds/temperature";
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, TEMPERATURE_FEED);

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
  
  //Make sure to reset watchdog every loop iteration
  Watchdog.reset();
  
  //ping adafruit io a few times to check we remain connected
  if(! mqtt.ping(3)){
    //reconnect to adafruit io
    if(! mqtt.connected())
      connect();
  }
  
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");  
  
}

// connect to adafruit io via MQTT
void connect() {

  Serial.print(F("Connecting to Adafruit IO... "));

  int8_t ret;

  while ((ret = mqtt.connect()) != 0) {

    switch (ret) {
      case 1: Serial.println(F("Wrong protocol")); break;
      case 2: Serial.println(F("ID rejected")); break;
      case 3: Serial.println(F("Server unavail")); break;
      case 4: Serial.println(F("Bad user/pass")); break;
      case 5: Serial.println(F("Not authed")); break;
      case 6: Serial.println(F("Failed to subscribe")); break;
      default:
        Serial.println(F("Connection failed"));
        //CC3000connect(WLAN_SSID, WLAN_PASS, WLAN_SECURITY);
        break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println(F("Retrying connection..."));
    delay(5000);

  }

  Serial.println(F("Adafruit IO Connected!"));

}
