#define TINY_GSM_MODEM_SIM800

#include <Arduino.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

SoftwareSerial SerialAT(D2, D1);
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

unsigned long lastHeartbeat = 0;

void ensureNetworkAndMqttConnectivity()
{
  if (!modem.isNetworkConnected())
  {
    Serial.println("Network not connected");
    while (!modem.waitForNetwork(5000))
    {
      Serial.println("Network connectivity failed. Retrying in 1 second.");
      delay(1000);
    }
    Serial.println("Connected to network");
  }
  if (!modem.isGprsConnected())
  {
    Serial.println("GPRS not connected");
    while (!modem.isGprsConnected())
    {
      if (!modem.gprsConnect("net"))
      {
        Serial.println("GPRS connectivity failed. Retrying in 1 second.");
        delay(1000);
      }
    }
    Serial.println("Connected to GPRS");
  }
  if (!mqtt.connected())
  {
    Serial.println("MQTT not connected");
    while (!mqtt.connected())
    {
      if (!mqtt.connect("mondeo", "zvyhbhor:zvyhbhor", "CDWguXi3eA1_gAafB6fkwcp76k6Js8sC"))
      {
        Serial.println("MQTT connectivity failed. Retrying in 1 second.");
        delay(1000);
      }
      else
      {
        mqtt.subscribe("commands/mondeo");
      }
    }
    Serial.println("Connected to MQTT");
  }
}

void handleCommand(String payload)
{
  payload.toLowerCase();
  if (payload == "reboot")
  {
    Serial.println("[Command] Rebooting...");
    ESP.restart();
    return;
  }

  Serial.println("[Command] Unknown command");
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  String topicStr = String(topic);
  String payloadStr = "";
  for (unsigned int i = 0; i < length; i++)
  {
    payloadStr += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topicStr);
  Serial.print("] ");
  Serial.print(payloadStr);
  Serial.println();
  if (topicStr.startsWith("commands/"))
  {
    handleCommand(payloadStr);
    return;
  }
}

void setNetworkParameters()
{
  SerialAT.begin(9600);
  modem.init();
  mqtt.setServer("cow.rmq2.cloudamqp.com", 1883);
  mqtt.setBufferSize(1024);
  mqtt.setCallback(mqttCallback);
}

void setupPowerSaving()
{
  WiFi.mode(WIFI_OFF);
}

void setupRadioRx()
{
  pinMode(D8, OUTPUT);
  digitalWrite(D8, LOW);
}

void heartbeat() {
  mqtt.publish("heartbeat/mondeo", "mondeo", true);
}

void setup()
{
  setupPowerSaving();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(9600);
  Serial.println("Begin setup");

  setupRadioRx();

  setNetworkParameters();
  ensureNetworkAndMqttConnectivity();

  Serial.println("End setup");

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
  digitalWrite(LED_BUILTIN, LOW);
  unsigned long currentTime = millis();
  ensureNetworkAndMqttConnectivity();
  mqtt.loop();
  if (lastHeartbeat == 0 || currentTime - lastHeartbeat >= 30000) {
    heartbeat();
    lastHeartbeat = currentTime;
  }
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
}