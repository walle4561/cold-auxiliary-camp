void setup() {
  Serial.begin(9600);

  for (int i = 0; i <= 4; i++) {
    for (int j = 0; j < i; j++) {
      Serial.print("*");
    }
    Serial.println();
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
