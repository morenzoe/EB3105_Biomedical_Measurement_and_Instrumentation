// import needed libraries
#include <DallasTemperature.h>
#include <OneWire.h>

// Declaration of the input pin which is connected with the sensor module
#define KY001_Signal_PIN 4

// libraries configuration
OneWire oneWire(KY001_Signal_PIN);
DallasTemperature sensors(&oneWire);

void setup() {
  // heater
  pinMode(7,OUTPUT);
  digitalWrite(7, LOW);    // heater turn on
  
  // serial output initialization
  Serial.begin(9600);
  Serial.println("KY-001 temperature measurement");
  
  // sensor will be initialized
  sensors.begin();
}

//main program loop
void loop()
{
  // temperature measurment will be started...
  sensors.requestTemperatures();
  // ... and measured temperature will be displayed
  Serial.print("Temperature: ");
  Serial.print(sensors.getTempCByIndex(0));
  Serial.write(176); // UniCode of the char-symbol "°-Symbol"
  Serial.println("C");
  
  delay(1000); // 1s break till next measurment
}
