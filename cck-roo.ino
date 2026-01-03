#include <Adafruit_NeoPixel.h>

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define ANALOG_MAX         1023.0f
#define ANALOG_V           3.3f
#define MIN_REFRESH_DELAY (3 * 60 * 1000)
#define NUMPIXELS         12

//==== Pin out ====
#define VBAT_PIN            (A6)

#define SD_CS_PIN       13    // SDcard Chip Select (E-Ink)
#define EPD_DC_PIN      6     // Data/Command Pin E-Ink
#define EPD_CS_PIN      5     // E-Ink Chip Select
#define EPD_BUSY_PIN    12    // E-Ink Busy pin, can set to -1 to not use a pin (will wait a fixed delay)
#define SRAM_CS_PIN     9     // SRAM Chip Select (E-Ink)
#define EPD_RESET_PIN   11    // E-Ink Reset pin, can set to -1 and share with microcontroller Reset!
#define EPD_SPI_PIN     &SPI  // primary SPI

#define BUILT_IN_PIXEL_PIN  8
#define NEO_STRIP_PIN       10
#define IR_PIN_1            (A0)
#define IR_PIN_2            (A1)
#define IR_PIN_3            (A2)
#define PHOTO_TRAN_PIN      (A3)
//=================

static Adafruit_NeoPixel strip(NUMPIXELS, BUILT_IN_PIXEL_PIN, NEO_GRB + NEO_KHZ800);
static uint16_t counter = 0;
static uint8_t pixelColor[3];

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  pinMode(LED_BUILTIN, OUTPUT);
  strip.show();  // Initialize all pixels to 'off'
  strip.begin();
}

void loop() {
  int sensorValue = analogRead(IR_PIN_1);
  pixelColor[0] = (uint8_t)(255 * (sensorValue/ANALOG_MAX));
  float voltage = sensorValue * (ANALOG_V / ANALOG_MAX);  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.3V):
  //Serial.print("R1 (V): ");Serial.print(voltage);Serial.print("\t");Serial.println(sensorValue);

  sensorValue = analogRead(IR_PIN_2);
  voltage = sensorValue * (ANALOG_V / ANALOG_MAX);
  pixelColor[1] = (uint8_t)(255 * (sensorValue/ANALOG_MAX));
  //Serial.print("R2 (V): ");Serial.print(voltage);Serial.print("\t");Serial.println(sensorValue);

  sensorValue = analogRead(IR_PIN_3);
  voltage = sensorValue * (ANALOG_V / ANALOG_MAX);
  pixelColor[2] = (uint8_t)(255 * (sensorValue/ANALOG_MAX));
  //Serial.print("R3 (V): ");Serial.print(voltage);Serial.print("\t");Serial.println(sensorValue);

  sensorValue = analogRead(PHOTO_TRAN_PIN);
  Serial.println(sensorValue);

  //strip.setPixelColor(0, strip.gamma32(strip.ColorHSV(counter++, 255, 55))); //strip.Color(r,g,b);
  strip.setPixelColor(0, strip.gamma32(strip.Color(pixelColor[0], pixelColor[1], pixelColor[2])));
  strip.show();
}
