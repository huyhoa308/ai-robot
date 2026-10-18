// =========================================================================================
// TEST SERVO CO BAN - 1 servo tren GPIO14
//  1) Kiem tra servo con song khong (co quay / co luc giu khong)
//  2) Phan biet servo 180 do (positional) va servo 360 do (continuous rotation)
//
// Servo khong co tin hieu phan hoi ve ESP32, nen ket qua phai QUAN SAT BANG MAT.
// Serial (USB native, COM11) se in ra tung buoc va "neu thay X thi la Y".
//
// Lenh Serial (115200, gui kem Enter):
//   r          : chay lai toan bo bai test (mac dinh tu lap lai)
//   x          : dung, tha xung (servo tha long)
//   500..2500  : giu co dinh 1 do rong xung (us), vd: 1500
// =========================================================================================
#include <Arduino.h>
#include "soc/gpio_periph.h"
#include "soc/io_mux_reg.h"

#ifndef SERVO_TEST_PIN
#define SERVO_TEST_PIN 14
#endif

static const int LEDC_CH = 0;
static const int LEDC_FREQ = 50;     // 50Hz -> chu ky 20ms
static const int LEDC_BITS = 14;     // 16384 buoc / 20ms ~ 1.22us moi buoc

static bool attached = false;
static bool autoLoop = true;
static String rxLine;

static void servoWriteUs(int us) {
  if (!attached) {
    ledcSetup(LEDC_CH, LEDC_FREQ, LEDC_BITS);
    ledcAttachPin(SERVO_TEST_PIN, LEDC_CH);
    // Bat them input de doc nguoc xung thuc te tren chan (tu kiem tra)
    PIN_INPUT_ENABLE(GPIO_PIN_MUX_REG[SERVO_TEST_PIN]);
    attached = true;
  }
  us = constrain(us, 500, 2500);
  ledcWrite(LEDC_CH, (uint32_t)us * ((1 << LEDC_BITS) - 1) / 20000);
}

static void servoRelease() {
  if (attached) {
    ledcWrite(LEDC_CH, 0);
    ledcDetachPin(SERVO_TEST_PIN);
    pinMode(SERVO_TEST_PIN, INPUT);
    attached = false;
  }
}

// Doc lenh Serial. Tra ve true neu co lenh lam gian doan bai test.
static bool pollSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c != '\n' && c != '\r') { rxLine += c; continue; }
    rxLine.trim();
    if (rxLine.length() == 0) continue;
    String cmd = rxLine;
    rxLine = "";

    if (cmd == "r") {
      autoLoop = true;
      Serial.println("\n>> Chay lai bai test.");
      return true;
    }
    if (cmd == "x") {
      autoLoop = false;
      servoRelease();
      Serial.println("\n>> Da dung, tha xung. Gui 'r' de chay lai.");
      return true;
    }
    int us = cmd.toInt();
    if (us >= 500 && us <= 2500) {
      autoLoop = false;
      servoWriteUs(us);
      Serial.printf("\n>> Giu co dinh %d us. Gui 'r' de chay lai, 'x' de dung.\n", us);
      return true;
    }
    Serial.printf(">> Lenh khong hop le: '%s'\n", cmd.c_str());
  }
  return false;
}

// delay co kiem tra lenh. Tra ve true neu bi gian doan.
static bool waitMs(uint32_t ms) {
  uint32_t t0 = millis();
  while (millis() - t0 < ms) {
    if (pollSerial()) return true;
    delay(10);
  }
  return false;
}

// Giu 1 xung trong ms, in mo ta
static bool hold(int us, uint32_t ms, const char *what) {
  Serial.printf("   [%4d us] %s\n", us, what);
  servoWriteUs(us);
  return waitMs(ms);
}

static bool sweep(int fromUs, int toUs, int stepUs, uint32_t stepMs) {
  int dir = (toUs > fromUs) ? 1 : -1;
  for (int us = fromUs; dir * (toUs - us) >= 0; us += dir * stepUs) {
    servoWriteUs(us);
    if (waitMs(stepMs)) return true;
  }
  return false;
}

static void printHeader() {
  Serial.println();
  Serial.println("=====================================================");
  Serial.printf (" TEST SERVO CO BAN - chan tin hieu GPIO%d\n", SERVO_TEST_PIN);
  Serial.println(" Lenh: r=chay lai | x=dung | 500..2500=giu xung (us)");
  Serial.println("=====================================================");
}

