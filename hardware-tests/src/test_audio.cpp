// TEST LOA + MIC (I2S full-duplex, BCLK 16 / WS 17 dung chung)
//  - Luc khoi dong: phat 1 doan nhac ngan (nghe bang tai).
//  - Lap lai moi 3 s: bip 1 kHz trong 1 s, roi im 2 s.
//  - Moi 0,5 s in muc mic L/R; cuoi moi chu ky in ket qua "mic nghe thay loa" (loopback).
// Loa (WeAct I2S Speaker V1): VCC 5V, GND, SD->3V3, MC->GND, BCK->16, WS->17, DIN->15, loa vao R+/R-, L+/L-
// Mic (INMP441): VDD 3V3, GND, SD->14, SCK->16, WS->17, L/R->GND
#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>

#ifndef I2S_PIN_BCLK
#define I2S_PIN_BCLK 16
#define I2S_PIN_WS 17
#define I2S_PIN_DIN 14
#define I2S_PIN_DOUT 15
#endif

static const int RATE = 16000;
static const int N = 256;          // frames per chunk (16 ms)
static const float VOLUME = 0.35f; // 0..1
static int32_t tx[N * 2], rx[N * 2];
static float phase = 0;

struct Win { double sum[2]; float s1[2], s2[2]; long frames; };

static void winReset(Win& w) { memset(&w, 0, sizeof(w)); }

// Play `freq` (0 = silence) for one chunk and accumulate mic stats into w.
static void step(float freq, Win& w) {
  float inc = 2 * M_PI * (freq > 0 ? freq : 1000) / RATE;
  for (int i = 0; i < N; i++) {
    int32_t v = freq > 0 ? (int32_t)(sinf(phase) * 32767 * VOLUME) << 16 : 0;
    tx[i * 2] = tx[i * 2 + 1] = v;
    phase += inc;
    if (phase > 2 * M_PI) phase -= 2 * M_PI;
  }
  size_t wr = 0, rd = 0;
  i2s_write(I2S_NUM_0, tx, sizeof(tx), &wr, 100);
  i2s_read(I2S_NUM_0, rx, sizeof(rx), &rd, 100);
  float k = 2 * cosf(2 * M_PI * 1000.0f / RATE);
  for (size_t i = 0; i < rd / 8; i++) {
    for (int c = 0; c < 2; c++) {
      float x = rx[i * 2 + c] >> 16;
      w.sum[c] += fabsf(x);
      float s0 = x + k * w.s1[c] - w.s2[c];
      w.s2[c] = w.s1[c];
      w.s1[c] = s0;
    }
  }
  w.frames += rd / 8;
}

static float level(const Win& w, int c) { return w.frames ? w.sum[c] / w.frames : 0; }
static float tone(const Win& w, int c) {
  float k = 2 * cosf(2 * M_PI * 1000.0f / RATE);
  float p = w.s1[c] * w.s1[c] + w.s2[c] * w.s2[c] - k * w.s1[c] * w.s2[c];
  return w.frames ? sqrtf(fmaxf(0, p)) / w.frames : 0;
}

void setup() {
  Serial.begin(115200);
  delay(500);
  i2s_config_t cfg = {};
  cfg.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX);
  cfg.sample_rate = RATE;
  cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
  cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  cfg.dma_buf_count = 6;
  cfg.dma_buf_len = N;
  cfg.tx_desc_auto_clear = true;
  i2s_pin_config_t pins = {};
  pins.mck_io_num = I2S_PIN_NO_CHANGE;
  pins.bck_io_num = I2S_PIN_BCLK;
  pins.ws_io_num = I2S_PIN_WS;
  pins.data_out_num = I2S_PIN_DOUT;
  pins.data_in_num = I2S_PIN_DIN;
  esp_err_t e = i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL);
  if (e == ESP_OK) e = i2s_set_pin(I2S_NUM_0, &pins);
  Serial.printf("\n=== TEST LOA + MIC  BCK=%d WS=%d DOUT(loa)=%d DIN(mic)=%d  %s ===\n", I2S_PIN_BCLK,
                I2S_PIN_WS, I2S_PIN_DOUT, I2S_PIN_DIN, e == ESP_OK ? "I2S OK" : "I2S LOI");
  Serial.println(">> Phat nhac Do-Mi-Sol-Do: NGHE loa co keu khong?");
  const float melody[] = {523.25f, 659.25f, 783.99f, 1046.5f};
  Win w;
  for (float f : melody) {
    for (int i = 0; i < 18; i++) step(f, w);   // ~0,3 s moi not
    for (int i = 0; i < 4; i++) step(0, w);
  }
  Serial.println(">> Tu gio: bip 1 kHz 1 s, im 2 s, lap lai. Noi/vo tay gan mic de xem muc nhay.");
}

void loop() {
  static int cycle = 0;
  Win beepWin, quietWin;   // s1[] is reused here as a sum of per-window 1 kHz energies
  winReset(beepWin);
  winReset(quietWin);
  for (int part = 0; part < 6; part++) {          // 6 x 0.5 s = 3 s: 2 beep + 4 quiet
    bool beep = part < 2;
    Win w; winReset(w);
    for (int i = 0; i < 31; i++) step(beep ? 1000 : 0, w);
    Win& acc = beep ? beepWin : quietWin;
    for (int c = 0; c < 2; c++) { acc.sum[c] += w.sum[c]; }
    acc.frames += w.frames;
    if (beep) { beepWin.s1[0] += tone(w, 0); beepWin.s1[1] += tone(w, 1); }
    else if (part >= 3) { quietWin.s1[0] += tone(w, 0); quietWin.s1[1] += tone(w, 1); }
    Serial.printf("[%s] MIC L %6.0f  R %6.0f\n", beep ? "LOA BIP" : "LOA im ", level(w, 0), level(w, 1));
  }
  // loopback verdict: 1 kHz energy while beeping vs while quiet (per channel, averaged)
  float qL = quietWin.s1[0] / 3, qR = quietWin.s1[1] / 3, bL = beepWin.s1[0] / 2, bR = beepWin.s1[1] / 2;
  float rL = bL / fmaxf(qL, 0.01f), rR = bR / fmaxf(qR, 0.01f);
  float mL = level(quietWin, 0), mR = level(quietWin, 1);
  const char* mic;
  if (mL < 1 && mR < 1) mic = "MIC: KHONG TIN HIEU (VDD? SD->14? SCK/WS dao?)";
  else if (mL > 3 && mR > 3 && fminf(mL, mR) > 0.3f * fmaxf(mL, mR)) mic = "MIC: CA 2 KENH = NHIEU (SD ho/long?)";
  else mic = mL >= mR ? "MIC: OK (kenh trai)" : "MIC: OK (kenh phai, L/R chua noi GND)";
  const char* spk = fmaxf(rL, rR) > 4 ? "LOA: OK - mic nghe thay tieng bip"
                    : "LOA: mic khong nghe thay bip -> NGHE BANG TAI de kiem tra loa";
  Serial.printf("---- chu ky %d | %s | %s (x%.1f) ----\n", ++cycle, mic, spk, fmaxf(rL, rR));
}
