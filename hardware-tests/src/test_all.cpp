// =========================================================================================
// TEST_ALL - tu kiem tra toan bo ngoai vi robot Xiaozhi (WeAct ESP32-S3 N16R8)
//
// Moi bai test in 1 dong may doc duoc:
//     RESULT|<id>|<ten>|PASS|FAIL|WARN|MANUAL|<chi tiet>
// va cuoi cung:
//     SUMMARY|pass=..|fail=..|warn=..|manual=..
//     TEST_ALL_DONE
// Script test_all.ps1 nap firmware nay, doc log va bao cai nao loi.
// Log in ra ca UART0 (CH343) lan cong USB native, cam cong nao cung doc duoc.
//
// Bai test:
//   CHIP    PSRAM 8MB, flash 16MB
//   I2C     SDA/SCL khong bi chap GND, quet thiet bi
//   TOF     VL53L0X 0x29: XSHUT bat/tat duoc, do khoang cach
//   IMU     MPU6050 0x68/0x69: WHO_AM_I, gia toc ~1 g
//   MIC     INMP441: co du lieu, kenh L/R, muc nhieu hop ly
//   SPK     PCM5100A + loa: phat tieng bip 1 kHz, mic nghe lai (loopback)
//   TFT     ILI9341: ve mau (MANUAL - khong doc nguoc duoc vi MISO de trong)
//   LED     WS2812 GPIO48 (MANUAL)
//   SERVO   4 chan nhich nhe (MANUAL)
//   BOOT    nut BOOT: muc nghi + nhan trong 8 s
//   WIFI    radio Wi-Fi: quet mang
// =========================================================================================
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_VL53L0X.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <driver/i2s.h>
#include <math.h>

#ifndef TFT_CS
#define TFT_CS 39
#define TFT_DC 38
#define TFT_RST 47
#define TFT_MOSI 41
#define TFT_SCLK 40
#define TFT_MISO -1
#endif
#ifndef TFT_SPI_HZ
#define TFT_SPI_HZ 27000000
#endif
#ifndef I2S_PIN_BCLK
#define I2S_PIN_BCLK 16
#define I2S_PIN_WS 17
#define I2S_PIN_DIN 14
#define I2S_PIN_DOUT 15
#endif
#ifndef I2C_SDA_PIN
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#endif
#ifndef TOF_XSHUT_PIN
#define TOF_XSHUT_PIN 21
#endif
#ifndef SERVO_PIN_FL
#define SERVO_PIN_FL 7
#define SERVO_PIN_FR 4
#define SERVO_PIN_RL 6
#define SERVO_PIN_RR 5
#endif
#ifndef SERVO_FR_IS_360
#define SERVO_FR_IS_360 1
#endif
#ifndef PIN_BOOT_BTN
#define PIN_BOOT_BTN 0
#endif
#ifndef STATUS_LED_PIN
#define STATUS_LED_PIN 48
#endif

static const int RATE = 16000;
static SPIClass tftSPI(FSPI);
static Adafruit_ILI9341 tft(&tftSPI, TFT_DC, TFT_CS, TFT_RST);
static bool tftOk = false;
static bool i2sOk = false;

// ---------------------------------------------------------------- log to both ports
#if !ARDUINO_USB_CDC_ON_BOOT
#define HAS_USB_LOG 1
#endif
static void logLine(const String& s) {
  Serial.println(s);
#ifdef HAS_USB_LOG
  if (USBSerial) USBSerial.println(s);
#endif
}

struct Result { const char* id; const char* name; const char* status; String detail; };
static Result results[16];
static int nResults = 0;

static void report(const char* id, const char* name, const char* status, const String& detail) {
  results[nResults++] = {id, name, status, detail};
  logLine(String("RESULT|") + id + "|" + name + "|" + status + "|" + detail);
}

// ---------------------------------------------------------------- CHIP
static void testChip() {
  uint32_t psram = ESP.getPsramSize();
  uint32_t flash = ESP.getFlashChipSize();
  String d = "PSRAM " + String(psram / 1024 / 1024.0, 1) + " MB, flash " + String(flash / 1024 / 1024) + " MB";
  report("CHIP", "ESP32-S3 PSRAM/Flash", (psram >= 7u * 1024 * 1024 && flash >= 15u * 1024 * 1024) ? "PASS" : "FAIL", d);
}

