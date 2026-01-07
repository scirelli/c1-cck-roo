#include "mqtt.h"

static bool reconnect();

// if you don't want to use DNS (and reduce your sketch size) use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
//static char server[] = "broker.mqtt-dashboard.com";    // name address for mqtt broker (using DNS)
static long lastMsg = 0;
static char msg[50];
static long value = 0;
static PubSubClient client;
static long lastReconnectAttempt = 0;
static mqtt_conf_t mqtt_config;


static bool reconnect()
{
    if (client.connect(mqtt_config.id)) {
        Serial.println("connected");
        // Once connected, publish an announcement...
        client.publish(mqtt_config.pub_topic, "hello world");
        // ... and resubscribe
        client.subscribe(mqtt_config.sub_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
    }

    return client.connected();
}

bool mqtt_setup(mqtt_conf_t config)
{
    mqtt_config = config;
    if(config.ethClient == nullptr) return false;
    client.setClient(*config.ethClient);
    client.setServer(config.domain, config.port);
    client.setCallback(config.callback);
    // Allow the hardware to sort itself out
    delay(MQTT_SETUP_DELAY);
    return true;
}

void mqtt_loop()
{
    if (!client.connected()) {
        long now = millis();
        if (now - lastReconnectAttempt > 5000) {
            lastReconnectAttempt = now;
            if (reconnect()) {
                lastReconnectAttempt = 0;
            }
        }
    } else {
        client.loop();
    }

    // long now = millis();
    // if (now - lastMsg > 2000) {
    //     lastMsg = now;
    //     ++value;
    //     snprintf (msg, 50, "hello world #%ld", value);
    //     Serial.print(F("Publish message: "));
    //     Serial.println(msg);
    //     client.publish(config.pub_topic, msg);
    // }
}

bool send(const outgoingMsg_t *payload) {
    return client.publish(mqtt_config.pub_topic, (uint8_t*)payload, sizeof(*payload));
}
