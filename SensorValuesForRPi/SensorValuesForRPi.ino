int sensorPin = A0;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  Serial.begin(9600);
}

void loop() {
  map(sensorValue, 0, 1023, 0, 300);

  sensorValue = analogRead(sensorPin);

  if (sensorValue >= 0 && sensorValue < 50) {
    Serial.println(1);
  }

  if (sensorValue > 50 && sensorValue < 100) {
    Serial.println(2);
  }

  if (sensorValue > 100 && sensorValue < 150) {
    Serial.println(3);
  }

  if (sensorValue > 150 && sensorValue < 200) {
    Serial.println(4);
  }

  if (sensorValue > 250 && sensorValue < 300) {
    Serial.println(5);
  }
  delay(500);
}
