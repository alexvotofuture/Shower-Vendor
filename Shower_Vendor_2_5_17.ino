//Flow Sensor Global Variables
#define FLOWSENSORPIN 2

#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
Adafruit_7segment matrix = Adafruit_7segment();
const int solenoid = 12;

// count how many pulses!
volatile uint16_t pulses = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
volatile float flowrate;
// Interrupt is called once a millisecond, looks for any pulses from the sensor!
SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(FLOWSENSORPIN);
  
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return; // nothing changed!
  }
  
  if (x == HIGH) {
    //low to high transition!
    pulses++;
  }
  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;  // in hertz
  lastflowratetimer = 0;
}

void useInterrupt(boolean v) {
  if (v) {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}
//RFID Variables
int RFIDResetPin = 11;

//Register your RFID tags here
char tag1[13] = "690025C10489";//690025C10489ß
char tag2[13] = "67007B95B33A";
char tag3[13] = "67007B75355C";
char tag4[13] = "01023101093A";
char tag5[13] = "01023C0A4376";

void setup(){
  Serial.begin(9600);

  pinMode(RFIDResetPin, OUTPUT);
  digitalWrite(RFIDResetPin, HIGH);

  matrix.begin(0x70);
  matrix.setBrightness(1); // 0 is darkest, 15 is brightest
   
  pinMode(FLOWSENSORPIN, INPUT);
  digitalWrite(FLOWSENSORPIN, HIGH);
  lastflowpinstate = digitalRead(FLOWSENSORPIN);
  useInterrupt(true);
   
  pinMode (solenoid, OUTPUT);
}

void loop(){
  char tagString[13];
  int index = 0;
  boolean reading = false;

  while(Serial.available()){

    int readByte = Serial.read(); //read next available byte

    if(readByte == 2) reading = true; //begining of tag
    if(readByte == 3) reading = false; //end of tag

    if(reading && readByte != 2 && readByte != 10 && readByte != 13){
      //store the tag
      tagString[index] = readByte;
      index ++;
    }
  }

  checkTag(tagString); //Check if it is a match
  clearTag(tagString); //Clear the char of all value
  resetReader(); //reset the RFID reader
}

void checkTag(char tag[]){
///////////////////////////////////
//Check the read tag against known tags
///////////////////////////////////

  if(strlen(tag) == 0) return; //empty, no need to contunue

  if(compareTag(tag, tag1)){ // if matched tag1, do this
    Serial.print("Welcome, Alex"); Serial.println(tag);//read out any unknown tag
    shower(5); //call the shower function, passing on 65 liters as a variable

  }else if(compareTag(tag, tag2)){ //if matched tag2, do this
    Serial.print("Welcome, Tenant #2"); Serial.println(tag);//read out any unknown tag
    shower(5); //call the shower function, passing on 65 liters as a variable

  }else{
    Serial.print("ACCESS DENIED. Unknown Card #: "); Serial.println(tag);//read out any unknown tag
  }

}

void shower(int duration){
  while(true){//keep looping through this until "duration" liters worth of water have been used
    digitalWrite(solenoid, HIGH);
    // if a plastic sensor use the following calculation
    // Sensor Frequency (Hz) = 7.5 * Q (Liters/min)
    // Liters = Q * time elapsed (seconds) / 60 (seconds/minute)
    // Liters = (Frequency (Pulses/second) / 7.5) * time elapsed (seconds) / 60
    // Liters = Pulses / (7.5 * 60)
    float liters = pulses;
    liters /= 7.5;
    liters /= 60.0;
  
    Serial.print(liters); Serial.println(" Liters");
  //  int disp = (int) liters;
  //  matrix.print(disp, DEC); matrix.writeDisplay();
  
  if (liters > duration){
  digitalWrite(solenoid, LOW);
  break;
  }
 } 
}

void resetReader(){
///////////////////////////////////
//Reset the RFID reader to read again.
///////////////////////////////////
  digitalWrite(RFIDResetPin, LOW);
  digitalWrite(RFIDResetPin, HIGH);
  delay(500);
}

void clearTag(char one[]){
///////////////////////////////////
//clear the char array by filling with null - ASCII 0
//Will think same tag has been read otherwise
///////////////////////////////////
  for(int i = 0; i < strlen(one); i++){
    one[i] = 0;
  }
}

boolean compareTag(char one[], char two[]){
///////////////////////////////////
//compare two value to see if same,
//strcmp not working 100% so we do this
///////////////////////////////////

  if(strlen(one) == 0) return false; //empty

  for(int i = 0; i < 12; i++){
    if(one[i] != two[i]) return false;
  }

  return true; //no mismatches
}
