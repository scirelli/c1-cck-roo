#include "mqtt.h"

static void callback(char* topic, byte* payload, unsigned int length);
static void reconnect();

static byte mac[] = { 0x98, 0x76, 0xB6, 0x13, 0x37, 0x0F };
// Set the static IP address to use if the DHCP fails to assign
static IPAddress ip(172, 16, 0, 15);
static IPAddress myDns(9, 9, 9, 9);

// if you don't want to use DNS (and reduce your sketch size) use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
static char server[] = "broker.mqtt-dashboard.com";    // name address for mqtt broker (using DNS)
static long lastMsg = 0;
static char msg[50];
static long value = 0;
static EthernetClient ethClient;
static PubSubClient client(ethClient);
static bool failedInit = false;

//Move this to a config obj
static void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

static void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient72")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic","hello world");
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

void mqtt_setup()
{
  Ethernet.init(10);  // Most Arduino shields

  client.setServer(server, 1883);
  client.setCallback(callback);

  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
     // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      failedInit = true;
      return;
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to configure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // Allow the hardware to sort itself out
  delay(MQTT_SETUP_DELAY);
}

void mqtt_loop()
{
  if(failedInit) return;
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 50, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
}