// Do do rong xung thuc te tren chan tin hieu
static void signalCheck() {
  Serial.printf("\n[BUOC 0] TU KIEM TRA XUNG TREN GPIO%d\n", SERVO_TEST_PIN);
  const int targets[] = {1000, 1500, 2000};
  bool ok = true;
  for (int t : targets) {
    servoWriteUs(t);
    delay(60);
    unsigned long hi = pulseIn(SERVO_TEST_PIN, HIGH, 50000);
    unsigned long lo = pulseIn(SERVO_TEST_PIN, LOW, 50000);
    bool good = hi > 0 && abs((long)hi - t) < 30 && lo > 15000;
    ok &= good;
    Serial.printf("   dat %4d us -> do duoc HIGH=%5lu us, LOW=%5lu us, chu ky=%5lu us  %s\n",
                  t, hi, lo, hi + lo, good ? "OK" : "SAI");
  }
  if (ok) {
    Serial.println("   => ESP32 DANG PHAT XUNG DUNG tren chan. Neu servo khong chay -> loi NGUON / GND / DAY.");
  } else {
    Serial.println("   => Xung SAI/MAT: chan bi chap xuong GND/5V, hoac noi nham chan.");
  }
}

// Tra ve true neu bi gian doan
static bool runTest() {
  printHeader();
  signalCheck();

  // ---------------- BUOC 1: CON SONG KHONG ----------------
  Serial.println("\n[BUOC 1] KIEM TRA CON SONG");
  Serial.println("   Quan sat: servo phai co tieng 'rit' nhe / co luc giu / chuyen dong.");
  if (hold(1500, 1500, "xung giua (1500us)")) return true;
  Serial.println("   Quet cham 1000us -> 2000us -> 1000us ...");
  if (sweep(1000, 2000, 10, 20)) return true;
  if (sweep(2000, 1000, 10, 20)) return true;
  if (hold(1500, 1000, "ve giua")) return true;
  Serial.println("   => Neu servo HOAN TOAN dung yen, khong tieng, khong luc giu khi lay tay xoay:");
  Serial.println("      kiem tra nguon 5V/GND (GND phai noi chung voi ESP32), day tin hieu GPIO14,");
  Serial.println("      neu van khong phan ung -> servo da hong.");

  // ---------------- BUOC 2: 180 HAY 360 ----------------
  Serial.println("\n[BUOC 2] PHAN BIET SERVO 180 / 360");
  if (hold(1500, 3000, "giua  -> 180: dung o 90 do | 360: DUNG (hoac bo cham neu lech trung diem)")) return true;
  if (hold(1000, 3000, "min   -> 180: quay toi ~0 do roi DUNG han | 360: QUAY LIEN TUC chieu 1")) return true;
  if (hold(1500, 2000, "giua  -> 180: quay ve 90 do | 360: dung")) return true;
  if (hold(2000, 3000, "max   -> 180: quay toi ~180 do roi DUNG han | 360: QUAY LIEN TUC chieu 2")) return true;
  if (hold(1500, 2000, "giua")) return true;

  Serial.println("\n[KET LUAN - doi chieu voi quan sat]");
  Serial.println("   * Moi lan doi xung, truc quay mot doan roi DUNG LAI va giu cung -> SERVO 180 do");
  Serial.println("   * Truc quay MAI KHONG DUNG khi 1000/2000us, dung khi 1500us  -> SERVO 360 do");
  Serial.println("   * 360 ma o 1500us van bo cham -> trung diem lech, chinh vit/bien tro calib");
  Serial.println("     hoac gui so (vd 1480, 1520) de tim xung lam no dung han.");

  servoRelease();
  Serial.println("\n>> Xong 1 vong. Tu lap lai sau 5s (gui 'x' de dung).");
  return waitMs(5000);
}

void setup() {
  Serial.begin(115200);
  // USB CDC: cho toi da 3s de mo Serial Monitor, khong bat buoc
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 3000) delay(10);
  delay(500);
  pinMode(SERVO_TEST_PIN, INPUT);
}

void loop() {
  if (autoLoop) {
    runTest();
  } else {
    pollSerial();
    delay(10);
  }
}
