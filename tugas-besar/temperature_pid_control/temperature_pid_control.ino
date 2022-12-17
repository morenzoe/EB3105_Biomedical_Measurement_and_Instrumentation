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

// Solid state relay
#define ssr_pin  11

// Mode inkubator
uint8_t modes = 0; 

// Kontrol PID
float temp_read;            // Pembacaan temperatur inkubator
float temp_set = 36;        // Pengaturan temperatur inkubator
float temp_diff = 1;        // Selisih temperatur untuk memulai mode PID
float temp_error_now;       // Selisih temperatur pembacaan dengan pengaturan sekarang
float temp_error_prev;      // Selisih temperatur pembacaan dengan pengaturan sebelumnya
float time_now;             // Waktu sekarang
float time_prev;            // Waktu sebelumnya
float time_passed;          // Selisih waktu sekarang dengan sebelumnya
float sample_period = 200;  // Perioda pencuplikan PID
float pid_kp = 2176.0;      // Koefisien komponen proporsional PID
float pid_ki = 1.7;         // Koefisien komponen integral PID
float pid_p;                // Nilai komponen proporsional PID
float pid_i;                // Nilai komponen integral PID
int pid_total;              // Nilai PID

// libraries configuration
OneWire oneWire(ky001_pin);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);

  // Solid State Relay
  pinMode(ssr_pin, OUTPUT);  
  digitalWrite(ssr_pin, HIGH);    // When HIGH, the SSR is Off
  TCCR2B = TCCR2B & B11111000 | B00000111;    // D11 PWM is now 30.64 Hz

  // Temperature Sensor
  Serial.println("KY-001 temperature measurement");
  sensors.begin();
  sensors.requestTemperatures();
  temp_read = (sensors.getTempCByIndex(0))*0.8904+1.2337;
  Serial.print(temp_read);
}

void loop() {  
  if(modes == 0){
    ramp_up(); 
  }
  else if (modes == 1){
    PID_control(); 
  }
  else{
    cool_down(); 
  }
}

//Fucntion for ramping up the temperature
//Function buat nyalain SSR/heater kalau suhunya masih kurang
void ramp_up(void){  
  //Rising temperature to (temp_set - temp_diff)
  time_passed = millis() - time_prev; 
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
    //Serial.print("Set: "); 
    //Serial.println(temp_set,2);
    
    //Serial.print((char)247); 
    //Serial.print("C: "); 
    Serial.println(temp_read,1);     
       
    Serial.println();
    
    time_prev = millis();
  }
}//End of ramp_up loop

//Main PID compute and execute function
void PID_control(void){
/*
  if(temp_read < (temp_set - temp_diff)){
      digitalWrite(ssr_pin, LOW);                //Turn On SSR
      modes = 0;
    }
    else
    {  
    */
      time_passed = millis() - time_prev;   
      if(time_passed > sample_period){    
        //1. We get the temperature and calculate the error
        //Baca Nilai
        sensors.requestTemperatures();
        temp_read = (sensors.getTempCByIndex(0))*0.8904+1.2337;
        temp_read = (float)round(temp_read*10)/10;
        
        temp_error_now = temp_set - temp_read;
      
        //2. We calculate PID values
        pid_p = pid_kp * temp_error_now;
        //2.2 Decide if we apply I or not. Only when error is very small
        //if((-0.2 < temp_error_now) && (temp_error_now < 0.2)){
          pid_i = pid_i + (pid_ki * temp_error_now);//}
        //else{pid_i = 0;}
    
        //3. Calculate and map total PID value
        pid_total = floor(pid_p + pid_i);  
        pid_total = constrain(pid_total, 0, 255);
    
//        //4. Set limits for PID values
//        if(PID_map < 0){PID_map = 0;}
//        if(PID_map > 255) {PID_map = 255; } 
    
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
    
        //Serial.print("Set: "); 
        //Serial.println(temp_set,2);
        
        //Serial.print((char)247); 
        //Serial.print("C: "); 
        Serial.println(temp_read,1);  
        
        Serial.println();
        //Serial.display();//Finally display the created image
        
        
        //7. Save values for next loop
        time_prev = millis();                       //Store time for next loop
        temp_error_prev = temp_error_now;             //Store error for next loop
        //Serial.println(time_passed);                //For debug only
        
        //LED_State = !LED_State;
        //digitalWrite(led, LED_State);
      }  
    }
//}//End PID_control loop







//Function for turning off everything and monitoring the coolidn down process
void cool_down(void){
  digitalWrite(ssr_pin, HIGH);    //SSR is OFF with HIGH pulse!
  //digitalWrite(led, LOW);
  time_passed = millis() - time_prev;   
  if(time_passed > sample_period){ 

    Serial.println("Off");     
    Serial.print("Set: "); 
    Serial.println(temp_set,2);
       
    //Serial.display();//Finally display the created image
    time_prev = millis();
  }
}//End cool_down loop
