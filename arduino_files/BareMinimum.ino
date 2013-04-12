int increment_pin = 6;  // When in setup mode, increments the number of steps the device will count.
int decrement_pin = 7;  // When in setup mode, decrements the number of steps the device will count.
int reset_pin = 8;      // Aborts the session in action, and places the device in setup mode.
int debug;              // Toggles debug mode, which intializes a serial connection to a computer and prints data.

void setup()
{
  debug = 0;  // Set debug state.
  
  // I/O setup.  Disable when uploading final version
  if (debug) {
    Serial.begin(9600);
  }
  
  // Basic pin setup
  pinMode(increment_pin, INPUT);
  pinMode(decrement_pin, INPUT);
  pinMode(reset_pin, INPUT);
  
  // put your setup code here, to run once:
  pinMode(4, OUTPUT);
  pinMode(17, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly: 
  digitalWrite(4, LOW);
  digitalWrite(17, HIGH);

  delay(500);
  digitalWrite(4, HIGH);
  digitalWrite(17, LOW);
}