// ---------------------------------------------------------------- TFT
static void testTft() {
  tftSPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
  tft.begin(TFT_SPI_HZ);
  tft.setRotation(1);
  const uint16_t bars[] = {ILI9341_RED, ILI9341_GREEN, ILI9341_BLUE, ILI9341_WHITE};
  for (int i = 0; i < 4; i++) tft.fillRect(i * 80, 0, 80, 240, bars[i]);
  delay(1500);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("TEST_ALL dang chay...");
  tftOk = true;
  report("TFT", "Man hinh ILI9341", "MANUAL",
         "Vua hien 4 cot DO-XANH LA-XANH DUONG-TRANG roi chu 'TEST_ALL'. Khong thay = kiem tra SCK40 MOSI41 CS39 DC38 RST47, VCC/LED 3V3");
}

static void tftStatus(const char* line) {
  if (!tftOk) return;
  tft.fillRect(0, 40, 320, 20, ILI9341_BLACK);
  tft.setCursor(10, 40);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_CYAN);
  tft.print(line);
}

// ---------------------------------------------------------------- I2C + ToF + IMU
static bool i2cPresent(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

static void testI2c() {
  pinMode(I2C_SDA_PIN, INPUT_PULLUP);
  pinMode(I2C_SCL_PIN, INPUT_PULLUP);
  delay(5);
  bool sdaHigh = digitalRead(I2C_SDA_PIN), sclHigh = digitalRead(I2C_SCL_PIN);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, 100000);
  String found;
  int n = 0;
  for (uint8_t a = 1; a < 127; a++) {
    if (i2cPresent(a)) { found += String(" 0x") + String(a, HEX); n++; }
  }
  if (!sdaHigh || !sclHigh) {
    report("I2C", "Bus I2C (SDA8 SCL9)", "FAIL",
           String("Duong ") + (!sdaHigh ? "SDA " : "") + (!sclHigh ? "SCL " : "") + "bi keo xuong GND (chap/cam nham)");
  } else if (n == 0) {
    report("I2C", "Bus I2C (SDA8 SCL9)", "FAIL", "Khong thay thiet bi nao - kiem tra VIN/GND va SDA->8, SCL->9 cua ToF + MPU");
  } else {
    report("I2C", "Bus I2C (SDA8 SCL9)", "PASS", String(n) + " thiet bi:" + found);
  }
}

static void testTof() {
  pinMode(TOF_XSHUT_PIN, OUTPUT);
  digitalWrite(TOF_XSHUT_PIN, LOW);
  delay(20);
  bool presentWhenOff = i2cPresent(0x29);
  digitalWrite(TOF_XSHUT_PIN, HIGH);
  delay(20);
  bool presentWhenOn = i2cPresent(0x29);
  if (!presentWhenOn) {
    report("TOF", "Laser VL53L0X (0x29)", "FAIL", "Khong thay 0x29 - kiem tra VIN 3V3, GND, SDA8, SCL9, XSHUT21");
    return;
  }
  String xs = presentWhenOff ? "XSHUT KHONG tac dung (day XSHUT->GPIO21 chua noi?)" : "XSHUT OK";
  Adafruit_VL53L0X lox;
  if (!lox.begin(0x29, false, &Wire)) {
    report("TOF", "Laser VL53L0X (0x29)", "FAIL", "Co 0x29 nhung khoi tao loi (VL53L1X? nguon yeu?). " + xs);
    return;
  }
  int ok = 0, sum = 0;
  for (int i = 0; i < 5; i++) {
    VL53L0X_RangingMeasurementData_t m;
    lox.rangingTest(&m, false);
    if (m.RangeStatus != 4 && m.RangeMilliMeter < 8000) { ok++; sum += m.RangeMilliMeter; }
    delay(50);
  }
  String d = ok ? ("do duoc " + String(sum / ok) + " mm (" + String(ok) + "/5 lan), ") : String("khong co vat trong tam, ");
  report("TOF", "Laser VL53L0X (0x29)", presentWhenOff ? "WARN" : "PASS", d + xs);
}

