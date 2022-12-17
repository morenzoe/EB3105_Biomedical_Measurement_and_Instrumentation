// Temperature Sensor
#include <DallasTemperature.h>
#include <OneWire.h>
#define KY001_Signal_PIN 4

// Solid State Relay
#define SSR_PIN  11

// Fan PWM
#define Fan_PWM_PIN 9
int fanValue = 255; // range 0 - 255

// State
uint8_t state = 0; 

// Variables
float real_temp;           //We will store here the real temp 
float Setpoint = 36;      //In degrees C
float SetpointDiff = 1;   //In degrees C
float elapsedTime, now_time, prev_time;        //Variables for time control
float refresh_rate = 200;                   //PID loop time in ms
float now_pid_error, prev_pid_error;



///////////////////PID constants///////////////////////
float kp=1280.0;         //Mine was 2.5
float ki=1.0;         //Mine was 0.06
float kd=0.0;         //Mine was 0.8
float PID_p, PID_i, PID_d, PID_total;
int PID_map;
///////////////////////////////////////////////////////

// libraries configuration
OneWire oneWire(KY001_Signal_PIN);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);

  // Solid State Relay
  pinMode(SSR_PIN, OUTPUT);  
  digitalWrite(SSR_PIN, HIGH);    // When HIGH, the SSR is Off
  TCCR2B = TCCR2B & B11111000 | B00000111;    // D11 PWM is now 30.64 Hz

  // Temperature Sensor
  Serial.println("KY-001 temperature measurement");
  sensors.begin();
  sensors.requestTemperatures();
  real_temp = (sensors.getTempCByIndex(0))*0.8904+1.2337;
  Serial.print(real_temp);
  
  // Fan PWM
  analogWrite(Fan_PWM_PIN, fanValue);
}

void loop() {  
  if(state == 0){
    ramp_up(); 
  }
  else if (state == 1){
    PID_control(); 
  }
  else{
    cool_down(); 
  }
}

//Fucntion for ramping up the temperature
//Function buat nyalain SSR/heater kalau suhunya masih kurang
void ramp_up(void){  
  //Rising temperature to (Setpoint - SetpointDiff)
  elapsedTime = millis() - prev_time; 
  if(elapsedTime > refresh_rate){  
    //Baca Nilai
    sensors.requestTemperatures();
    real_temp = (sensors.getTempCByIndex(0))*0.8904+1.2337;
    real_temp = (float)round(real_temp*10)/10;
    
    if(real_temp < (Setpoint - SetpointDiff)){
      digitalWrite(SSR_PIN, LOW);                //Turn On SSR
    }
    else
    {
      digitalWrite(SSR_PIN, HIGH);                 //Turn Off SSR
      state = 1;                                  //Already hot so we go to PID control
    }

    Serial.println("Ramp Up");
    //Serial.print("Set: "); 
    //Serial.println(Setpoint,2);
    
    //Serial.print((char)247); 
    //Serial.print("C: "); 
    Serial.println(real_temp,1);     
       
    Serial.println();
    
    prev_time = millis();
  }
}//End of ramp_up loop

//Main PID compute and execute function
void PID_control(void){
/*
  if(real_temp < (Setpoint - SetpointDiff)){
      digitalWrite(SSR_PIN, LOW);                //Turn On SSR
      state = 0;
    }
    else
    {  
    */
      elapsedTime = millis() - prev_time;   
      if(elapsedTime > refresh_rate){    
        //1. We get the temperature and calculate the error
        //Baca Nilai
        sensors.requestTemperatures();
        real_temp = (sensors.getTempCByIndex(0))*0.8904+1.2337;
        real_temp = (float)round(real_temp*10)/10;
        
        now_pid_error = Setpoint - real_temp;
      
        //2. We calculate PID values
        PID_p = kp * now_pid_error;
        PID_d = kd*((now_pid_error - prev_pid_error)/refresh_rate);
        //2.2 Decide if we apply I or not. Only when error is very small
        //if((-0.2 < now_pid_error) && (now_pid_error < 0.2)){
          PID_i = PID_i + (ki * now_pid_error);//}
        //else{PID_i = 0;}
    
        //3. Calculate and map total PID value
        PID_total = PID_p + PID_i + PID_d;  
        PID_map = int(map(PID_total, 0, 150, 0, 255));
    
        //4. Set limits for PID values
        if(PID_map < 0){PID_map = 0;}
        if(PID_map > 255) {PID_map = 255; } 
    
        //5. Write PWM signal to the SSR
        analogWrite(SSR_PIN, 255-PID_map);
    
        Serial.print("PID map: ");  
        Serial.print(PID_map);
        Serial.print(", p: ");
        Serial.print(PID_p);
        Serial.print(", d: ");
        Serial.print(PID_d);
        Serial.print(", i: ");
        Serial.print(PID_i);
        Serial.print(", total: ");
        Serial.println(PID_total);
    
        //Serial.print("Set: "); 
        //Serial.println(Setpoint,2);
        
        //Serial.print((char)247); 
        //Serial.print("C: "); 
        Serial.println(real_temp,1);  
        
        Serial.println();
        //Serial.display();//Finally display the created image
        
        
        //7. Save values for next loop
        prev_time = millis();                       //Store time for next loop
        prev_pid_error = now_pid_error;             //Store error for next loop
        //Serial.println(elapsedTime);                //For debug only
        
        //LED_State = !LED_State;
        //digitalWrite(led, LED_State);
      }  
    }
//}//End PID_control loop







//Function for turning off everything and monitoring the coolidn down process
void cool_down(void){
  digitalWrite(SSR_PIN, HIGH);    //SSR is OFF with HIGH pulse!
  //digitalWrite(led, LOW);
  elapsedTime = millis() - prev_time;   
  if(elapsedTime > refresh_rate){ 

    Serial.println("Off");     
    Serial.print("Set: "); 
    Serial.println(Setpoint,2);
       
    //Serial.display();//Finally display the created image
    prev_time = millis();
  }
}//End cool_down loop
