#include <Adafruit_NeoPixel.h>
#include <stdbool.h>
#include <Ethernet.h>
#include "rooDisplay.h"
#include "mqtt.h"
#include "message.h"

#define MICRO_SEC_SEC 1000000
#define US_MILLI      1000
#define NEOPIXEL_DELAY (100 * US_MILLI)
#define PUBLISH_RATE (5 * MICRO_SEC_SEC)
#define BAT_V_MIN 0.f
#define BAT_V_MAX 165.f
#define MIN_SEND_FREQ 0.2f //Hz
#define MAX_SEND_FREQ 15.f //Hz
#define SEND_FREQ(x) ((((x/BAT_V_MAX) * ((MAX_SEND_FREQ+1) - MIN_SEND_FREQ)) + MIN_SEND_FREQ))
#define SEND_PERIOD_US(x) ((int)((MICRO_SEC_SEC)/(SEND_FREQ(x))))

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define SERIAL_BAUD_RATE 115200

#define ANALOG_RES  8
#if ANALOG_RES == 8
#define ANALOG_MAX        255.0f // 8bit res
#elif ANALOG_RES == 12
#define ANALOG_MAX        4095.0f //10bit res
#else
#define ANALOG_MAX        1023.0f //10bit res
#endif
#define ANALOG_V          3.3f
#define NUMPIXELS         12

//==== Pin out ====
//#define BOOT_EN_PIN       (B0)  // Used to enable flashing
//#define BAT_PIN           (BAT) // Tied to positive battery terminal. Not defined but there is one.
//#define USB_PIN           (USB) // Tied to 5v of the USB C. Not defined but there is one.
#define VBAT_PIN            (A6)  // Pin for reading battery voltage

#define ETH_CS_PIN          10

#define IR_1_PIN                          (A0)
#define IR_2_PIN                          (A1)
#define IR_3_PIN                          (A2)
#define IR_4_PIN                          0
#define IR_5_PIN                          0
#define IR_6_PIN                          0
#define AMBIENT_LIGHT_SENSE_PIN           (A3)
#define MOTOR_OVERCURRENT_SENSE_PIN       0
#define SYSTEM_12V_OVERCURRENT_SENSE_PIN  (VBAT_PIN)
#define IR_PIN_LIST  {(IR_1_PIN), (IR_2_PIN), (IR_3_PIN)}
#define ALL_ADC_PINS {(IR_1_PIN), (IR_2_PIN), (IR_3_PIN), (IR_4_PIN), (IR_5_PIN), (IR_6_PIN), (AMBIENT_LIGHT_SENSE_PIN),(MOTOR_OVERCURRENT_SENSE_PIN),(SYSTEM_12V_OVERCURRENT_SENSE_PIN)}
#define ADC_COUNT 4

#define BTN_PIN                   (A4)
#define LIMIT_SWITCH_FRONT_PIN    (BTN_PIN)
#define LIMIT_SWITCH_REAR_PIN     (BTN_PIN)
#define DOOR_SWITCH_OPEN_PIN      (BTN_PIN)
#define DOOR_SWITCH_CLOSE_PIN     (BTN_PIN)
#define PIXEL_POWER_GOOD_PIN      (BTN_PIN)
#define LCD_POWER_GOOD_PIN        (BTN_PIN)
#define SYSTEM_5V_POWER_GOOD_PIN  (BTN_PIN)

#define BUILT_IN_PIXEL_PIN      8
#define NEO_STRIP_PIN           (A5)
//=================

typedef unsigned long time__t;

static Adafruit_NeoPixel builtInNeo(1, BUILT_IN_PIXEL_PIN, NEO_GRB + NEO_KHZ800);
static Adafruit_NeoPixel strip(NUMPIXELS, NEO_STRIP_PIN, NEO_GRBW + NEO_KHZ800);
static byte mac[] = { 0x98, 0x76, 0xB6, 0x13, 0x37, 0x0F };
// Set the static IP address to use if the DHCP fails to assign
static IPAddress ip(172, 16, 0, 15);
static IPAddress myDns(9, 9, 9, 9);
static EthernetClient ethClient;
static outgoingMsg_t outgoingMsg;



