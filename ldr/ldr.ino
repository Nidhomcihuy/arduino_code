// ================= KONFIGURASI PIN =================
#define RELAY_PIN   12   // Pin ke relay
#define LDR_AO_PIN  A0   // Output analog LDR (0-1023)

// Relay menggunakan kontak Normally Closed (NC):
// - Relay TIDAK aktif -> kontak NC tertutup -> lampu MENYALA
// - Relay AKTIF        -> kontak NC terbuka  -> lampu MATI
// Sesuaikan HIGH/LOW berikut dengan modul relay yang dipakai (kebanyakan modul relay aktif LOW)
#define RELAY_ON   LOW    // mengaktifkan relay   -> lampu MATI
#define RELAY_OFF  HIGH   // menonaktifkan relay  -> lampu MENYALA

// ================= VARIABEL =================
int ldrValue = 0;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LDR_AO_PIN, INPUT);

  Serial.begin(9600);

  digitalWrite(RELAY_PIN, RELAY_OFF); // kondisi awal: lampu menyala
}

void loop() {
  ldrValue = analogRead(LDR_AO_PIN);

  Serial.print("Nilai LDR (AO): ");
  Serial.println(ldrValue);

  if (ldrValue < 200) {
    // Terang
    Serial.println("Kondisi: TERANG -> Nyala 9s, Mati 3s");
    nyalakanLampu(9000);
    matikanLampu(3000);
  } 
  else if (ldrValue >= 200 && ldrValue <= 500) {
    // Redup
    Serial.println("Kondisi: REDUP -> Nyala 7s, Mati 1s");
    nyalakanLampu(7000);
    matikanLampu(1000);
  } 
  else {
    // Gelap
    Serial.println("Kondisi: GELAP -> Nyala 3s, Mati 3s");
    nyalakanLampu(3000);
    matikanLampu(3000);
  }
}

// ================= FUNGSI RELAY =================
void nyalakanLampu(unsigned long durasi) {
  digitalWrite(RELAY_PIN, RELAY_OFF); // kontak NC tertutup -> lampu ON
  delay(durasi);
}

void matikanLampu(unsigned long durasi) {
  digitalWrite(RELAY_PIN, RELAY_ON);  // kontak NC terbuka -> lampu OFF
  delay(durasi);
}