static void testImu() {
  uint8_t addr = i2cPresent(0x68) ? 0x68 : (i2cPresent(0x69) ? 0x69 : 0);
  if (!addr) { report("IMU", "MPU6050 (0x68/0x69)", "FAIL", "Khong thay 0x68/0x69 - kiem tra VCC, GND, SDA8, SCL9"); return; }
  auto wr = [&](uint8_t r, uint8_t v) { Wire.beginTransmission(addr); Wire.write(r); Wire.write(v); Wire.endTransmission(); };
  auto rd = [&](uint8_t r, uint8_t* b, int n) {
    Wire.beginTransmission(addr); Wire.write(r); Wire.endTransmission(false);
    Wire.requestFrom((int)addr, n); for (int i = 0; i < n && Wire.available(); i++) b[i] = Wire.read();
  };
  uint8_t who = 0; rd(0x75, &who, 1);
  wr(0x6B, 0x00); delay(100);
  uint8_t b[6] = {0}; rd(0x3B, b, 6);
  float ax = (int16_t)(b[0] << 8 | b[1]) / 16384.0f, ay = (int16_t)(b[2] << 8 | b[3]) / 16384.0f,
        az = (int16_t)(b[4] << 8 | b[5]) / 16384.0f;
  float g = sqrtf(ax * ax + ay * ay + az * az);
  String d = "addr 0x" + String(addr, HEX) + ", WHO_AM_I 0x" + String(who, HEX) + ", |a|=" + String(g, 2) + " g";
  if (g < 0.6f || g > 1.4f) report("IMU", "MPU6050 (0x68/0x69)", "FAIL", d + " (gia toc sai - chip loi/nhai?)");
  else if (who != 0x68) report("IMU", "MPU6050 (0x68/0x69)", "WARN", d + " (chip clone, van dung duoc)");
  else report("IMU", "MPU6050 (0x68/0x69)", "PASS", d);
}

// ---------------------------------------------------------------- I2S mic + speaker
static void i2sStart() {
  i2s_config_t cfg = {};
  cfg.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX);
  cfg.sample_rate = RATE;
  cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
  cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  cfg.dma_buf_count = 6;
  cfg.dma_buf_len = 256;
  cfg.tx_desc_auto_clear = true;
  i2s_pin_config_t pins = {};
  pins.mck_io_num = I2S_PIN_NO_CHANGE;
  pins.bck_io_num = I2S_PIN_BCLK;
  pins.ws_io_num = I2S_PIN_WS;
  pins.data_out_num = I2S_PIN_DOUT;
  pins.data_in_num = I2S_PIN_DIN;
  i2sOk = i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL) == ESP_OK && i2s_set_pin(I2S_NUM_0, &pins) == ESP_OK;
}

// Plays `freq` (0 = silence) for `ms` while recording; returns per-slot stats.
struct MicStats { float level[2]; float tone[2]; bool stuck[2]; };
static MicStats playAndRecord(float freq, int ms, float vol) {
  MicStats st = {};
  const int N = 256;
  static int32_t tx[N * 2], rx[N * 2];
  float phase = 0, inc = 2 * M_PI * (freq > 0 ? freq : 1000) / RATE;
  // Goertzel on 1 kHz for both slots
  float k = 2 * cosf(2 * M_PI * 1000.0f / RATE), s1[2] = {0}, s2[2] = {0};
  double absSum[2] = {0};
  int32_t first[2] = {0};
  bool same[2] = {true, true};
  long frames = 0, total = (long)RATE * ms / 1000;
  while (frames < total) {
    for (int i = 0; i < N; i++) {
      int32_t v = freq > 0 ? (int32_t)(sinf(phase) * 32767 * vol) << 16 : 0;
      tx[i * 2] = tx[i * 2 + 1] = v;
      phase += inc; if (phase > 2 * M_PI) phase -= 2 * M_PI;
    }
    size_t w = 0, r = 0;
    i2s_write(I2S_NUM_0, tx, sizeof(tx), &w, 100);
    i2s_read(I2S_NUM_0, rx, sizeof(rx), &r, 100);
    int got = r / 8;
    for (int i = 0; i < got; i++) {
      for (int c = 0; c < 2; c++) {
        int32_t raw = rx[i * 2 + c];
        if (frames == 0 && i == 0) first[c] = raw;
        else if (raw != first[c]) same[c] = false;
        float x = (raw >> 16);
        absSum[c] += fabsf(x);
        float s0 = x + k * s1[c] - s2[c]; s2[c] = s1[c]; s1[c] = s0;
      }
    }
    frames += got > 0 ? got : N;
  }
  for (int c = 0; c < 2; c++) {
    st.level[c] = absSum[c] / (frames > 0 ? frames : 1);
    st.tone[c] = sqrtf(fmaxf(0, s1[c] * s1[c] + s2[c] * s2[c] - k * s1[c] * s2[c])) / (frames > 0 ? frames : 1);
    st.stuck[c] = same[c] && first[c] != 0;
  }
  return st;
}