static void gpio_setup();
static void neopixels_setup();
static bool ethernet_setup();
static bool ethernet_setup_dhcp();
static bool ethernet_setup_static_ip();
static void adc_setup();

static void mqtt_callback(char* topic, byte* payload, unsigned int length);
static void ethernet_loop(time__t elapsedTimeUs);
static void sensor_loop(time__t elapsedTimeUs);
static void read_analog_sensors(time__t elapsedTimeUs);
static void read_gpio(time__t elapsedTimeUs);
static void updateBuiltinNeoPixel(time__t elapsedTimeUs);
static void updateStrip(time__t elapsedTimeUs);
static void print_WIZnet_chip_id(EthernetHardwareStatus id);
static void print_message(outgoingMsg_t msg);

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

static void print_message(outgoingMsg_t msg) {
    Serial.print("crc: ");Serial.print(msg.crc);
    Serial.print(" adc0: ");Serial.print(msg.adc_data[0]);
    Serial.print(" adc1: ");Serial.print(msg.adc_data[1]);
    Serial.print(" adc2: ");Serial.print(msg.adc_data[2]);
    Serial.print(" amb: ");Serial.print(msg.adc_data[6]);
    Serial.print(" sys12v: ");Serial.print(msg.adc_data[8]);
    Serial.print(" gpio: ");Serial.println(msg.gpio);
}

static void adc_setup()
{
  analogReadResolution(8);
}

static void gpio_setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  //Make sure all CS pins are high to start, let the drivers pull them down when they want to communicate.
  pinMode(ETH_CS_PIN, OUTPUT);
  digitalWrite(ETH_CS_PIN, HIGH);

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



static void read_analog_sensors(time__t elapsedTimeUs)
{
  outgoingMsg.adc_data[0] = (uint8_t)analogRead(IR_1_PIN);
  outgoingMsg.adc_data[1] = (uint8_t)analogRead(IR_2_PIN);
  outgoingMsg.adc_data[2] = (uint8_t)analogRead(IR_3_PIN);
  outgoingMsg.adc_data[6] = (uint8_t)analogRead(AMBIENT_LIGHT_SENSE_PIN);
  outgoingMsg.adc_data[8] = (uint8_t)analogRead(SYSTEM_12V_OVERCURRENT_SENSE_PIN);
}

static void read_gpio(time__t elapsedTimeUs)
{
  static int prvButtonState = false;
  int buttonState = digitalRead(DOOR_SWITCH_OPEN_PIN);

  if(prvButtonState != buttonState) {
    Serial.print("Btn: "); Serial.println(buttonState);
    prvButtonState = buttonState;
  }

   outgoingMsg.gpio = (
      (digitalRead(LIMIT_SWITCH_FRONT_PIN)   << LIMIT_SWITCH_FRONT_BIT) |
      (digitalRead(LIMIT_SWITCH_REAR_PIN)    << LIMIT_SWITCH_REAR_BIT) |
      (digitalRead(DOOR_SWITCH_OPEN_PIN)     << DOOR_SWITCH_OPEN_BIT) |
      (digitalRead(DOOR_SWITCH_CLOSE_PIN)    << DOOR_SWITCH_CLOSE_BIT) |
      (digitalRead(PIXEL_POWER_GOOD_PIN)     << PIXEL_POWER_GOOD_BIT) |
      (digitalRead(LCD_POWER_GOOD_PIN)       << LCD_POWER_GOOD_BIT) |
      (digitalRead(SYSTEM_5V_POWER_GOOD_PIN) << SYSTEM_5V_POWER_GOOD_BIT)
   );
}

