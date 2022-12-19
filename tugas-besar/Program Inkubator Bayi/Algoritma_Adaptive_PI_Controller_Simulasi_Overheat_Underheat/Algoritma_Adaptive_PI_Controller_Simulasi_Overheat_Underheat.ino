/*EB3105 Pengukuran dan Instrumentasi Biomedika
 *Modul             : Tugas Besar
 *Hari dan Tanggal  : Sabtu, 17 Desember 2022
 *Nama (NIM) 1      : Eraraya Morenzo Muten (18320003)
 *Nama (NIM) 2      : Kayyisa Zahratulfirdaus (18320011)
 *Nama (NIM) 3      : Indira Mona Farhany (18320040)
 *Nama (NIM) 4      : Farrel Jonathan Vickeldo (18320008)
 *Nama File         : Algoritma_Adaptive_PI_Controller_Simulasi_Overheat_Underheat
 *Deskripsi         : 
 *                    
 *                    
 */

// Deklarasi Variabel dan Library
// Sensor temperatur
#include <DallasTemperature.h>
#include <OneWire.h>
#define ky001_pin 4
OneWire oneWire(ky001_pin);
DallasTemperature sensors(&oneWire);

// Solid state relay
#define ssr_pin  11

// Fan PWM
#define fan_pin 9

// Mode inkubator
uint8_t modes = 0; 

// Kontrol PID
float temp_read;            // Pembacaan temperatur inkubator (drjtC)
float temp_set = 36;        // Pengaturan temperatur inkubator (drjtC)
float temp_diff = 1;        // Selisih temperatur untuk memulai mode PID (drjtC)
float temp_error;           // Selisih temperatur pembacaan dengan pengaturan (drjtC)
float temp_start;           // Pembacaan temperatur pertama (drjtC)
float time_now = 0;         // Waktu sekarang (ms)
float time_prev = 0;        // Waktu sebelumnya (ms)
float time_passed;          // Selisih waktu sekarang dengan sebelumnya (ms)
float sample_period = 1010; // Perioda pencuplikan PID (ms)
float pid_kp = 2560.0;      // Koefisien komponen proporsional PID
float pid_ki = 1.0;         // Koefisien komponen integral PID
float pid_p;                // Nilai komponen proporsional PID
float pid_i = 0;            // Nilai komponen integral PID
int pid_total;              // Nilai PID

void setup() {
  // Mengatur pin kipas
  pinMode(fan_pin, OUTPUT);  
  digitalWrite(fan_pin, LOW);
  
  // Memulai komunikasi serial
  Serial.begin(9600);

  // Mengatur pin solid state relay sebagai PWM
  pinMode(ssr_pin, OUTPUT);  
  digitalWrite(ssr_pin, HIGH);
  TCCR2B = TCCR2B & B11111000 | B00000111;  // Pengaturan register untuk mengubah frekuensi PWM SSR_PIN menjadi 30.64 Hz
  
  // Memulai pembacaan temperatur
  sensors.begin();
  // Menyimpan temperatur awal
  sensors.requestTemperatures();
  temp_start = (sensors.getTempCByIndex(0))*0.8904+1.2337;
  temp_start = (float)round(temp_start*10)/10;
}

void loop() {
  if(Serial.available()) {
      byte command=Serial.read();
      if(command==110) {      // 'n'
        // Memasuki mode rise
        modes = 1;
        // Menyalakan Kipas
        digitalWrite(fan_pin, HIGH);
        // Menyimpan temperatur awal
        sensors.requestTemperatures();
        temp_start = (sensors.getTempCByIndex(0))*0.8904+1.2337;
        temp_start = (float)round(temp_start*10)/10;        
        Serial.println("on");
      } else if (command==102) { // 'f'
        // Memasuki mode shutdown
        modes = 4;
        Serial.println("off");
      }
  }
  
  // Mode off
  if(modes == 0){
    digitalWrite(ssr_pin, HIGH);                 // SSR off
    digitalWrite(fan_pin, LOW);                 // Kipas off
  } else if (modes == 1){
    mode_rise(); 
  } else if (modes == 2){
    mode_pid(); 
  } else if (modes == 3){
    mode_fall(); 
  } else if (modes == 4){
    mode_shutdown(); 
  } else if (modes == 5){
    mode_pid_testing(); 
  }
}

// Fungsi mode rise
void mode_rise(void){  
  //Rising temperature to (temp_set - temp_diff)
  time_now = millis();
  time_passed = time_now - time_prev; 
  if(time_passed > sample_period){  
    //Baca Nilai
    sensors.requestTemperatures();
    temp_read = (sensors.getTempCByIndex(0))*0.8904+1.2337;
    temp_read = (float)round(temp_read*10)/10;
    
    if(temp_read < (temp_set - temp_diff)){
      digitalWrite(ssr_pin, LOW);                //SSR on
    }
    else
    {
      digitalWrite(ssr_pin, HIGH);                 //SSR off
      modes = 2;                                  //Memulai mode PID untuk koreksi dengan PI
    }

    Serial.println("Rise");
    Serial.println(temp_read,1);     
    Serial.println();
    
    time_prev = time_now;
  }
} // Akhir dari mode rise

