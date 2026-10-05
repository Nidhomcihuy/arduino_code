// Percobaan sensor ultrasonik HC-SR04 dengan Arduino Uno
// Benda dekat -> LED merah, benda jauh -> LED biru (+ kalibrasi)
//
// Pengkabelan:
//   HC-SR04 VCC  -> Arduino 5V
//   HC-SR04 Trig -> Arduino D12
//   HC-SR04 Echo -> Arduino D11
//   HC-SR04 GND  -> Arduino GND
//   LED merah (anode, lewat resistor 220 ohm) -> D10, katode -> GND
//   LED biru  (anode, lewat resistor 220 ohm) -> D9,  katode -> GND
//
// Perintah via Serial Monitor (set ke "Newline", baud 9600):
//   k<jarak>  kalibrasi offset, contoh: k20 (benda dipasang tepat 20 cm)
//   t<suhu>   set suhu ruangan dalam derajat C, contoh: t30
//   r         reset kalibrasi ke nilai awal
//   ?         tampilkan bantuan

const int trigPin = 12;
const int echoPin = 11;
const int ledMerah = 10;
const int ledBiru = 9;

const float batasJarak = 20.0;          // di bawah ini = dekat (merah), di atas = jauh (biru)
const float jarakMaks = 400.0;          // batas jangkauan sensor (cm)
const unsigned long timeoutUs = 30000;  // batas waktu menunggu pantulan (30 ms)

// Parameter kalibrasi
const float suhuAwal = 27.0;            // suhu ruangan awal (derajat C)
float suhu = suhuAwal;
float offsetCm = 0.0;                   // koreksi hasil ukur (cm)

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledMerah, OUTPUT);
  pinMode(ledBiru, OUTPUT);
  Serial.println("Sensor ultrasonik siap digunakan");
  tampilBantuan();
}

// Atur LED: 'm' = merah, 'b' = biru, 'x' = keduanya mati
void setLed(char warna) {
  digitalWrite(ledMerah, warna == 'm' ? HIGH : LOW);
  digitalWrite(ledBiru, warna == 'b' ? HIGH : LOW);
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
  cekPerintah();

  float mentah = bacaJarakRata(5);

  if (mentah < 0 || mentah > jarakMaks) {
    Serial.println("Di luar jangkauan");
    setLed('x');  // kedua LED mati: tidak ada pembacaan valid
  } else {
    float jarak = mentah + offsetCm;
    if (jarak < 0) {
      jarak = 0;
    }

    Serial.print("Jarak: ");
    Serial.print(jarak, 1);

    if (jarak < batasJarak) {
      Serial.println(" cm -> DEKAT (merah)");
      setLed('m');
    } else {
      Serial.println(" cm -> JAUH (biru)");
      setLed('b');
    }
  }

  delay(200);
}
ultrasonic jarak benda dengan 2 led