static void sensor_loop(time__t elapsedTimeUs)
{
  read_analog_sensors(elapsedTimeUs);
  read_gpio(elapsedTimeUs);
}

static void ethernet_loop(time__t elapsedTimeUs)
{
  if(elapsedTimeUs >= 1000) {
    Ethernet.maintain();
  }
}

static void updateBuiltinNeoPixel(time__t elapsedTimeUs)
{
  static uint8_t r,g,b;
  static time__t t;
  t += elapsedTimeUs;

  //required delay, if show() disables interrupts, which messes up timing.
  if(t > NEOPIXEL_DELAY) {
    t=0;
    if(r != outgoingMsg.adc_data[0] || g != outgoingMsg.adc_data[1] || b != outgoingMsg.adc_data[2]) {
      builtInNeo.setPixelColor(0, builtInNeo.gamma32(builtInNeo.Color(
        outgoingMsg.adc_data[0],
        outgoingMsg.adc_data[1],
        outgoingMsg.adc_data[2],
        0
      )));
      builtInNeo.show();
      r = outgoingMsg.adc_data[0];
      g = outgoingMsg.adc_data[1];
      b = outgoingMsg.adc_data[2];
    }
  }
}

static void updateStrip(time__t elapsedTimeUs)
{
  static uint16_t counter = 0;
  static time__t t;
  t += elapsedTimeUs;

  //required delay, if show() disables interrupts, which messes up timing.
  if(t > NEOPIXEL_DELAY) {
    t=0;
    counter += 100;
    uint32_t color = strip.gamma32(strip.ColorHSV(counter, 255, 55));
    strip.setPixelColor(0, color);
    strip.setPixelColor(1, color);
    strip.setPixelColor(2, color);
    strip.setPixelColor(3, color);
    strip.setPixelColor(4, color);
    strip.setPixelColor(5, color);
    strip.setPixelColor(6, color);
    strip.setPixelColor(7, color);
    strip.setPixelColor(8, color);
    strip.setPixelColor(9, color);
    strip.setPixelColor(10, color);
    strip.setPixelColor(11, color);
    strip.show();
  }
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

static void publishMessage(time__t elapsedTimeUs) {
  static time__t t = 0;
  t += elapsedTimeUs;
  if (t > SEND_PERIOD_US(outgoingMsg.adc_data[8])) {
      t=0;
      //msg_insertCRC(&outgoingMsg); this is not needed when using mqtt
      Serial.print("Publishing message (");
      Serial.print(SEND_FREQ(outgoingMsg.adc_data[8]));
      //Serial.print(" Hz): ");
      Serial.print("Hz ");
      Serial.print(SEND_PERIOD_US(outgoingMsg.adc_data[8]));
      Serial.print(" us): ");
      print_message(outgoingMsg);
      send(&outgoingMsg);
  }
}

/* Temp function */
static void drawOnDisplay() {
  Serial.println(F("Draw QR..."));
  display_draw_imme("https://www.capitalone.com");
}

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) delay(10);

  adc_setup();
  gpio_setup();
  neopixels_setup();
  display_setup();
  ethernet_setup();
  mqtt_setup((mqtt_conf_t){
    .id = "roo-client-42",
    .sub_topic = "command",
    .pub_topic = "roo-sensors",
    //.domain = "broker.mqtt-dashboard.com",
    .domain = "test.mosquitto.org",
    .port = 1883,
    .callback = mqtt_callback,
    .ethClient = &ethClient
  });
}

void loop() {
  static time__t prevTime = 0;

  time__t curTime = micros();
  time__t elapsed = curTime - prevTime;
  prevTime = curTime;
  sensor_loop(elapsed);
  updateBuiltinNeoPixel(elapsed);
  updateStrip(elapsed);
  publishMessage(elapsed);
  mqtt_loop(elapsed);
  ethernet_loop(elapsed);
}
