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

static void print_WIZnet_chip_id(EthernetHardwareStatus id)
{
    switch(id) {
        case EthernetNoHardware:
            Serial.println("No hardware found");
            break;
        case EthernetW5100:
            Serial.println("EthernetW5100");
            break;
        case EthernetW5200:
            Serial.println("EthernetW5200");
            break;
        case EthernetW5500:
            Serial.println("EthernetW5500");
            break;
        default:
            Serial.println("Unknown");
    }
}

static void connect()
{
  Serial.println(F("Initialize Ethernet with DHCP:"));
  if (Ethernet.begin(mac) == DHCP_CON_FAIL) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
     // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
      failedInit = true;
      return;
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println(F("Ethernet cable is not connected."));
    }
    // try to configure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print(F("  DHCP assigned IP "));
    Serial.println(Ethernet.localIP());
    print_WIZnet_chip_id(Ethernet.hardwareStatus());
  }
}

static int connectWithStaticIP()
{
    Serial.println(F("Initialize Ethernet with static ip:"));
    Ethernet.begin(mac, ip, myDns);
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
      failedInit = true;
    }else {
        Serial.print("  Static IP ");
        Serial.println(Ethernet.localIP());
        failedInit = false;
    }
}

void mqtt_setup()
{
    Ethernet.init(ETH_CS_PIN);
    connect();

    client.setServer(server, 1883);
    client.setCallback(callback);
    // Allow the hardware to sort itself out
    delay(MQTT_SETUP_DELAY);
}

void mqtt_loop()
{
  if(failedInit) {
      //connectWithStaticIP();
      return;
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 50, "hello world #%ld", value);
    Serial.print(F("Publish message: "));
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
}
