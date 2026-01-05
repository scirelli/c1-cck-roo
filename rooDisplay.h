#ifndef _ROO_DISPLAY_
#define _ROO_DISPLAY_
#include <Arduino.h>
#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ThinkInk.h>
#include <SdFat_Adafruit_Fork.h>  // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader_EPD.h> // Image-reading functions
#include <QRCodeGFX.h>

#define MIN_REFRESH_DELAY (3 * 60 * 1000)

//==== Pin out ====
#define SD_CS_PIN       13    // SDcard Chip Select (E-Ink)
#define EPD_DC_PIN      6     // Data/Command Pin E-Ink
#define EPD_CS_PIN      5     // E-Ink Chip Select
#define EPD_BUSY_PIN    12    // E-Ink Busy pin, can set to -1 to not use a pin (will wait a fixed delay)
#define SRAM_CS_PIN     9     // SRAM Chip Select (E-Ink)
#define EPD_RESET_PIN   11    // E-Ink Reset pin, can set to -1 and share with microcontroller Reset!
#define EPD_SPI_PIN     &SPI  // primary SPI
//==================

void display_setup();
void display_loop();
bool display_draw_imme(const char *text);

#endif /* _ROO_DISPLAY_ */
