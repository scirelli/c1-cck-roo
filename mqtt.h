#ifndef _MQTT_H_
#define _MQTT_H_
#include <stdbool.h>

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include "message.h"

#define MQTT_SETUP_DELAY    1500

#define DHCP_CON_FAIL    0
#define DHCP_CON_SUCCESS 1

typedef void (*callback_t)(char*, uint8_t*, unsigned int);

typedef struct {
    const char *id;
    const char *sub_topic;
    const char *pub_topic;
    const char *domain;
    uint16_t port;
    callback_t callback;
    Client *ethClient;
} mqtt_conf_t;

bool mqtt_setup(mqtt_conf_t);
void mqtt_loop(long timeMs);
bool send(const outgoingMsg_t *payload);

#endif /* _MQTT_H_ */
