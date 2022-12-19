/*EB3105 Pengukuran dan Instrumentasi Biomedika
 *Modul             : Tugas Besar
 *Hari dan Tanggal  : Sabtu, 17 Desember 2022
 *Nama (NIM) 1      : Eraraya Morenzo Muten (18320003)
 *Nama (NIM) 2      : Kayyisa Zahratulfirdaus (18320011)
 *Nama (NIM) 3      : Indira Mona Farhany (18320040)
 *Nama (NIM) 4      : Farrel Jonathan Vickeldo (18320008)
 *Nama File         : Algoritma_On_Off
 *Deskripsi         : 
 *                    
 *                    
 */

// Deklarasi Library
#include <DallasTemperature.h>
#include <OneWire.h>

// Deklarasi Pin Sensor 
#define KY001_Signal_PIN 4

// Deklarasi Pin Solid State Relay
#define SSR_PIN  11

// Deklarasi Pin Kipas PWM
#define Fan_PWM_PIN 9
int fanValue = 255; //Rentang 0 - 255

// Deklarasi Variabel
float real_temp;  //Menyimpan hasil pembacaan temperatur


// Konfigurasi Library
OneWire oneWire(KY001_Signal_PIN);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  pinMode(SSR_PIN, OUTPUT);  
  digitalWrite(SSR_PIN, HIGH);    // Ketika HIGH, SSR akan off
  TCCR2B = TCCR2B & B11111000 | B00000111;    // Pengaturan register untuk mengubah frekuensi PWM SSR_PIN menjadi 30.64 Hz

  Serial.println("Pengukuran Suhu Awal: ");
  sensors.begin();

  // Membaca nilai temperatur
  sensors.requestTemperatures();
  real_temp = (sensors.getTempCByIndex(0))*0.8154 + 3.8662;
  Serial.print(real_temp);
  
  //Menyalakan kipas
  analogWrite(Fan_PWM_PIN, fanValue);
  
}

void loop() {

  // Membaca nilai temperatur
  sensors.requestTemperatures();
  real_temp = (sensors.getTempCByIndex(0))*0.8154 + 3.8662;

  // Kondisi ketika suhu < 36 
  if (real_temp < 36){
    digitalWrite(SSR_PIN, LOW); // SSR on
  }

  // Kondisi ketika suhu >= 36 
  else if (real_temp >= 36){
    digitalWrite(SSR_PIN, HIGH);  //SSR off
  }

  // Mencetak pembacaan suhu
  Serial.print("Suhu saat ini: ");
  Serial.println(real_temp);

  delay(1000);
}