static int micSlot = -1;
static void testMic() {
  i2sStart();
  if (!i2sOk) { report("MIC", "Micro INMP441", "FAIL", "Khong khoi tao duoc I2S"); return; }
  playAndRecord(0, 300, 0);  // flush
  MicStats st = playAndRecord(0, 1500, 0);
  String d = "muc L=" + String(st.level[0], 0) + " R=" + String(st.level[1], 0);
  bool act[2];
  for (int c = 0; c < 2; c++) act[c] = st.level[c] > 3 && !st.stuck[c];
  if (st.stuck[0] && st.stuck[1]) {
    report("MIC", "Micro INMP441", "FAIL", d + " - du lieu dung yen (SD bi chap VCC/GND hoac SCK/WS dao nhau)");
  } else if (!act[0] && !act[1]) {
    report("MIC", "Micro INMP441", "FAIL", d + " - KHONG co tin hieu: kiem tra VDD->3V3, SD->14, SCK->16, WS->17 (khong dao)");
  } else {
    micSlot = act[0] && (!act[1] || st.level[0] >= st.level[1]) ? 0 : 1;
    float lv = st.level[micSlot];
    float other = st.level[1 - micSlot];
    // INMP441 chi xuat du lieu o 1 kenh (theo chan L/R), kenh con lai phai gan 0.
    // Phong yen tinh: muc nen thuong vai chuc den vai tram.
    if (act[0] && act[1] && other > 0.3f * lv) {
      micSlot = -1;
      report("MIC", "Micro INMP441", "FAIL",
             d + " - CA 2 kenh deu co 'tin hieu' = nhieu, mic khong dieu khien duong SD: kiem tra day SD->14 (long/ho), VDD 3V3, GND, L/R->GND");
    } else if (lv > 3000) {
      micSlot = -1;
      report("MIC", "Micro INMP441", "FAIL", d + " - muc nen QUA CAO (nhieu, khong phai am thanh): kiem tra day SD, VDD, GND");
    } else if (micSlot == 1) {
      report("MIC", "Micro INMP441", "WARN", d + " - mic o kenh PHAI: chan L/R chua noi GND (firmware van tu do duoc)");
    } else {
      report("MIC", "Micro INMP441", "PASS", d + " - kenh trai, muc nen binh thuong");
    }
  }
}

static void testSpeaker() {
  if (!i2sOk) { report("SPK", "Loa PCM5100A", "FAIL", "I2S loi"); return; }
  tftStatus("Loa: phat tieng bip...");
  MicStats quiet = playAndRecord(0, 600, 0);
  MicStats beep = playAndRecord(1000, 1200, 0.5f);
  playAndRecord(0, 200, 0);
  if (micSlot < 0) {
    report("SPK", "Loa PCM5100A", "MANUAL", "Vua phat bip 1 kHz 1,2 s - co nghe thay khong? (mic loi nen khong tu kiem tra duoc)");
    return;
  }
  float q = quiet.tone[micSlot], b = beep.tone[micSlot];
  float ratio = b / (q > 0.01f ? q : 0.01f);
  String d = "mic nghe tieng bip 1 kHz: x" + String(ratio, 1) + " so voi luc im";
  if (ratio > 4) report("SPK", "Loa PCM5100A", "PASS", d);
  else report("SPK", "Loa PCM5100A", "WARN",
              d + " - mic KHONG nghe thay. Neu tai cung khong nghe: kiem tra VCC 5V, SD->3V3, MC->GND, BCK16 WS17 DIN15, day loa");
}

// ---------------------------------------------------------------- LED, servo, boot, wifi
static void testLed() {
  const uint8_t c[][3] = {{40, 0, 0}, {0, 40, 0}, {0, 0, 40}, {0, 0, 0}};
  for (auto& x : c) { neopixelWrite(STATUS_LED_PIN, x[0], x[1], x[2]); delay(300); }
  report("LED", "LED RGB GPIO48", "MANUAL", "Vua nhay DO-XANH LA-XANH DUONG");
}

