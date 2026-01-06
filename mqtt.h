#ifndef _MQTT_H_
#define _MQTT_H_
#include <stdbool.h>

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#define MQTT_SETUP_DELAY    1500
#define ETH_CS_PIN          10

#define DHCP_CON_FAIL    0
#define DHCP_CON_SUCCESS 1

void mqtt_setup();
void mqtt_loop();

#endif /* _MQTT_H_ */
