#define TINY_GSM_MODEM_SIM800


#include <Arduino.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>


// #define DUMP_AT_COMMANDS


#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(Serial1, Serial);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(Serial1);
#endif
TinyGsmClient client(modem);
PubSubClient  mqtt(client);


void ensureNetworkAndMqttConnectivity() {
  if (!modem.isNetworkConnected()) {
    while (!modem.waitForNetwork()) {
      delay(1000);
    }
  }
  if (!modem.isGprsConnected()) {
    while (!modem.isGprsConnected()) {
      if (!modem.gprsConnect("net")) {
        delay(1000);
      }
    }
  }
  if (!mqtt.connected()) {
    while (!mqtt.connected()) {
      if (!mqtt.connect("mondeo", "zvyhbhor:zvyhbhor", "CDWguXi3eA1_gAafB6fkwcp76k6Js8sC")) {
        delay(1000);
      } else {
        mqtt.subscribe("commands/mondeo");
      }
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.write(payload, len);
  Serial.println();
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial1.begin(115200);
  modem.init();
  mqtt.setCallback(mqttCallback);
  ensureNetworkAndMqttConnectivity();
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  ensureNetworkAndMqttConnectivity();
  mqtt.loop();
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}