static void servoWrite(int ch, int deg) { ledcWrite(ch, map(deg, 0, 180, 410, 2048)); }
static void testServo() {
  tftStatus("Servo: nhich tung chan...");
  const int pins[4] = {SERVO_PIN_FL, SERVO_PIN_FR, SERVO_PIN_RL, SERVO_PIN_RR};
  const char* names[4] = {"FL", "FR", "RL", "RR"};
  String d;
  for (int i = 0; i < 4; i++) {
    if (SERVO_FR_IS_360 && i == 1) { d += "FR(360) bo qua; "; continue; }
    ledcSetup(i, 50, 14);
    ledcAttachPin(pins[i], i);
    servoWrite(i, 90); delay(400);
    servoWrite(i, 105); delay(400);
    servoWrite(i, 90); delay(400);
    ledcWrite(i, 0);
    ledcDetachPin(pins[i]);
    pinMode(pins[i], OUTPUT); digitalWrite(pins[i], LOW);
    d += String(names[i]) + "(G" + pins[i] + ") ";
  }
  report("SERVO", "4 servo chan", "MANUAL", "Lan luot nhich 15 do: " + d + "- can nguon 5V servo; chan nao khong nhuc nhich = kiem tra day");
}

static void testBoot() {
  pinMode(PIN_BOOT_BTN, INPUT_PULLUP);
  if (digitalRead(PIN_BOOT_BTN) == LOW) {
    report("BOOT", "Nut BOOT GPIO0", "FAIL", "Dang bi giu xuong LOW khi khong nhan (ket nut/chap)");
    return;
  }
  logLine("ACTION|Nhan nut BOOT trong 8 giay...");
  tftStatus("NHAN NUT BOOT (8 s)");
  unsigned long t0 = millis();
  while (millis() - t0 < 8000) {
    if (digitalRead(PIN_BOOT_BTN) == LOW) { report("BOOT", "Nut BOOT GPIO0", "PASS", "Nhan duoc"); return; }
    delay(10);
  }
  report("BOOT", "Nut BOOT GPIO0", "WARN", "Khong ai nhan trong 8 s (muc nghi binh thuong)");
}

static void testWifi() {
  tftStatus("Wi-Fi: dang quet...");
  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks();
  if (n <= 0) { report("WIFI", "Radio Wi-Fi", "FAIL", "Khong quet thay mang nao (anten/radio?)"); return; }
  String d = String(n) + " mang, manh nhat: ";
  for (int i = 0; i < n && i < 3; i++) d += WiFi.SSID(i) + "(" + WiFi.RSSI(i) + "dBm) ";
  report("WIFI", "Radio Wi-Fi", "PASS", d);
  WiFi.mode(WIFI_OFF);
}

// ---------------------------------------------------------------- summary
static void summary() {
  int p = 0, f = 0, w = 0, m = 0;
  for (int i = 0; i < nResults; i++) {
    String s = results[i].status;
    if (s == "PASS") p++; else if (s == "FAIL") f++; else if (s == "WARN") w++; else m++;
  }
  logLine("SUMMARY|pass=" + String(p) + "|fail=" + String(f) + "|warn=" + String(w) + "|manual=" + String(m));
  logLine("TEST_ALL_DONE");
  if (!tftOk) return;
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(6, 4);
  tft.setTextColor(f ? ILI9341_RED : ILI9341_GREEN);
  tft.printf("PASS %d FAIL %d WARN %d", p, f, w);
  for (int i = 0; i < nResults; i++) {
    String s = results[i].status;
    uint16_t col = s == "PASS" ? ILI9341_GREEN : s == "FAIL" ? ILI9341_RED : s == "WARN" ? ILI9341_YELLOW : ILI9341_CYAN;
    tft.setTextColor(col);
    tft.setCursor(6, 30 + i * 19);
    tft.printf("%-5s %s", results[i].id, results[i].status);
  }
}

void setup() {
  Serial.begin(115200);
#ifdef HAS_USB_LOG
  USBSerial.begin(115200);
#endif
  delay(1500);
  logLine("TEST_ALL_START");
  testChip();
  testTft();
  tftStatus("I2C / ToF / IMU...");
  testI2c();
  testTof();
  testImu();
  tftStatus("Micro: giu yen lang...");
  testMic();
  testSpeaker();
  testLed();
  testServo();
  testWifi();
  testBoot();
  summary();
}

void loop() {
  // nhac lai ket qua moi 10 s de script doc duoc ke ca khi mo cong muon
  static unsigned long last = 0;
  if (millis() - last > 10000) {
    last = millis();
    for (int i = 0; i < nResults; i++)
      logLine(String("RESULT|") + results[i].id + "|" + results[i].name + "|" + results[i].status + "|" + results[i].detail);
    summary();
  }
  delay(50);
}
