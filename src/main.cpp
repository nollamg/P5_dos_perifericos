#include <Arduino.h>
#include <Wire.h>

// ===============================
// CONFIGURACION I2C
// ===============================
constexpr uint8_t SDA_PIN = 8;
constexpr uint8_t SCL_PIN = 9;
constexpr uint32_t I2C_FREQ = 400000;
constexpr uint32_t SERIAL_BAUD = 115200;

// ===============================
// DIRECCIONES I2C
// ===============================
constexpr uint8_t OLED_ADDR = 0x3C;
constexpr uint8_t AHT_ADDR  = 0x38;

// ===============================
// OLED SSD1306 128x64
// ===============================
constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr size_t FRAMEBUFFER_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT / 8;

uint8_t framebuffer[FRAMEBUFFER_SIZE];

// ===============================
// FUENTE 5x7 BASICA
// Solo incluye caracteres que usaremos
// ===============================
const uint8_t font5x7[][5] = {
  // espacio 32
  {0x00,0x00,0x00,0x00,0x00}, // ' '
  {0x00,0x00,0x5F,0x00,0x00}, // !
  {0x00,0x07,0x00,0x07,0x00}, // "
  {0x14,0x7F,0x14,0x7F,0x14}, // #
  {0x24,0x2A,0x7F,0x2A,0x12}, // $
  {0x23,0x13,0x08,0x64,0x62}, // %
  {0x36,0x49,0x55,0x22,0x50}, // &
  {0x00,0x05,0x03,0x00,0x00}, // '
  {0x00,0x1C,0x22,0x41,0x00}, // (
  {0x00,0x41,0x22,0x1C,0x00}, // )
  {0x14,0x08,0x3E,0x08,0x14}, // *
  {0x08,0x08,0x3E,0x08,0x08}, // +
  {0x00,0x50,0x30,0x00,0x00}, // ,
  {0x08,0x08,0x08,0x08,0x08}, // -
  {0x00,0x60,0x60,0x00,0x00}, // .
  {0x20,0x10,0x08,0x04,0x02}, // /
  {0x3E,0x51,0x49,0x45,0x3E}, // 0
  {0x00,0x42,0x7F,0x40,0x00}, // 1
  {0x42,0x61,0x51,0x49,0x46}, // 2
  {0x21,0x41,0x45,0x4B,0x31}, // 3
  {0x18,0x14,0x12,0x7F,0x10}, // 4
  {0x27,0x45,0x45,0x45,0x39}, // 5
  {0x3C,0x4A,0x49,0x49,0x30}, // 6
  {0x01,0x71,0x09,0x05,0x03}, // 7
  {0x36,0x49,0x49,0x49,0x36}, // 8
  {0x06,0x49,0x49,0x29,0x1E}, // 9
  {0x00,0x36,0x36,0x00,0x00}, // :
  {0x00,0x56,0x36,0x00,0x00}, // ;
  {0x08,0x14,0x22,0x41,0x00}, // <
  {0x14,0x14,0x14,0x14,0x14}, // =
  {0x00,0x41,0x22,0x14,0x08}, // >
  {0x02,0x01,0x51,0x09,0x06}, // ?
  {0x32,0x49,0x79,0x41,0x3E}, // @
  {0x7E,0x11,0x11,0x11,0x7E}, // A
  {0x7F,0x49,0x49,0x49,0x36}, // B
  {0x3E,0x41,0x41,0x41,0x22}, // C
  {0x7F,0x41,0x41,0x22,0x1C}, // D
  {0x7F,0x49,0x49,0x49,0x41}, // E
  {0x7F,0x09,0x09,0x09,0x01}, // F
  {0x3E,0x41,0x49,0x49,0x7A}, // G
  {0x7F,0x08,0x08,0x08,0x7F}, // H
  {0x00,0x41,0x7F,0x41,0x00}, // I
  {0x20,0x40,0x41,0x3F,0x01}, // J
  {0x7F,0x08,0x14,0x22,0x41}, // K
  {0x7F,0x40,0x40,0x40,0x40}, // L
  {0x7F,0x02,0x0C,0x02,0x7F}, // M
  {0x7F,0x04,0x08,0x10,0x7F}, // N
  {0x3E,0x41,0x41,0x41,0x3E}, // O
  {0x7F,0x09,0x09,0x09,0x06}, // P
  {0x3E,0x41,0x51,0x21,0x5E}, // Q
  {0x7F,0x09,0x19,0x29,0x46}, // R
  {0x46,0x49,0x49,0x49,0x31}, // S
  {0x01,0x01,0x7F,0x01,0x01}, // T
  {0x3F,0x40,0x40,0x40,0x3F}, // U
  {0x1F,0x20,0x40,0x20,0x1F}, // V
  {0x3F,0x40,0x38,0x40,0x3F}, // W
  {0x63,0x14,0x08,0x14,0x63}, // X
  {0x07,0x08,0x70,0x08,0x07}, // Y
  {0x61,0x51,0x49,0x45,0x43}, // Z
};

