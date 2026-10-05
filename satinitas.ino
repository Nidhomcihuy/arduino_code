#include <EEPROM.h>

#define TDS_PIN     A0     // Output analog TDS Meter V1
#define BUTTON_PIN  13     // Push button kalibrasi
#define VREF        5.0    // Tegangan referensi Analog 5V
#define VMAX        2.3    // Tegangan AOUT pada 1000 ppm
#define PPM_MAX   1000.0   // Rentang ukur modul (ppm)

const int CALIB_ADDR = 0;      // Alamat EEPROM untuk offset
float calibrationOffset = 0.0;

float bacaTDSMentah() {
  float v = analogRead(TDS_PIN) * VREF / 1023.0;
  return (v / VMAX) * PPM_MAX;   // skala kasar 0-1000 ppm
}

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // aktif-low

  // Baca offset yang tersimpan di EEPROM
  EEPROM.get(CALIB_ADDR, calibrationOffset);

  Serial.println("=== Kalibrasi TDS Meter V1 ===");
  Serial.print("Offset tersimpan : ");
  Serial.print(calibrationOffset, 2);
  Serial.println(" ppm");
  Serial.println("Tekan tombol saat pembacaan TDS stabil,");
  Serial.println("lalu kirim ppm larutan standar via Serial.");
  Serial.println("");
}

void loop() {
  float tdsMentah = bacaTDSMentah();

  // Cek tombol ditekan (aktif-low: LOW saat ditekan)
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50);   // debounce sederhana
    if (digitalRead(BUTTON_PIN) == LOW) {
      // Tunggu input ppm larutan standar dari Serial Monitor
      Serial.println("* Tombol DITEKAN! *");
      Serial.print("TDS sensor sekarang : ");
      Serial.print(tdsMentah, 2);
      Serial.println(" ppm");
      Serial.println("Masukkan ppm larutan standar, lalu Enter:");
      while (!Serial.available()) {
        delay(50);
      }
      float ppmReferensi = Serial.parseFloat();
      while (Serial.available()) Serial.read();  // clear buffer

      // Hitung dan simpan offset ke EEPROM
      calibrationOffset = ppmReferensi - tdsMentah;
      EEPROM.put(CALIB_ADDR, calibrationOffset);
      Serial.println("Selesai kalibrasi!");
      Serial.print("Offset tersimpan : ");
      Serial.print(calibrationOffset, 2);
      Serial.println(" ppm");
      // Tunggu tombol dilepas
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
    }
    delay(200);
  }
  // Terapkan offset ke hasil pembacaan
  float tdsTerkalibrasi = tdsMentah + calibrationOffset;

  Serial.print("TDS mentah      : ");
  Serial.print(tdsMentah, 2);
  Serial.println(" ppm");
  Serial.print("Salinitas akhir : ");
  Serial.print(tdsTerkalibrasi, 2);
  Serial.println(" ppm");
  Serial.println("---");
  delay(1000);
}