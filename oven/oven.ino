#include <max6675.h> // ADAFRUIT MAX6675 LIBRARY

#define THERMO_DO 5
#define THERMO_CS 4
#define THERMO_CLK 3

#define RELAY_PIN 2

#define LOG_INTERVAL 250

#define RELAY_OFF_STATE HIGH
#define RELAY_ON_STATE LOW

#define MIN_TEMP 0.00
#define MAX_TEMP 250.00
#define SLOPE_PEAK_RISE   1.20
#define SLOPE_PEAK_FALL   0.4
#define TEMP_OVERSHOOT    33.75
#define TEMP_UNDERSHOOT   0.75
#define STABILIZE_TEMP    2.00

MAX6675 thermocouple(THERMO_CLK, THERMO_CS, THERMO_DO);
bool heatOn = false;
float targetTemp = 130;

#define WINDOW_SIZE 4
#define LOG_WINDOW_COUNT 4
#define LOG_COUNT LOG_WINDOW_COUNT * WINDOW_SIZE

boolean initializedTempLogs = false;
float tempLogs[LOG_COUNT];

void logTemp(float temp){
  if(initializedTempLogs == true){
    for(int i = 0; i < LOG_COUNT - 1; i++) tempLogs[i] = tempLogs[i + 1];
    tempLogs[LOG_COUNT - 1] = temp;
  } else{
    for(int i = 0; i < LOG_COUNT; i++) tempLogs[i] = temp;
    initializedTempLogs = true;
  }
}

void applyTempLogWindow(float (& tempLogWindow)[LOG_WINDOW_COUNT]){
  for(int i = 0; i < LOG_WINDOW_COUNT; i++){
    float sum = 0.0;
    for(int j = 0; j < WINDOW_SIZE; j++){
      int tempLogIndex = (i * WINDOW_SIZE) + j;
      sum += tempLogs[tempLogIndex];
    }
    float average = sum / WINDOW_SIZE;
    tempLogWindow[i] = average;
  }
}

float getSlope(){
  float tempLogWindow[LOG_WINDOW_COUNT];
  applyTempLogWindow(tempLogWindow);
  float slopeSum = 0.0;
  for(int i = 0; i < LOG_WINDOW_COUNT - 1; i++){
    float diff = tempLogWindow[i + 1] - tempLogWindow[i];
    slopeSum += diff;
  }
  return slopeSum / ((float) LOG_WINDOW_COUNT - 1);
}

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  Serial.println("HELLO!");

  delay(1000);
}

void loop() {

  float currentTemp = thermocouple.readCelsius();
  float currentTempF = thermocouple.readFahrenheit();
  
  logTemp(currentTemp);

  float slope = getSlope();
  float reference = slope >= 0 ?
    (slope / SLOPE_PEAK_RISE) * TEMP_OVERSHOOT :
    (slope / SLOPE_PEAK_FALL) * (STABILIZE_TEMP - TEMP_UNDERSHOOT) + STABILIZE_TEMP;

  heatOn = currentTemp + reference < targetTemp;

  digitalWrite(RELAY_PIN, heatOn ? RELAY_ON_STATE : RELAY_OFF_STATE);

  Serial.print("TC = ");
  Serial.print(targetTemp);
  Serial.print("\t");

  Serial.print("C = ");
  Serial.print(currentTemp);
  Serial.print("\t");

  Serial.print("S = ");
  Serial.print(slope);
  Serial.print("\t");

  Serial.print("R = ");
  Serial.print(reference);
  Serial.print("\t");

  Serial.print("O = ");
  Serial.print(heatOn);
  Serial.print("\t");

  Serial.print("F = ");
  Serial.print(currentTempF);
  Serial.print("\n");

  delay(500);
}