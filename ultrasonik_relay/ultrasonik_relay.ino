// Sensor ultrasonik HC-SR04 + relay (dekat -> LED merah, jauh -> LED biru) + kalibrasi
//
// Pengkabelan:
//   HC-SR04 VCC  -> 5V,  Trig -> D12,  Echo -> D11,  GND -> GND
//   Relay   VCC  -> 5V,  IN   -> D8,   GND  -> GND
//   Relay COM -> 5V
//   Relay NO  -> resistor 220 ohm -> LED merah -> GND
//   Relay NC  -> resistor 220 ohm -> LED biru  -> GND
//   Push button: satu kaki -> D2, kaki lain -> GND (pakai INPUT_PULLUP)
//
// Kalibrasi:
//   Push button : kalibrasi offset pada jarak referensi (20 cm)
//
// Perintah via Serial Monitor (set ke "Newline", baud 9600):
//   k<jarak>  kalibrasi offset, contoh: k20 (benda dipasang tepat 20 cm)
//   t<suhu>   set suhu ruangan dalam derajat C, contoh: t30
//   r         reset kalibrasi ke nilai awal
//   ?         tampilkan bantuan

const int trigPin = 12;
const int echoPin = 11;
const int relayPin = 8;
const int tombolPin = 13;                // push button kalibrasi (ke GND)

// Banyak modul relay aktif saat IN = LOW. Jika relay bekerja terbalik,
// ubah nilai ini (true <-> false).
const bool relayActiveLow = false;

const float batasJarak = 20.0;          // di bawah ini = dekat (relay aktif)
const float histeresis = 2.0;           // mencegah relay "berdetak" di sekitar batas (cm)
const float jarakMaks = 400.0;          // batas jangkauan sensor (cm)
const unsigned long timeoutUs = 30000;  // batas waktu menunggu pantulan (30 ms)

// Parameter kalibrasi
const float suhuAwal = 27.0;            // suhu ruangan awal (derajat C)
const float jarakKalibrasi = 20.0;      // jarak referensi saat tombol ditekan (cm)
float suhu = suhuAwal;
float offsetCm = 0.0;                   // koreksi hasil ukur (cm)

bool dekat = false;                     // status terakhir: benda dekat atau tidak

// Variabel push button (diubah di dalam interrupt)
volatile bool tombolDitekan = false;
volatile unsigned long waktuTekanTerakhir = 0;

void isrTombol() {
  unsigned long sekarang = millis();
  if (sekarang - waktuTekanTerakhir > 250) {  // debounce 250 ms
    tombolDitekan = true;
    waktuTekanTerakhir = sekarang;
  }
}

void setRelay(bool aktif) {
  bool level = relayActiveLow ? !aktif : aktif;
  digitalWrite(relayPin, level ? HIGH : LOW);
}

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(tombolPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(tombolPin), isrTombol, FALLING);
  setRelay(false);
  Serial.println("Sensor ultrasonik siap digunakan");
  tampilBantuan();
}

// Kecepatan suara (cm/mikrodetik) bergantung pada suhu udara
float kecepatanSuara() {
  return (331.3 + 0.606 * suhu) / 10000.0;
}

// Satu kali pembacaan jarak tanpa koreksi offset
float bacaJarakMentah() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long durasi = pulseIn(echoPin, HIGH, timeoutUs);

  if (durasi == 0) {
    return -1;  // tidak ada pantulan
  }

  return durasi * kecepatanSuara() / 2;
}

// Rata-rata beberapa pembacaan agar hasil lebih stabil
float bacaJarakRata(int jumlah) {
  float total = 0;
  int valid = 0;

  for (int i = 0; i < jumlah; i++) {
    float d = bacaJarakMentah();
    if (d >= 0) {
      total += d;
      valid++;
    }
    delay(60);  // jeda antar pengukuran agar gema tidak saling tumpang
  }

  if (valid == 0) {
    return -1;
  }
  return total / valid;
}

void tampilBantuan() {
  Serial.println("--- Perintah kalibrasi ---");
  Serial.println("Tombol   : kalibrasi di 20 cm");
  Serial.println("k<jarak> : kalibrasi offset, contoh k20");
  Serial.println("t<suhu>  : set suhu (C), contoh t30");
  Serial.println("r        : reset kalibrasi");
  Serial.println("?        : bantuan");
  Serial.print("Suhu: ");
  Serial.print(suhu, 1);
  Serial.print(" C, offset: ");
  Serial.print(offsetCm, 2);
  Serial.println(" cm");
}

void kalibrasiOffset(float jarakAsli) {
  Serial.print("Kalibrasi pada jarak ");
  Serial.print(jarakAsli, 1);
  Serial.println(" cm, jangan gerakkan benda...");

  float rata = bacaJarakRata(20);

  if (rata < 0) {
    Serial.println("Gagal: tidak ada pantulan terdeteksi");
    return;
  }

  offsetCm = jarakAsli - rata;
  Serial.print("Rata-rata terbaca: ");
  Serial.print(rata, 2);
  Serial.println(" cm");
  Serial.print("Offset baru: ");
  Serial.print(offsetCm, 2);
  Serial.println(" cm");
}

void cekPerintah() {
  if (!Serial.available()) {
    return;
  }

  String perintah = Serial.readStringUntil('\n');
  perintah.trim();
  if (perintah.length() == 0) {
    return;
  }

  char kode = perintah.charAt(0);
  float nilai = perintah.substring(1).toFloat();

  if (kode == 'k' && nilai > 0) {
    kalibrasiOffset(nilai);
  } else if (kode == 't') {
    suhu = nilai;
    Serial.print("Suhu diatur ke ");
    Serial.print(suhu, 1);
    Serial.println(" C");
  } else if (kode == 'r') {
    suhu = suhuAwal;
    offsetCm = 0.0;
    Serial.println("Kalibrasi direset");
  } else if (kode == '?') {
    tampilBantuan();
  } else {
    Serial.println("Perintah tidak dikenal, ketik ? untuk bantuan");
  }
}

void loop() {
  // Kalibrasi lewat push button
  if (tombolDitekan) {
    tombolDitekan = false;
    Serial.println("Tombol ditekan -> kalibrasi");
    kalibrasiOffset(jarakKalibrasi);
    tombolDitekan = false;  // abaikan pantulan kontak selama kalibrasi
  }

  cekPerintah();

  float mentah = bacaJarakRata(5);

  if (mentah < 0 || mentah > jarakMaks) {
    Serial.println("Di luar jangkauan -> dianggap jauh (LED biru)");
    dekat = false;
  } else {
    float jarak = mentah + offsetCm;
    if (jarak < 0) {
      jarak = 0;
    }

    // Histeresis: relay aktif di bawah batas, baru mati setelah melewati batas + histeresis
    if (jarak < batasJarak) {
      dekat = true;
    } else if (jarak > batasJarak + histeresis) {
      dekat = false;
    }

    Serial.print("Jarak: ");
    Serial.print(jarak, 1);
    if (dekat) {
      Serial.println(" cm -> DEKAT (relay aktif, LED merah)");
    } else {
      Serial.println(" cm -> JAUH (relay mati, LED biru)");
    }
  }

  setRelay(dekat);
  delay(200);
}