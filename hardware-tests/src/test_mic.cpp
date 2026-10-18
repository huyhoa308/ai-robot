// TEST MIC INMP441 - in muc am luong 2 kenh L/R moi 0,5 s.
// Noi / vo tay gan mic: kenh dung phai nhay len, kenh kia ~0.
// Dau day: VDD->3V3, GND->GND, SD->14, SCK->16, WS->17, L/R->GND
#include <Arduino.h>
#include <driver/i2s.h>

#ifndef I2S_PIN_BCLK
#define I2S_PIN_BCLK 16
#define I2S_PIN_WS 17
#define I2S_PIN_DIN 14
#define I2S_PIN_DOUT 15
#endif

static const int RATE = 16000;

void setup() {
  Serial.begin(115200);
  delay(500);
  i2s_config_t cfg = {};
  cfg.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
  cfg.sample_rate = RATE;
  cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
  cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  cfg.dma_buf_count = 4;
  cfg.dma_buf_len = 256;
  i2s_pin_config_t pins = {};
  pins.mck_io_num = I2S_PIN_NO_CHANGE;
  pins.bck_io_num = I2S_PIN_BCLK;
  pins.ws_io_num = I2S_PIN_WS;
  pins.data_out_num = I2S_PIN_NO_CHANGE;
  pins.data_in_num = I2S_PIN_DIN;
  esp_err_t e = i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL);
  if (e == ESP_OK) e = i2s_set_pin(I2S_NUM_0, &pins);
  Serial.printf("\n=== TEST MIC INMP441 (SD=%d SCK=%d WS=%d) %s ===\n", I2S_PIN_DIN, I2S_PIN_BCLK,
                I2S_PIN_WS, e == ESP_OK ? "I2S OK" : "I2S LOI");
  Serial.println("Noi hoac vo tay gan mic. Binh thuong: yen lang ~10-300, noi to > 1000, kenh kia ~0.");
}

static void bar(char* out, float v) {
  int n = (int)constrain(log10f(v + 1) * 6, 0, 30);  // thang log
  for (int i = 0; i < 30; i++) out[i] = i < n ? '#' : '.';
  out[30] = 0;
}

void loop() {
  static int32_t buf[512];
  double sum[2] = {0, 0};
  int32_t peak[2] = {0, 0}, first[2] = {0, 0};
  bool stuck[2] = {true, true};
  long frames = 0;
  unsigned long t0 = millis();
  while (millis() - t0 < 500) {
    size_t r = 0;
    i2s_read(I2S_NUM_0, buf, sizeof(buf), &r, 100);
    for (size_t i = 0; i < r / 8; i++) {
      for (int c = 0; c < 2; c++) {
        int32_t raw = buf[i * 2 + c];
        if (frames == 0 && i == 0) first[c] = raw;
        else if (raw != first[c]) stuck[c] = false;
        int32_t v = raw >> 16;
        sum[c] += abs(v);
        if (abs(v) > peak[c]) peak[c] = abs(v);
      }
    }
    frames += r / 8;
  }
  float L = frames ? sum[0] / frames : 0, R = frames ? sum[1] / frames : 0;
  char bl[31], br[31];
  bar(bl, L);
  bar(br, R);
  const char* verdict;
  if (!frames) verdict = "KHONG DOC DUOC I2S";
  else if (L < 1 && R < 1) verdict = "KHONG TIN HIEU (VDD? SD? SCK/WS dao?)";
  else if ((stuck[0] && first[0]) || (stuck[1] && first[1])) verdict = "DU LIEU DUNG YEN (SD chap VCC?)";
  else if (L > 3 && R > 3 && fminf(L, R) > 0.3f * fmaxf(L, R)) verdict = "CA 2 KENH = NHIEU (SD ho/long?)";
  else if (fmaxf(L, R) > 3000) verdict = "MUC QUA CAO (nhieu?)";
  else verdict = L >= R ? "OK - kenh TRAI" : "OK - kenh PHAI (L/R chua noi GND)";
  Serial.printf("MIC L %6.0f [%s] R %6.0f [%s] peak %5ld/%5ld  %s\n", L, bl, R, br, (long)peak[0],
                (long)peak[1], verdict);
}
