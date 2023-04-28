/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C_Hangul.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>


// Update these with values suitable for your network.

const char* ssid = "DK40-WiFi";
const char* password = "d3lk0m40!";
const char* mqtt_server = "192.168.241.251";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg_t[MSG_BUFFER_SIZE];
char msg_p[MSG_BUFFER_SIZE];
char msg_a[MSG_BUFFER_SIZE];
char msg_pot[MSG_BUFFER_SIZE];
char msg_v[MSG_BUFFER_SIZE];
char msg_b[MSG_BUFFER_SIZE];

int but;
int test;

////////////////////////////////////////////////////////////////// Идентификация датчика и LCD
Adafruit_BMP280 bmp; // I2C
LiquidCrystal_I2C_Hangul lcd(0x27,16,2); //LCD 클래스 초기화
//float temperature = bmp.readTemperature();
//////////////////////////////////////////////////////////////////

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(D7, OUTPUT);
  digitalWrite(D7, LOW);
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

///////////////////////////////////////////////////////////////////////////////////////////    Проверка соединения и адреса датчика
 pinMode(A0, INPUT);   // к входу A0 подключаем потенциометр
 lcd.init();
 lcd.backlight();

 lcd.setCursor(0, 0);
 lcd.print("BMP280");
 lcd.setCursor(0, 1);
 lcd.print("");
 delay(5000);

  if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
  //if (!bmp.begin()) {
    lcd.print("Check wiring!");
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
///////////////////////////////////////////////////////////////////////////////////////////////
}

void loop() {
 
//////////////////////////////////////////////////////////////////////////////////////////////

 int val = analogRead(A0); // считываем данные с порта A0
 float voltage = ((float)(val * 3) / 1024); // перевод аналогового значения в вольты

  if (bmp.takeForcedMeasurement()) {
    /////////////////////////  ВЫВОД НА LCD /////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////////////////////////////// Вывод на LCD данные с датчика 

    lcd.setCursor(0, 0);
    lcd.print("T=");
    lcd.print(bmp.readTemperature(), 1);
    lcd.print("*C ");

    lcd.print(F("A="));
    lcd.print(bmp.readAltitude(1013.25) + 44, 1); /* Adjusted to local forecast! */
    lcd.print("m");

    lcd.setCursor(0, 1);
    lcd.print("P = ");
    lcd.print(bmp.readPressure() / 1000, 2);
    lcd.print(" kPa");
    delay(1000);
    lcd.clear();

    /////////////////////////////////////////////////////////////////////////////////////////////  Вывод на LCD данные с потенциометра
 
    lcd.setCursor(0, 0);
    lcd.print("ACP = ");            
    lcd.print(val);
    lcd.setCursor(0, 1);
    lcd.print("Voltage = ");
    lcd.print(voltage);
    lcd.print("V");
    delay(1000);
    lcd.clear();
   
    ////////////////////////////////////////////////////////////////////////////////////////////// Вывод в порт
    Serial.print("Temperature = ");
    Serial.print(bmp.readTemperature(), 2);
    Serial.println(" *C");

    Serial.print(("Altitude = "));
    Serial.print(bmp.readAltitude(1013.25) + 44, 2); /* Adjusted to local forecast! */
    Serial.println(" m");

    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure() / 1000, 2);
    Serial.println(" kPa");

    Serial.print(val);
    Serial.print(" = ");
    Serial.print(voltage);
    Serial.println("V");

    Serial.println();
    delay(1000);
    lcd.clear();
  } else {
    lcd.print("Failed!");
  }

  if (digitalRead(D7) == LOW){

    Serial.println("The button isn't pressed");
    but = 0;

  } else if (digitalRead(D7) == HIGH) {

  Serial.println("The button is pressed");
    but = 1;
    
  }
///////////////////////////////////////////////////////////////////////////////////////////// Вывод данных на страницу


  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    snprintf (msg_t, MSG_BUFFER_SIZE, "%f", bmp.readTemperature());
    snprintf (msg_a, MSG_BUFFER_SIZE, "%f", bmp.readAltitude(1013.25) + 44);
    snprintf (msg_p, MSG_BUFFER_SIZE, "%f", bmp.readPressure()/1000);
    snprintf (msg_pot, MSG_BUFFER_SIZE, "%d", val);
    snprintf (msg_v, MSG_BUFFER_SIZE, "%f", voltage);
    snprintf (msg_b, MSG_BUFFER_SIZE, "%d", but);
   
    client.publish("temperature", msg_t);
    client.publish("pressure", msg_p);
    client.publish("altitude", msg_a);
    client.publish("potenciometr", msg_pot);
    client.publish("voltage", msg_v);
    client.publish("button", msg_b);
  }
}