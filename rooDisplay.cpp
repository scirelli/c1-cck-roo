#include "rooDisplay.h"

// Tri-Color Displays
static ThinkInk_154_Tricolor_Z90 display(EPD_DC_PIN, EPD_RESET_PIN, EPD_CS_PIN, SRAM_CS_PIN, EPD_BUSY_PIN, EPD_SPI_PIN);
static QRCodeGFX qrcode(display);

void display_setup()
{
  pinMode(EPD_CS_PIN, OUTPUT);
  digitalWrite(EPD_CS_PIN, HIGH);
  pinMode(SD_CS_PIN, OUTPUT);
  digitalWrite(SD_CS_PIN, HIGH);
  pinMode(SRAM_CS_PIN, OUTPUT);
  digitalWrite(SRAM_CS_PIN, HIGH);

  Serial.println(F("Init display..."));
  display.begin(THINKINK_TRICOLOR);
  display.fillScreen(EPD_BLACK);

  Serial.println(F("Init QR..."));
  qrcode.setScale(4)                  //1 to 20
    .setColors(EPD_BLACK, EPD_RED);
  qrcode.getGenerator()
      .setErrorCorrectionLevel(QRCodeECCLevel::High)
      .setVersion(5);
}

bool display_draw_imme(const char *text)
{
  if(!qrcode.draw(text, 15, 15)) {
    Serial.println(F("Failed to generate QR code!"));
    return false;
  }
  display.display();
  return true;
}

void display_loop(long timeMs)
{
}
