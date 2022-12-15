// Temperature Sensor
#include <DallasTemperature.h>
#include <OneWire.h>
#define KY001_Signal_PIN 4

// Solid State Relay
#define SSR_PIN  11

// Fan PWM
#define Fan_PWM_PIN 9
int fanValue = 255; // range 0 - 255

float real_temp; 


// libraries configuration
OneWire oneWire(KY001_Signal_PIN);
DallasTemperature sensors(&oneWire);

void setup() {
  //Serial.begin(250000);     //For debug
  Serial.begin(9600);
  pinMode(SSR_PIN, OUTPUT);  
  digitalWrite(SSR_PIN, HIGH);    // When HIGH, the SSR is Off

  TCCR2B = TCCR2B & B11111000 | B00000111;    // D11 PWM is now 30.64 Hz

  Serial.println("KY-001 temperature measurement");
  sensors.begin();

  //Baca Nilai
  sensors.requestTemperatures();
  real_temp = (sensors.getTempCByIndex(0))*0.8154 + 3.8662;
  Serial.print(real_temp);
  
  //Nyalain Kipas
  analogWrite(Fan_PWM_PIN, fanValue);
  
}

void loop() {
  sensors.requestTemperatures();
  real_temp = (sensors.getTempCByIndex(0))*0.8154 + 3.8662;
    
  if (real_temp < 36){
    digitalWrite(SSR_PIN, LOW);
  }

  else if (real_temp >= 36){
    digitalWrite(SSR_PIN, HIGH);
  }

  Serial.print("Suhu saat ini: ");
  Serial.println(real_temp);

  delay(1000);
}

  