void clearFramebuffer() {
  memset(framebuffer, 0x00, sizeof(framebuffer));
}

void setPixel(int x, int y, bool on = true) {
  if (x < 0 || y < 0 || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
  size_t index = x + (y / 8) * SCREEN_WIDTH;
  uint8_t mask = 1 << (y & 7);
  if (on) framebuffer[index] |= mask;
  else framebuffer[index] &= ~mask;
}

void drawChar5x7(int x, int y, char c, bool scale2 = false) {
  if (c < 32 || c > 90) c = ' ';
  const uint8_t *glyph = font5x7[c - 32];

  for (int col = 0; col < 5; col++) {
    uint8_t bits = glyph[col];
    for (int row = 0; row < 7; row++) {
      bool on = bits & (1 << row);
      if (scale2) {
        if (on) {
          setPixel(x + col * 2,     y + row * 2,     true);
          setPixel(x + col * 2 + 1, y + row * 2,     true);
          setPixel(x + col * 2,     y + row * 2 + 1, true);
          setPixel(x + col * 2 + 1, y + row * 2 + 1, true);
        }
      } else {
        if (on) setPixel(x + col, y + row, true);
      }
    }
  }
}

void drawText(int x, int y, const String &text, bool scale2 = false) {
  int cursorX = x;
  for (size_t i = 0; i < text.length(); i++) {
    drawChar5x7(cursorX, y, text[i], scale2);
    cursorX += scale2 ? 12 : 6;
  }
}

// ===============================
// OLED
// ===============================
bool oledWriteCommand(uint8_t cmd) {
  Wire.beginTransmission(OLED_ADDR);
  Wire.write(0x00);
  Wire.write(cmd);
  return Wire.endTransmission() == 0;
}

bool oledWriteCommands(const uint8_t *cmds, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (!oledWriteCommand(cmds[i])) return false;
  }
  return true;
}

bool oledWriteData(const uint8_t *data, size_t len) {
  size_t offset = 0;
  while (offset < len) {
    Wire.beginTransmission(OLED_ADDR);
    Wire.write(0x40);
    size_t chunk = min(static_cast<size_t>(16), len - offset);
    for (size_t i = 0; i < chunk; i++) {
      Wire.write(data[offset + i]);
    }
    if (Wire.endTransmission() != 0) return false;
    offset += chunk;
  }
  return true;
}

bool oledInit() {
  const uint8_t initSeq[] = {
    0xAE,
    0xD5, 0x80,
    0xA8, 0x3F,
    0xD3, 0x00,
    0x40,
    0x8D, 0x14,
    0x20, 0x00,
    0xA1,
    0xC8,
    0xDA, 0x12,
    0x81, 0xCF,
    0xD9, 0xF1,
    0xDB, 0x40,
    0xA4,
    0xA6,
    0x2E,
    0xAF
  };
  return oledWriteCommands(initSeq, sizeof(initSeq));
}

bool oledShow() {
  const uint8_t addrSeq[] = {
    0x21, 0x00, 0x7F,
    0x22, 0x00, 0x07
  };
  if (!oledWriteCommands(addrSeq, sizeof(addrSeq))) return false;
  return oledWriteData(framebuffer, FRAMEBUFFER_SIZE);
}