// Fungsi mode pid
void mode_pid(void){
  time_now = millis();
  time_passed = time_now - time_prev;   
  if(time_passed > sample_period){    
 
    //Membaca nilai suhu dan menentukan nilai error
    sensors.requestTemperatures();
    temp_read = (sensors.getTempCByIndex(0))*0.8904+1.2337;
    temp_read = (float)round(temp_read*10)/10;
    
    temp_error = temp_set - temp_read;
  
    // Menghitung faktor koreksi PID
    pid_p = pid_kp * temp_error;
    pid_i = pid_i + (pid_ki * temp_error);

    // Menghitung dan melakukan mapping pada nilai total PID
    pid_total = floor(pid_p + pid_i);  
    pid_total = constrain(pid_total, 0, 255);

    // Menghidupkan SSR dengan sinyal PWM
    analogWrite(ssr_pin, 255-pid_total);

    Serial.println(temp_read,1);  
    
    
    time_prev = time_now;                       // Menyimpan nilai waktu untuk loop selanjutnya

    // Kembali ke mode rise jika turun terlalu jauh
    if (temp_read <= (temp_set - (temp_diff+0.5))){
      modes = 1;
      // Reset nilai komponen integral PID
      pid_i = 0;
    } else if (temp_read >= (temp_set + (temp_diff+0.5))){
      modes = 3;
      // Reset nilai komponen integral PID
      pid_i = 0;
    }
  }  
}

//Fungsi mode fall
void mode_fall(void){
  time_now = millis();
  time_passed = time_now - time_prev; 
  if(time_passed > sample_period){  
    //Membaca nilai suhu
    sensors.requestTemperatures();
    temp_read = (sensors.getTempCByIndex(0))*0.8904+1.2337;
    temp_read = (float)round(temp_read*10)/10;
    
    if(temp_read > (temp_set + temp_diff)){
      digitalWrite(ssr_pin, HIGH);                //SSR off
    }
    else
    {
      digitalWrite(ssr_pin, LOW);                 //SSR on
      modes = 2;                                  //Memulai mode PID untuk koreksi dengan PI
    }

//    Serial.println("Fall");
    Serial.println(temp_read,1);     
//    Serial.println();
    
    time_prev = time_now;
  }
} // Akhir dari mode fall

// Fungsi mode shutdown
void mode_shutdown(void){
  time_now = millis();
  time_passed = time_now - time_prev; 
  if(time_passed > sample_period){  
    //Membaca nilai suhu
    sensors.requestTemperatures();
    temp_read = (sensors.getTempCByIndex(0))*0.8904+1.2337;
    temp_read = (float)round(temp_read*10)/10;
    
    if(temp_read > (temp_start)){
      digitalWrite(ssr_pin, HIGH);                // SSR off
    }
    else
    {
      digitalWrite(ssr_pin, HIGH);                 // SSR off
      digitalWrite(fan_pin, LOW);                 // Kipas off
      modes = 0;                                  // Sudah dingin sehingga dimatikan
    }

//    Serial.println("Shutdown");
    Serial.println(temp_read,1);     
//    Serial.println();
    
    time_prev = time_now;
  }
} // Akhir dari mode shutdown

// Fungsi mode pid testing (untuk simulasi penanganan kasus overheat dan underheat)
void mode_pid_testing(void){
  time_now = millis();
  time_passed = time_now - time_prev;   
  if(time_passed > sample_period){    
    
    // Membaca nilai suhu
    sensors.requestTemperatures();
    temp_read = (sensors.getTempCByIndex(0))*0.8904+1.2337;
    temp_read = (float)round(temp_read*10)/10;
    
    temp_error = temp_set - temp_read;
  
    // Menghitung faktor koreksi PID
    pid_p = pid_kp * temp_error;
    pid_i = pid_i + (pid_ki * temp_error);

    // Menghitung dan melakukan mapping pada nilai total PID
    pid_total = floor(pid_p + pid_i);  
    pid_total = constrain(pid_total, 0, 255);

    // Memanaskan inkubator untuk simulasi overheat
    digitalWrite(ssr_pin, LOW); // Turn on SSR

    Serial.print("SIM PWM: ");  
    Serial.print(255-pid_total);
    Serial.print(", p: ");
    Serial.print(pid_p);
    Serial.print(", i: ");
    Serial.print(pid_i);
    Serial.print(", total: ");
    Serial.println(pid_total);

    Serial.println(temp_read,1);  
    Serial.println();
    
    time_prev = time_now;                       // Menyimpan nilai waktu untuk loop selanjutnya

    // Kembali ke mode rise jika turun terlalu jauh
    if (temp_read <= (temp_set - (temp_diff+0.5))){
      modes = 1;
      // Reset nilai komponen integral PID
      pid_i = 0;
    } else if (temp_read >= (temp_set + (temp_diff+0.5))){
      modes = 3;
      // Reset nilai komponen integral PID
      pid_i = 0;
    }
  }  
} // Akhir mode pid testing
