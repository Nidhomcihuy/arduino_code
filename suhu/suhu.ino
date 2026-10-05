#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 13

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  sensors.begin();

  Serial.println("Sensor DS18B20");
  Serial.println("-------------------------");
}

void loop() {
  sensors.requestTemperatures();

  float suhu = sensors.getTempCByIndex(0);

  if (suhu == DEVICE_DISCONNECTED_C) {
    Serial.println("DS18B20 TIDAK TERDETEKSI!");
  }
  else {
    Serial.print("Suhu Air: ");
    Serial.print(suhu);
    Serial.println(" °C");

    if (suhu > 30) {
      Serial.println("SUHU TINGGI");
    }
    else if (suhu <= 26) {
      Serial.println("SUHU RENDAH");
    }
    else {
      Serial.println("SUHU NORMAL");
    }
  }

  Serial.println("-------------------------");
  delay(1000);
}