// ===============================
// AHT10 / AHT20
// ===============================
bool pingI2C(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

bool ahtInit() {
  Wire.beginTransmission(AHT_ADDR);
  Wire.write(0xE1);
  Wire.write(0x08);
  Wire.write(0x00);
  return Wire.endTransmission() == 0;
}

bool ahtTriggerMeasurement() {
  Wire.beginTransmission(AHT_ADDR);
  Wire.write(0xAC);
  Wire.write(0x33);
  Wire.write(0x00);
  return Wire.endTransmission() == 0;
}

bool ahtRead(float &temperatureC, float &humidityRH) {
  if (!ahtTriggerMeasurement()) {
    return false;
  }

  delay(80);

  Wire.requestFrom(AHT_ADDR, (uint8_t)6);
  if (Wire.available() < 6) {
    return false;
  }

  uint8_t b0 = Wire.read();
  uint8_t b1 = Wire.read();
  uint8_t b2 = Wire.read();
  uint8_t b3 = Wire.read();
  uint8_t b4 = Wire.read();
  uint8_t b5 = Wire.read();

  if (b0 & 0x80) {
    return false;
  }

  uint32_t rawHumidity = ((uint32_t)b1 << 12) | ((uint32_t)b2 << 4) | (b3 >> 4);
  uint32_t rawTemp = (((uint32_t)b3 & 0x0F) << 16) | ((uint32_t)b4 << 8) | b5;

  humidityRH = (rawHumidity * 100.0f) / 1048576.0f;
  temperatureC = (rawTemp * 200.0f) / 1048576.0f - 50.0f;

  return true;
}

// ===============================
// PANTALLA DE DATOS
// ===============================
void drawSensorScreen(float tempC, float humRH) {
  clearFramebuffer();

  drawText(18, 2, "AHT SENSOR", false);
  drawText(0, 18, "TEMP:", false);
  drawText(0, 42, "HUM :", false);

  String t = String(tempC, 1) + " C";
  String h = String(humRH, 1) + " %";

  drawText(42, 14, t, true);
  drawText(42, 38, h, true);

  oledShow();
}

void drawErrorScreen(const char *line1, const char *line2 = "") {
  clearFramebuffer();
  drawText(0, 8, "ERROR", true);
  drawText(0, 34, String(line1), false);
  drawText(0, 46, String(line2), false);
  oledShow();
}

// ===============================
// SETUP / LOOP
// ===============================
void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1000);

  Serial.println();
  Serial.println("Iniciando sistema I2C...");
  Serial.println("SDA = GPIO 8");
  Serial.println("SCL = GPIO 9");

  Wire.begin(SDA_PIN, SCL_PIN, I2C_FREQ);
  Wire.setTimeOut(50);

  if (!pingI2C(OLED_ADDR)) {
    Serial.println("No responde la OLED en 0x3C");
    while (true) delay(1000);
  }

  if (!pingI2C(AHT_ADDR)) {
    Serial.println("No responde el sensor en 0x38");
    oledInit();
    drawErrorScreen("NO SENSOR 0x38");
    while (true) delay(1000);
  }

  if (!oledInit()) {
    Serial.println("Error al inicializar la OLED");
    while (true) delay(1000);
  }

  if (!ahtInit()) {
    Serial.println("Error al inicializar AHT");
    drawErrorScreen("AHT INIT FAIL");
    while (true) delay(1000);
  }

  drawErrorScreen("INICIANDO...");
  delay(800);
}

void loop() {
  float temperature = 0.0f;
  float humidity = 0.0f;

  if (ahtRead(temperature, humidity)) {
    Serial.print("Temperatura: ");
    Serial.print(temperature, 1);
    Serial.print(" C | Humedad: ");
    Serial.print(humidity, 1);
    Serial.println(" %");

    drawSensorScreen(temperature, humidity);
  } else {
    Serial.println("Error leyendo el sensor 0x38");
    drawErrorScreen("ERROR LECTURA", "SENSOR 0x38");
  }

  delay(500);
}