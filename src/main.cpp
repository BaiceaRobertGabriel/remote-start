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

void mqttCallback(char *topic, byte *payload, unsigned int len)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.write(payload, len);
  Serial.println();
}

void setNetworkParameters()
{
  SerialAT.begin(115200);
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

void setup()
{
  setupPowerSaving();

  Serial.begin(115200);
  Serial.println("Begin setup");

  setupRadioRx();

  setNetworkParameters();
  ensureNetworkAndMqttConnectivity();
  
  Serial.println("End setup");
}

void loop()
{

  ensureNetworkAndMqttConnectivity();
  mqtt.loop();

  Serial.println("Test...");
  
  delay(1000);
}