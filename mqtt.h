#ifndef _MQTT_H_
#define _MQTT_H_
#include <stdbool.h>

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

#define MQTT_SETUP_DELAY 1500

void mqtt_setup();
void mqtt_loop();

#endif /* _MQTT_H_ */
