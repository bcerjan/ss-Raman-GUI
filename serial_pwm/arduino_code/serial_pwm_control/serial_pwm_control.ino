// based on example 5 here:
// https://forum.arduino.cc/index.php?topic=396450.0

// Code to control a PWM output over a serial connection
// Expects input of the form <1.0> where 1.0 is a float that
// ranges from 0.0 - 100.0 representing duty cycle %, and 
// uses '<' as a start character and '>' as an end character

// Variables for serial communication:
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];

float incomingFloat = 0.0;

boolean newData = false;

// Variables for PWM control:
int pwmPin = 9; // pin that goes HI for PWM

//============

void setup() {
  // Turn on serial communication
  Serial.begin(9600);
  Serial.println();
  
  // Set PWM Pin as output
  pinMode(pwmPin, OUTPUT);
}

//============

void loop() {
  recvWithStartEndMarkers();
  
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    parseData();
    showParsedData();
    updatePWM();
    newData = false;
  }
}

//============

void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;
  
  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();
    
    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // string terminator
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }
    
    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

//===========

void parseData() {
  //char *strtokIndx;
  incomingFloat = atof(tempChars);
  
}

//===========

void showParsedData() {
  Serial.print("Float Transmitted: ");
  Serial.println(incomingFloat);
}

//===========

void updatePWM() {
  // First chop our float to 0-100 in case of bad input
  if (incomingFloat < 0.0) {
    incomingFloat = 0.0;
  }
  
  if (incomingFloat > 100.0) {
    incomingFloat = 100.0;
  }
  
  // Now convert to 0-255 (integer) for analogWrite()
  int dutyCycle = round(255.0 * incomingFloat / 100.0);
  Serial.print("Duty Cycle: ");
  Serial.println(dutyCycle);
  
  // Finally, set our PWM:
  analogWrite(pwmPin, dutyCycle);
}
