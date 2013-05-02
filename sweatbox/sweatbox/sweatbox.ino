#include <Wire.h> // Used for I2C

// The SparkFun breakout board defaults to 1, set to 0 if SA0 jumper on the bottom of the board is set
#define MMA8452_ADDRESS 0x1D  // 0x1D if SA0 is high, 0x1C if low

//Define a few of the registers that we will be accessing on the MMA8452
#define OUT_X_MSB 0x01
#define XYZ_DATA_CFG  0x0E
#define WHO_AM_I   0x0D
#define CTRL_REG1  0x2A

#define GSCALE 2 // Sets full-scale range to +/-2, 4, or 8g. Used to calc real g values.

int increment_button = 0;
int decrement_button = 1;

boolean debug = true;

// Global variables are the best variables
boolean step_in_progress;
int step_count;
int step_target;

// the setup routine runs once when you press reset:
void setup() {                
   Serial.begin(57600);
 
  // initialize the heart led control pins as an output.
  for (int i=11; i<22; i++) {
    pinMode(i, OUTPUT);     
  }
  
  //turn off all rows
  for (int i=0; i<11; i++) {
    set_row(i, HIGH);
  }
  
  // talk to the acceleromter
  Wire.begin(); //Join the bus as a master
  initMMA8452(); //Test and intialize the MMA8452  
  
  // init globals
  step_in_progress = false;
  step_count = 0;
  step_target = 40;
  
  pinMode(increment_button, INPUT);
  pinMode(decrement_button, INPUT);
  
  enter_target();
}

// the loop routine runs over and over again forever:
void loop() {
  
   checkForStep();
   
    // calculate percent progress towards goal
   double progress = (double)step_count/(double)step_target;
   int row_count;
   
   //determine how many rows to light up, don't illuminate more rows than actually exist
   if (progress < 1.0)
      row_count = int(11 * progress);
   else
      row_count = 11;
    
    if (debug) {
      Serial.print("step count: ");  
      Serial.println(step_count);
      Serial.print("row count: ");
      Serial.println(row_count);
    }
   
   //clear the display
   for (int i=0; i<11; i++) {
     set_row(i, HIGH);
   }
   
   //display progress  
   for (int i=0; i<row_count; i++) {
     set_row(i, LOW);
   }
 
   //maybe this saves battery life?
   delay(50);  

     
  
  // Code for testing!
  /*
  for (int i=0; i<11; i++) {
    set_row(i, LOW);
  }
  Serial.println("going low");
  
  delay(1500);
  for (int i=0; i<11; i++) {
    set_row(i, HIGH);
    delay(250);
  }
  Serial.println("high");
  delay(1500);
  */ 
}

void enter_target() {
  int frodo = 0; // Frodo lives
  
  while (true) {
   if (digitalRead(increment_button) && digitalRead(decrement_button)) {
     if (frodo==1) {
        if (debug) {
          Serial.println("done programming");
        }
        break;
     } else {
        frodo=1;
     }
   }
   
   if (digitalRead(increment_button)) {
     if (debug) {
       Serial.println("incrementing");
     }
     pinMode(increment_button, OUTPUT);
     digitalWrite(increment_button, LOW);
     
     step_target += 5;
     if (step_target >= 100)
       step_target = 100;
   }
   
   if (digitalRead(decrement_button)) {
     if (debug) {
       Serial.println("deccrementing");
     }
     pinMode(decrement_button, OUTPUT);
     digitalWrite(decrement_button, LOW);
     
     step_target -= 5;
     if (step_target <= 0)
       step_target=0;
   }
   
   if (debug) {
     Serial.print("step target: ");
     Serial.println(step_target);
   }
   
   //turn off all rows
   for (int i=0; i<11; i++) {
     set_row(i, HIGH);
   }
   
   //display goal as percentage
   double percent = double(step_target)/100.0;
   int row_count;
   
   if (percent < 1.0)
      row_count = int(11 * percent);
   else
      row_count = 11;
   
   for (int i=0; i<row_count; i++) {
     set_row(i, LOW);
   }
   
   delay(250);
 }
}

