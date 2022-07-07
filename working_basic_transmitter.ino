//Final Basic Tramiter
#include "Arduino.h"

//Library to use serial port with other arbitary pins.
#include<SoftwareSerial.h>

#define RS485rx             7 //RS485 Receive Pin
#define RS485tx             5 //RS485 Transmit Pin
#define RS485inout          6 //RS485 Transmit of Receive controller

#define RS485Transmit       HIGH
#define RS485Receive        LOW
#define ledPin              13
#define baudRate            57600     //baudrate for rs485 transmission.
SoftwareSerial RS485(RS485rx, RS485tx); 

//definging pins for input values from joysticks.(not taking digital input on/off);
#define Joystick_1_xvalue  A0
#define Joystick_1_yvalue  A1
#define Joystick_2_xvalue  A2
#define Joystick_2_yvalue  A3

#define toggle_state     A5
#define toggle_led       11 

int Joystick_values[4];   //saving values taken from the Joystick.
int command_byte[8];      //storing the command bits in here.
                          
int state_bit;            //high state bit means the submarine will perform rotatory motion along vertical axis.,  
                          //low state bit means the submarine will perform linear motion.
float command_integer;    //integer is not working here(maybe because we are using pow() function in findint out the command_integer and pow() doesnt work properly unless output is used as float or double)
char transmitting_array[7];

byte txvalue;
byte rxvalue;

//deadzone rane for joystick input.
#define lowertolerance  412
#define highertolerance  712

void ReadInputs(); 
void set_commandbits();
void print_command_byte();
void create_decimalvalue();
void update_transmitting_array();
void print_transmitting_array();
void initiate_communication();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);      //initializing the serial communication to the pc not to the other arduino.!!!!!!!

  //Set modes for pins
  pinMode(ledPin, OUTPUT);
  pinMode(RS485inout, OUTPUT);
  pinMode(toggle_led, OUTPUT);

 
  pinMode(Joystick_1_xvalue, INPUT);
  pinMode(Joystick_1_yvalue, INPUT);
  pinMode(Joystick_2_xvalue, INPUT);
  pinMode(Joystick_2_yvalue, INPUT);
  pinMode(toggle_state, INPUT);


  
  digitalWrite(RS485inout, RS485Receive);
  
  RS485.begin(baudRate);     //serial baudrate for rs485 communication, communication to another arduino.
 }


void loop() {
  // put your main code here, to run repeatedly:
command_integer = 0;
ReadInputs();
set_commandbits();
create_decimalvalue();
update_transmitting_array();
initiate_communication();
Serial.println(" ");
//delay(3000);
}

void ReadInputs(){
  Joystick_values[0] = analogRead(Joystick_1_xvalue);
  Joystick_values[1] = analogRead(Joystick_1_yvalue);
  Joystick_values[2] = analogRead(Joystick_2_xvalue);
  Joystick_values[3] = analogRead(Joystick_2_yvalue);
  state_bit = analogRead(toggle_state);
  }

void set_commandbits(){
  if(state_bit > 512){
    state_bit = 1;
    digitalWrite(toggle_led, HIGH);
  }
  else{
    state_bit = 0;
    digitalWrite(toggle_led, LOW);
  }
  for(int k = 0; k<4; k++){
  if(Joystick_values[k] <= lowertolerance || Joystick_values[k] > highertolerance){
    command_byte[k*2] = 1;
    command_byte[(k*2)+1] = Joystick_values[k] < 512 ? 0 : 1;
  }
  else{
    command_byte[k*2] = 0;
    command_byte[(k*2)+1] = 0;
  }
}
print_command_byte(); //for debugging purposes, to check whether input values are read correctly or not.
}

void print_command_byte(){
  Serial.print(state_bit);
  Serial.print(" ");
  for(int i = 0; i<8; i++){
    Serial.print(command_byte[i]);
  }
  Serial.print("  ");
}


//this function takes the command_byte binary values and creates a decimal value out of it saves that decimal value in variable command_integer.
void create_decimalvalue(){
  for(int i = 0; i < 8; i++){

    double a = pow(2, i);           //need to take this double or else pow() function will not work properly(for some compiler). better to take double.
    
    command_integer = ((command_byte[7-i]) * a) + command_integer; 
    /*Serial.print(command_integer);
    Serial.print(" ");*/
    }
   /*Serial.print("    ");
  Serial.print(command_integer);
  Serial.print("    ");
  */
}

//this function updates the array which is being sent.
void update_transmitting_array(){
  int command_integer_1 = (int)(command_integer + 0.5);     //creating a another variable to convert the command_integer, which is float variable into integer type to convert in character with out errors.
  transmitting_array[0] = '<';
  transmitting_array[1] = state_bit + '0';
  transmitting_array[2] = ',';
  for(int n = 0; n < 3; n++){
    transmitting_array[5 - n] = ((int)(command_integer_1)% 10) + '0' ;
    command_integer_1  = command_integer_1/10;
  }
  transmitting_array[6] = '>';
  //print_transmitting_array();   //for debugging purposes will print the arrray which is to be transmitted.
}

void print_transmitting_array(){
  for(int q = 0; q < 7; q++){
    Serial.print(transmitting_array[q]);  
  }
  delay(4);    //approximately  adding delay to let the serial.print get completed  
}

void initiate_communication(){
    
    for(int z = 0; z < 7; z++){
      txvalue = transmitting_array[z];
      Serial.print(char(txvalue));
      digitalWrite(RS485inout, RS485Transmit); 
      RS485.write(txvalue);
      delay(4);
      digitalWrite(RS485inout, RS485Receive);
      while(!RS485.available()){Serial.println("test waiting.");
      delay(1);}
      if(RS485.available()){
        Serial.print("In the read loop:");
        rxvalue = RS485.read();
        Serial.print("    got back:"); Serial.print(char(rxvalue));
      }
   }
}
