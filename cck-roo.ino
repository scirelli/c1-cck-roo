#include <Adafruit_NeoPixel.h>
#include <stdbool.h>
#include <Ethernet.h>
#include "rooDisplay.h"
#include "mqtt.h"

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define SERIAL_BAUD_RATE 115200

#define ANALOG_MAX        1023.0f
#define ANALOG_V          3.3f
#define NUMPIXELS         12

//==== Pin out ====
//#define BOOT_EN_PIN       (B0)  // Used to enable flashing
//#define BAT_PIN           (BAT) // Tied to positive battery  terminal
//#define USB_PIN           (USB) // Tied to 5v of the USB C

#define ETH_CS_PIN          10

#define BUILT_IN_PIXEL_PIN  8
#define IR_PIN_1            (A0)
#define IR_PIN_2            (A1)
#define IR_PIN_3            (A2)
#define PHOTO_TRAN_PIN      (A3)
#define BTN_PIN             (A4)
#define NEO_STRIP_PIN       (A5)
#define VBAT_PIN            (A6)  // Pin for reading battery voltage
//=================

static Adafruit_NeoPixel builtInNeo(1, BUILT_IN_PIXEL_PIN, NEO_GRB + NEO_KHZ800);
static Adafruit_NeoPixel strip(NUMPIXELS, NEO_STRIP_PIN, NEO_GRBW + NEO_KHZ800);
static byte mac[] = { 0x98, 0x76, 0xB6, 0x13, 0x37, 0x0F };
// Set the static IP address to use if the DHCP fails to assign
static IPAddress ip(172, 16, 0, 15);
static IPAddress myDns(9, 9, 9, 9);
static EthernetClient ethClient;
static int prvButtonState = false;
static int analogValues[5];

static void gpio_setup();
static void neopixels_setup();
static bool ethernet_setup();
static bool ethernet_setup_dhcp();
static bool ethernet_setup_static_ip();
static void mqtt_callback(char* topic, byte* payload, unsigned int length);


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

static void gpio_setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  //Make sure all CS pins are high to start, let the drivers pull them down when they want to communicate.
  pinMode(ETH_CS_PIN, OUTPUT);
  digitalWrite(ETH_CS_PIN, HIGH);
  pinMode(EPD_CS_PIN, OUTPUT);
  digitalWrite(EPD_CS_PIN, HIGH);
  pinMode(SD_CS_PIN, OUTPUT);
  digitalWrite(SD_CS_PIN, HIGH);
  pinMode(SRAM_CS_PIN, OUTPUT);
  digitalWrite(SRAM_CS_PIN, HIGH);

  pinMode(BTN_PIN, INPUT_PULLDOWN);
}

static void neopixels_setup()
{
  pinMode(NEO_STRIP_PIN, OUTPUT);
  builtInNeo.begin();
  builtInNeo.show();  // Initialize all pixels to 'off'

  strip.begin();
  strip.show();
  strip.setPixelColor(1, strip.Color(5,0,0,0));
  strip.setPixelColor(2, strip.Color(0,5,0,0));
  strip.setPixelColor(3, strip.Color(0,0,5,0));
  strip.setPixelColor(4, strip.Color(0,0,0,5));
  strip.show();
}


static bool ethernet_setup_dhcp()
{
  Serial.println(F("Initialize Ethernet with DHCP:"));
  if (Ethernet.begin(mac) == DHCP_CON_FAIL) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
     // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
      return false;
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println(F("Ethernet cable is not connected."));
      return false;
    }
    // try to configure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print(F("  DHCP assigned IP "));
    Serial.println(Ethernet.localIP());
    print_WIZnet_chip_id(Ethernet.hardwareStatus());
  }
  return true;
}

static bool ethernet_setup_static_ip()
{
    Serial.println(F("Initialize Ethernet with static ip:"));
    Ethernet.begin(mac, ip, myDns);
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
      return false;
    }else {
        Serial.print("  Static IP ");
        Serial.println(Ethernet.localIP());
    }
    return true;
}

static bool ethernet_setup()
{
    Ethernet.init(ETH_CS_PIN);
    return ethernet_setup_dhcp();
}




static void readAnalogSensores()
{
  analogValues[0] = analogRead(IR_PIN_1);
  analogValues[1] = analogRead(IR_PIN_2);
  analogValues[2] = analogRead(IR_PIN_3);
  analogValues[3] = analogRead(PHOTO_TRAN_PIN);
}

static void updateBuiltinNeoPixel()
{
  static uint8_t pixelColor[3];
  int sensorValue = analogValues[0];
  pixelColor[0] = (uint8_t)(255 * (sensorValue/ANALOG_MAX));
  float voltage = sensorValue * (ANALOG_V / ANALOG_MAX);  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.3V):
  //Serial.print("R1 (V): ");Serial.print(voltage);Serial.print("\t");Serial.println(sensorValue);

  sensorValue = analogValues[1];
  voltage = sensorValue * (ANALOG_V / ANALOG_MAX);
  pixelColor[1] = (uint8_t)(255 * (sensorValue/ANALOG_MAX));
  //Serial.print("R2 (V): ");Serial.print(voltage);Serial.print("\t");Serial.println(sensorValue);

  sensorValue = analogValues[2];
  voltage = sensorValue * (ANALOG_V / ANALOG_MAX);
  pixelColor[2] = (uint8_t)(255 * (sensorValue/ANALOG_MAX));
  //Serial.print("R3 (V): ");Serial.print(voltage);Serial.print("\t");Serial.println(sensorValue);
  //builtInNeo.setPixelColor(0, builtInNeo.gamma32(builtInNeo.ColorHSV(counter++, 255, 55))); //builtInNeo.Color(r,g,b);
  builtInNeo.setPixelColor(0, builtInNeo.gamma32(builtInNeo.Color(pixelColor[0], pixelColor[1], pixelColor[2])));
  builtInNeo.show();
}

static void updateStrip()
{
  static uint16_t counter = 0;
  strip.setPixelColor(0, strip.gamma32(strip.ColorHSV(counter++, 255, 55)));
  strip.show();
}

static void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}




void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) delay(10);

  gpio_setup();
  display_setup();
  ethernet_setup();
  mqtt_setup((mqtt_conf_t){
    .id = "arduinoClient42",
    .sub_topic = "inTopic",
    .pub_topic = "outTopic",
    .domain = "broker.mqtt-dashboard.com",
    .port = 1883,
    .callback = mqtt_callback,
    .ethClient = &ethClient
  });

  neopixels_setup();
}

void loop() {
  readAnalogSensores();
  updateBuiltinNeoPixel();
  //Serial.println(analogValues[3]);
  updateStrip();

  int buttonState = digitalRead(BTN_PIN);
  if(prvButtonState != buttonState) {
    Serial.print("Btn: "); Serial.println(buttonState);
    prvButtonState = buttonState;
  }

  display_loop();
  mqtt_loop();
}