void checkForStep() {
  //return true if this is the user is stepping and this step hasn't been previously reported.
    
  int accelCount[3];  // Stores the 12-bit signed value
  readAccelData(accelCount);  // Read the x/y/z adc values

  // Now we'll calculate the accleration value into actual g's
  float accelG[3];
  for (int i = 0 ; i < 3 ; i++)
  {
    accelG[i] = (float) accelCount[i] / ((1<<12)/(2*GSCALE));  // get actual g value, this depends on scale being set
  }
  
  // determine the magnitude of the acceleration vector
  float mag_acceleration = sqrt(pow(accelG[0], 2) + pow(accelG[1], 2) + pow(accelG[2], 2));

  // determine whether the acceration is a step
  if (mag_acceleration > 1.30 && !step_in_progress) {
    step_in_progress = true;
    step_count++; 
    if (debug) {
      Serial.print("You done stepped!: ");
      Serial.println(mag_acceleration);
    }
  } else if (mag_acceleration < 1.15) {
    step_in_progress = false;
  }
}

void set_row(int rowNumber, uint8_t value) {
  // Set a row HIGH or LOW. Rows are numbered 0-10 starting at bottom of heart.
 digitalWrite(rowNumber+11, value); //+11 to convert arduino pin # to row # 
}

void readAccelData(int *destination)
{
  byte rawData[6];  // x/y/z accel register data stored here

  readRegisters(OUT_X_MSB, 6, rawData);  // Read the six raw data registers into data array

  // Loop to calculate 12-bit ADC and g value for each axis
  for(int i = 0; i < 3 ; i++)
  {
    int gCount = (rawData[i*2] << 8) | rawData[(i*2)+1];  //Combine the two 8 bit registers into one 12-bit number
    gCount >>= 4; //The registers are left align, here we right align the 12-bit integer

    // If the number is negative, we have to make it so manually (no 12-bit data type)
    if (rawData[i*2] > 0x7F)
    {  
      gCount = ~gCount + 1;
      gCount *= -1;  // Transform into negative 2's complement #
    }

    destination[i] = gCount; //Record this gCount into the 3 int array
  }
}

// Initialize the MMA8452 registers 
// See the many application notes for more info on setting all of these registers:
// http://www.freescale.com/webapp/sps/site/prod_summary.jsp?code=MMA8452Q
void initMMA8452()
{
  byte c = readRegister(WHO_AM_I);  // Read WHO_AM_I register
  if (c == 0x2A) // WHO_AM_I should always be 0x2A
  {  
    if (debug) {
      Serial.println("MMA8452Q is online...");
    }
  }
  else
  {
    if (debug) {
      Serial.print("Could not connect to MMA8452Q: 0x");
      Serial.println(c, HEX);
    }
    while(1) ; // Loop forever if communication doesn't happen
  }

  MMA8452Standby();  // Must be in standby to change registers

  // Set up the full scale range to 2, 4, or 8g.
  byte fsr = GSCALE;
  if(fsr > 8) fsr = 8; //Easy error check
  fsr >>= 2; // Neat trick, see page 22. 00 = 2G, 01 = 4A, 10 = 8G
  writeRegister(XYZ_DATA_CFG, fsr);

  //The default data rate is 800Hz and we don't modify it in this example code

  MMA8452Active();  // Set to active to start reading
}

// Sets the MMA8452 to standby mode. It must be in standby to change most register settings
void MMA8452Standby()
{
  byte c = readRegister(CTRL_REG1);
  writeRegister(CTRL_REG1, c & ~(0x01)); //Clear the active bit to go into standby
}

// Sets the MMA8452 to active mode. Needs to be in this mode to output data
void MMA8452Active()
{
  byte c = readRegister(CTRL_REG1);
  writeRegister(CTRL_REG1, c | 0x01); //Set the active bit to begin detection
}

// Read bytesToRead sequentially, starting at addressToRead into the dest byte array
void readRegisters(byte addressToRead, int bytesToRead, byte * dest)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToRead);
  Wire.endTransmission(false); //endTransmission but keep the connection active

  Wire.requestFrom(MMA8452_ADDRESS, bytesToRead); //Ask for bytes, once done, bus is released by default

  while(Wire.available() < bytesToRead); //Hang out until we get the # of bytes we expect

  for(int x = 0 ; x < bytesToRead ; x++)
    dest[x] = Wire.read();    
}

// Read a single byte from addressToRead and return it as a byte
byte readRegister(byte addressToRead)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToRead);
  Wire.endTransmission(false); //endTransmission but keep the connection active

  Wire.requestFrom(MMA8452_ADDRESS, 1); //Ask for 1 byte, once done, bus is released by default

  while(!Wire.available()) ; //Wait for the data to come back
  return Wire.read(); //Return this one byte
}

// Writes a single byte (dataToWrite) into addressToWrite
void writeRegister(byte addressToWrite, byte dataToWrite)
{
  Wire.beginTransmission(MMA8452_ADDRESS);
  Wire.write(addressToWrite);
  Wire.write(dataToWrite);
  Wire.endTransmission(); //Stop transmitting
}
