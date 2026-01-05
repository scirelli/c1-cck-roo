#include <Adafruit_NeoPixel.h>
#include <stdbool.h>
#include "rooDisplay.h"
#include "mqtt.h"

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define ANALOG_MAX         1023.0f
#define ANALOG_V           3.3f
#define NUMPIXELS         12

//==== Pin out ====
//#define BOOT_EN_PIN       (B0)  // Used to enable flashing
//#define BAT_PIN           (BAT) // Tied to positive battery  terminal
//#define USB_PIN           (USB) // Tied to 5v of the USB C

#define SD_CS_PIN       13    // SDcard Chip Select (E-Ink)
#define EPD_DC_PIN      6     // Data/Command Pin E-Ink
#define EPD_CS_PIN      5     // E-Ink Chip Select
#define EPD_BUSY_PIN    12    // E-Ink Busy pin, can set to -1 to not use a pin (will wait a fixed delay)
#define SRAM_CS_PIN     9     // SRAM Chip Select (E-Ink)
#define EPD_RESET_PIN   11    // E-Ink Reset pin, can set to -1 and share with microcontroller Reset!
#define EPD_SPI_PIN     &SPI  // primary SPI

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
static int prvButtonState = false;
static int analogValues[5];

void gpio_setup();
void neopixels_setup();

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

static void updateStrip() {
  static uint16_t counter = 0;
  strip.setPixelColor(0, strip.gamma32(strip.ColorHSV(counter++, 255, 55)));
  strip.show();
}

void gpio_setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLDOWN);
}

void neopixels_setup() {
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

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  mqtt_setup();
  gpio_setup();
  //neopixels_setup();
  //display_setup();
}

void loop() {
  readAnalogSensores();
  //updateBuiltinNeoPixel();
  //Serial.println(analogValues[3]);
  //updateStrip();

  int buttonState = digitalRead(BTN_PIN);
  if(prvButtonState != buttonState) {
    Serial.print("Btn: "); Serial.println(buttonState);
    prvButtonState = buttonState;
  }

  display_loop();
  mqtt_loop();
}
