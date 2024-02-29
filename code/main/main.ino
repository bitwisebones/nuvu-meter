
#include <ArduinoMqttClient.h>
#include <WiFi.h>
#include <math.h>

char ssid[] = "";
char pass[] = "";
int status = WL_IDLE_STATUS;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "";
int port = 1883;
const char topic[] = "";

const int vu_pin = 5;
const int r_pin = 6;
const int g_pin = 7;
const int b_pin = 8;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(vu_pin, OUTPUT);
  pinMode(r_pin, OUTPUT);
  pinMode(g_pin, OUTPUT);
  pinMode(b_pin, OUTPUT);
  
  Serial.begin(9600);

  Serial.println("Attempting to connect to WiFi network...");

  while(WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(5000);
  }

  Serial.println("Connected.");
  Serial.println("Attempting to connect to MQTT broker...");

  if(!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed. Error code = ");
    Serial.println(mqttClient.connectError());
    while(1);
  }

  Serial.println("Connected.");

  mqttClient.onMessage(onMqttMessage);

  Serial.print("Subscribing to topic: ");
  Serial.println(topic);
  mqttClient.subscribe(topic);
}

void loop() {
  mqttClient.poll();
}

void onMqttMessage(int messageSize) {
  Serial.println("Received message.");

  if(messageSize > 3) {
    return; // only ever expecting 3 digits (0-100)
  }

  digitalWrite(LED_BUILTIN, HIGH);

  static char message[4]; // buffer needs to be big enough for 3 chars plus null terminator
  mqttClient.readBytes(message, messageSize);
  Serial.println(message);

  digitalWrite(LED_BUILTIN, LOW);

  updateOutputs(atoi(message));
  message[0] = 0;
  message[1] = 0;
  message[2] = 0;
  message[3] = 0;
}

void updateOutputs(int percent) {
  int vu_meter_duty = (int)floor(24 * (percent / 100.0));
  analogWrite(vu_pin, vu_meter_duty);

  if(percent < 33) {
    // green
    analogWrite(r_pin, 255); // off
    analogWrite(g_pin, 0);   // on
    analogWrite(b_pin, 255); // off
  }
  else if (percent < 66) {
    // yellow
    analogWrite(r_pin, 0);   // on
    analogWrite(g_pin, 0);   // on
    analogWrite(b_pin, 255); // off
  }
  else {
    // red
    analogWrite(r_pin, 0);   // on
    analogWrite(g_pin, 255); // off
    analogWrite(b_pin, 255); // off
  }
}
