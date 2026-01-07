#include <Adafruit_NeoPixel.h>
#include <stdbool.h>
#include <Ethernet.h>
#include "rooDisplay.h"
#include "mqtt.h"
#include "message.h"

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
//#define BAT_PIN           (BAT) // Tied to positive battery  terminal
//#define USB_PIN           (USB) // Tied to 5v of the USB C

#define ETH_CS_PIN          10

#define IR_1_PIN                          (A0)
#define IR_2_PIN                          (A1)
#define IR_3_PIN                          (A2)
#define IR_4_PIN                          0
#define IR_5_PIN                          0
#define IR_6_PIN                          0
#define AMBIENT_LIGHT_SENSE_PIN           (A3)
#define MOTOR_OVERCURRENT_SENSE_PIN       0
#define SYSTEM_12V_OVERCURRENT_SENSE_PIN  0
#define IR_PIN_LIST  {(IR_1_PIN), (IR_2_PIN), (IR_3_PIN)}
#define ALL_ADC_PINS {(IR_1_PIN), (IR_2_PIN), (IR_3_PIN), (IR_4_PIN), (IR_5_PIN), (IR_6_PIN), (AMBIENT_LIGHT_SENSE_PIN),(MOTOR_OVERCURRENT_SENSE_PIN),(SYSTEM_12V_OVERCURRENT_SENSE_PIN)}
#define ADC_COUNT 4

#define BUILT_IN_PIXEL_PIN      8
#define BTN_PIN                 (A4)
#define NEO_STRIP_PIN           (A5)
#define VBAT_PIN                (A6)  // Pin for reading battery voltage

//=================

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
static void ethernet_loop(unsigned long timeMs);
static void readAnalogSensores(unsigned long timeMs);
static void updateBuiltinNeoPixel(unsigned long timeMs);
static void updateStrip(unsigned long timeMs);
static void print_WIZnet_chip_id(EthernetHardwareStatus id);

// Override the default system clock configuration
// This forces: 12MHz Crystal -> 168MHz CPU | 48MHz USB
extern "C" void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  // 1. Configure the main internal regulator output voltage
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  // 2. Initialize the PLL with HSE (External 12MHz Crystal)
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  // PLL Math: 12MHz / M(12) * N(336) / P(2) = 168MHz
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7; // 12/12*336/7 = 48MHz for USB

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    // Initialization Error - Stuck here if crystal is bad/missing
    while (1);
  }

  // 3. Initialize the CPU, AHB and APB buses clocks
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
    while (1);
  }

  // 4. Update the global clock variable so millis() calculates correctly
  SystemCoreClockUpdate();

  // 5. Re-init SysTick for the new clock speed
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
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



static void ethernet_loop(unsigned long timeMs)
{
  static long prevTime = 0;

  if((timeMs - prevTime) >= 1000) {
    Ethernet.maintain();
  }
}

static void readAnalogSensores(unsigned long timeMs)
{
  outgoingMsg.adc_data[0] = analogRead(IR_1_PIN);
  outgoingMsg.adc_data[1] = analogRead(IR_2_PIN);
  outgoingMsg.adc_data[2] = analogRead(IR_3_PIN);
  outgoingMsg.adc_data[6] = analogRead(AMBIENT_LIGHT_SENSE_PIN);
  //Serial.print(outgoingMsg.adc_data[0]); Serial.print(", ");
  //Serial.print(outgoingMsg.adc_data[1]); Serial.print(", ");
  //Serial.print(outgoingMsg.adc_data[2]); Serial.print(", ");
  //Serial.println(outgoingMsg.adc_data[6]);
}

static void updateBuiltinNeoPixel(unsigned long timeMs)
{
  static uint8_t pixelColor[3];
  int sensorValue = outgoingMsg.adc_data[0];
  pixelColor[0] = (uint8_t)(255 * (sensorValue/ANALOG_MAX));
  float voltage = sensorValue * (ANALOG_V / ANALOG_MAX);  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.3V):
  //Serial.print("R1 (V): ");Serial.print(voltage);Serial.print("\t");Serial.println(sensorValue);

  sensorValue = outgoingMsg.adc_data[1];
  voltage = sensorValue * (ANALOG_V / ANALOG_MAX);
  pixelColor[1] = (uint8_t)(255 * (sensorValue/ANALOG_MAX));
  //Serial.print("R2 (V): ");Serial.print(voltage);Serial.print("\t");Serial.println(sensorValue);

  sensorValue = outgoingMsg.adc_data[2];
  voltage = sensorValue * (ANALOG_V / ANALOG_MAX);
  pixelColor[2] = (uint8_t)(255 * (sensorValue/ANALOG_MAX));
  //Serial.print("R3 (V): ");Serial.print(voltage);Serial.print("\t");Serial.println(sensorValue);
  //builtInNeo.setPixelColor(0, builtInNeo.gamma32(builtInNeo.ColorHSV(counter++, 255, 55))); //builtInNeo.Color(r,g,b);
  builtInNeo.setPixelColor(0, builtInNeo.gamma32(builtInNeo.Color(pixelColor[0], pixelColor[1], pixelColor[2])));
  builtInNeo.show();
}

static void updateStrip(unsigned long timeMs)
{
  static uint16_t counter = 0;
  uint32_t color = strip.gamma32(strip.ColorHSV(counter++, 255, 55));
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

static void gpio_loop(unsigned long timeMs)
{
  static int prvButtonState = false;
  int buttonState = digitalRead(BTN_PIN);
  if(prvButtonState != buttonState) {
    Serial.print("Btn: "); Serial.println(buttonState);
    prvButtonState = buttonState;
  }
}

#define D (2 * 1000000)
static void publishMessage(unsigned long timeMs) {
  static unsigned long lastMsg = 0;
  static char msg[50];

  //unsigned long now = micros();
  if ((timeMs - lastMsg) > 2000L) {
      snprintf (msg, 50, "timeMs: %ld lastMsg: %ld e: %ld", timeMs, lastMsg, timeMs - lastMsg);
      Serial.println(msg);
      lastMsg = timeMs;
      return;
      lastMsg = timeMs;
      Serial.println(F("Publish message: "));
      send(&outgoingMsg);
  }
}

/* Temp function */
static void drawOnDisplay() {
  Serial.println(F("Draw QR..."));
  display_draw_imme("https://www.capitalone.com");
  // Draw a small black and white QR code
  // Parameters:
  //   text: content to encode
  //   x: horizontal position (upper left corner)
  //   y: vertical position (upper left corner)
  //if(!qrcode.draw("https://www.capitalone.com", 15, 15)) {
    // Error generating QR code!
    // Possible causes:
    // - Text too long for selected version
    // - Not enough memory
  //  Serial.println(F("Failed to generate QR code!"));
  //}
  //display.display();
}

void setup() {
  SystemClock_Config();
  SystemCoreClock = 168000000;
  HAL_SYSTICK_Config(SystemCoreClock / 1000);
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

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
    .pub_topic = "sensors",
    .domain = "broker.mqtt-dashboard.com",
    .port = 1883,
    .callback = mqtt_callback,
    .ethClient = &ethClient
  });

}

void loop() {
  readAnalogSensores(millis());
  updateBuiltinNeoPixel(millis());
  updateStrip(millis());
  gpio_loop(millis());
  display_loop(millis());
  mqtt_loop(millis());
  ethernet_loop(millis());
  publishMessage(millis());
}
