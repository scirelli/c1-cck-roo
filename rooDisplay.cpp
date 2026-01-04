#include "rooDisplay.h"

// Tri-Color Displays
static ThinkInk_154_Tricolor_Z90 display(EPD_DC_PIN, EPD_RESET_PIN, EPD_CS_PIN, SRAM_CS_PIN, EPD_BUSY_PIN, EPD_SPI_PIN);
static QRCodeGFX qrcode(display);

void setupDisplay() {
  Serial.println(F("Init display..."));
  display.begin(THINKINK_TRICOLOR);
  display.fillScreen(EPD_BLACK);

  Serial.println(F("Init QR..."));
  qrcode.setScale(4)                  //1 to 20
    .setColors(EPD_BLACK, EPD_RED);
  qrcode.getGenerator()
      .setErrorCorrectionLevel(QRCodeECCLevel::High)
      .setVersion(5);

  Serial.println(F("Draw QR..."));
  // Draw a small black and white QR code
  // Parameters:
  //   text: content to encode
  //   x: horizontal position (upper left corner)
  //   y: vertical position (upper left corner)
  if(!qrcode.draw("https://www.capitalone.com", 15, 15)) {
    // Error generating QR code!
    // Possible causes:
    // - Text too long for selected version
    // - Not enough memory
    Serial.println(F("Failed to generate QR code!"));
  }
  display.display();
}
