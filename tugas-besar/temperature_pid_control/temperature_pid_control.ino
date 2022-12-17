/*EB3105 Pengukuran dan Instrumentasi Biomedika
 *Modul             : Tugas Besar
 *Hari dan Tanggal  : Sabtu, 17 Desember 2022
 *Nama (NIM) 1      : Eraraya Morenzo Muten (18320003)
 *Nama (NIM) 2      : Kayyisa Zahratulfirdaus (18320011)
 *Nama (NIM) 3      : Indira Mona Farhany (18320040)
 *Nama (NIM) 4      : Farrel Jonathan Vickeldo (18320008)
 *Nama File         : temperature_pid_control
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

// Mode inkubator
uint8_t modes = 2; 

// Kontrol PID
float temp_read;            // Pembacaan temperatur inkubator (drjtC)
float temp_set = 36;        // Pengaturan temperatur inkubator (drjtC)
float temp_diff = 1;        // Selisih temperatur untuk memulai mode PID (drjtC)
float temp_error_now;       // Selisih temperatur pembacaan dengan pengaturan sekarang (drjtC)
float temp_error_prev;      // Selisih temperatur pembacaan dengan pengaturan sebelumnya (drjtC)
float time_now = 0;         // Waktu sekarang (ms)
float time_prev = 0;        // Waktu sebelumnya (ms)
float time_passed;          // Selisih waktu sekarang dengan sebelumnya (ms)
float sample_period = 1010; // Perioda pencuplikan PID (ms)
float pid_kp = 2560.0;      // Koefisien komponen proporsional PID
float pid_ki = 1.0;         // Koefisien komponen integral PID
float pid_p;                // Nilai komponen proporsional PID
float pid_i;                // Nilai komponen integral PID
int pid_total;              // Nilai PID

void setup() {
  // Memulai komunikasi serial
  Serial.begin(9600);

  // Mengatur pin solid state relay sebagai PWM
  pinMode(ssr_pin, OUTPUT);  
  digitalWrite(ssr_pin, HIGH);
  TCCR2B = TCCR2B & B11111000 | B00000111;  // pin D11 PWM 30.64 Hz

  // Memulai pembacaan temperatur
  sensors.begin();
}

void loop() {
  if(Serial.available()) {
      byte command=Serial.read();
      if(command==110) {      // 'n'
        modes = 0;
        //Serial.println("on");
      } else if (command==102) { // 'f'
        modes = 2;
        //Serial.println("off");
      }
  }
  
  if(modes == 0){
    mode_rise(); 
  }
  else if (modes == 1){
    mode_pid(); 
  }
  else if (modes == 2){
    mode_fall(); 
  }
}

//Fucntion for ramping up the temperature
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
      digitalWrite(ssr_pin, LOW);                //Turn On SSR
    }
    else
    {
      digitalWrite(ssr_pin, HIGH);                 //Turn Off SSR
      modes = 1;                                  //Already hot so we go to PID control
    }

    Serial.println("Ramp Up");
    Serial.println(temp_read,1);     
    Serial.println();
    
    time_prev = time_now;
  }
}//End of ramp_up loop

//Main PID compute and execute function
void mode_pid(void){
  time_now = millis();
  time_passed = time_now - time_prev;   
  if(time_passed > sample_period){    
    //1. We get the temperature and calculate the error
    //Baca Nilai
    sensors.requestTemperatures();
    temp_read = (sensors.getTempCByIndex(0))*0.8904+1.2337;
    temp_read = (float)round(temp_read*10)/10;
    
    temp_error_now = temp_set - temp_read;
  
    //2. We calculate PID values
    pid_p = pid_kp * temp_error_now;
    pid_i = pid_i + (pid_ki * temp_error_now);

    //3. Calculate and map total PID value
    pid_total = floor(pid_p + pid_i);  
    pid_total = constrain(pid_total, 0, 255);

    //5. Write PWM signal to the SSR
    analogWrite(ssr_pin, 255-pid_total);

    Serial.print("PID PWM: ");  
    Serial.print(255-pid_total);
    Serial.print(", p: ");
    Serial.print(pid_p);
    Serial.print(", i: ");
    Serial.print(pid_i);
    Serial.print(", total: ");
    Serial.println(pid_total);

    Serial.println(temp_read,1);  
    Serial.println();
    
    //7. Save values for next loop
    time_prev = time_now;                       //Store time for next loop
    temp_error_prev = temp_error_now;             //Store error for next loop
  }  
}

//Function for turning off everything and monitoring the coolidn down process
void mode_fall(void){
  digitalWrite(ssr_pin, HIGH);    //SSR is OFF with HIGH pulse!
  //digitalWrite(led, LOW);
  time_now = millis();
  time_passed = time_now - time_prev;   
  if(time_passed > sample_period){ 

    //Baca Nilai
    sensors.requestTemperatures();
    temp_read = (sensors.getTempCByIndex(0))*0.8904+1.2337; 
    temp_read = (float)round(temp_read*10)/10;
    
    Serial.println("Off");     
    Serial.println(temp_read,1);
    Serial.println();
    
    time_prev = time_now;
  }
}//End cool_down loop
