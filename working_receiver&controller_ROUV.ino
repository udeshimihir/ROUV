//fullwork receiver with constant down thrust
//Full work nofeedback receiver
#include"Arduino.h"

//Library to allow a Serial Port on arbitary pins
#include<SoftwareSerial.h>

//Pins for  RS485 communication
#define RS485rx       4     //RS485 recieve Pin.
#define RS485tx       7     //RS485 Transmit Pin.
#define RS485inout    8  //Transmit or Recive Status.

#define RS485Transmit HIGH
#define RS485Recive   LOW
#define LEDPin        13
#define baudRate      57600

//Defining RS485 object
SoftwareSerial RS485(RS485rx, RS485tx);

#include<Servo.h> // to control the bldc motor by giving pwm pulse included in servo library.

//creating Servo object for each motor 
Servo FBM_a;                //FBM -> Forward-Backward motor 1.
Servo FBM_b;                //FBM -> Forward-Backward motor 2.
Servo UDM_a;                //UDM -> UP-Down motor 1.
Servo UDM_b;                //UDM -> UP-Down motor 2.
Servo LRM_a;                //LRM -> Left-Right motor 1.
Servo LRM_b;                //LRM -> Left-Right motor 2.

 //The data bytes that we are sending or receiving 
byte rxValue;
byte txValue;

int FBM = 0;
int LRM = 0;
char rx_array[7];
int buffer_delay;
int command_byte[8];
int prev_command_byte[8];
int command_decimal;
int decimal[3];
int state_bit;
int prev_state_bit;
int m = 0;
int thrust_variable;
int thrust = 540;
int set_constant_thrust_interrupt;
int constant_thrust_switch_interrupt;
int interrupt_3;
int Time_a_1;
int Time_a_2;
int Time_b_1;
int Time_b_2;
int Time_c_1;
int Time_c_2;

//motor signalling pins.
#define FBM_1             3
#define FBM_2             5
#define UDM_1             6
#define UDM_2             9
#define LRM_1             10
#define LRM_2             11

//void convert_to_integer();
void decimal_to_binary();
void allot_commands();
void print_recievedarray();
void print_commandbyte();
//void convert_to_integer_2();
void convert_to_integer_3();
int check();
int same();

int forward_motionthrust = map(580, 0, 1023, 0, 180);         //will give out 137 approximately
int reverse_motionthrust = map(450, 0, 1023, 0, 180);
int left_motionthrust    = map(450, 0, 1023, 0, 180);
int right_motionthrust   = map(580, 0, 1023, 0, 180);
int down_motionthrust    = map(390, 0, 1023, 0, 180);       //have limited the down motion to 450 can reduce it upto 335.
int up_motionthrust      = map(580, 0, 1023, 0, 180);
int no_thrust            = map(512, 0, 1023, 0, 180);
int constant_thrust      = map(thrust, 0, 1023, 0, 180);

  
void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);   //for Serial Monitor.

  //Setmodes for Pin.
    pinMode(LEDPin, OUTPUT);       
    pinMode(RS485inout, OUTPUT);

  //switching on the onboard LED.
    digitalWrite(LEDPin, HIGH);

  //attaching respective pins to the motor object.
    FBM_a.attach(FBM_1);
    FBM_b.attach(FBM_2);
    UDM_a.attach(UDM_1);
    UDM_b.attach(UDM_2);
    LRM_a.attach(LRM_1);
    LRM_b.attach(LRM_2);

  //calibrating motors.
    FBM_a.write(no_thrust);          //creating an object for each motor to control esc singals.
    FBM_b.write(no_thrust);
    UDM_a.write(no_thrust);
    UDM_b.write(no_thrust);
    LRM_a.write(no_thrust);
    LRM_b.write(no_thrust);
    delay(2000);

    set_constant_thrust_interrupt = 0;
    constant_thrust_switch_interrupt = 0;
    thrust_variable = 512;

  //Setting RS485 device to read initially
    digitalWrite(RS485inout, RS485Recive);
    buffer_delay = (baudRate / 10)*2 +100;

  //Set the baud rate. The longer the wire lower the baud rate.
  //Set the transmission rate. From the following:
  //300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 31250, 38400, 57600 and 115200
  //the transmissionrate should be same as rate of sender while receiving, and should be same as of reciver while sending.
    RS485.begin(baudRate);
}

void loop() {
  // put your main code here, to run repeatedly:

  //continously send the above data to the reciever with some pause in the end.
  if(RS485.available()){
    do{
      rxValue = RS485.read();
      delayMicroseconds(4500);
    }while((char(rxValue)) != '<');
    rx_array[0] = char(rxValue);
    //readding data character by character which is sent by the transmitter.
    for(int i= 0; i<6; i++){
   
    rxValue = RS485.read(); 
    rx_array[i + 1] = char(rxValue);
    /*Serial.print("Recived:");                                               //printing the current status of data transmission
    Serial.println(char(rxValue));*/
    delayMicroseconds(5000);
    /*
    digitalWrite(LEDPin, HIGH);                                             //to let things settle down before transmission.
    delay(2);
    digitalWrite(RS485inout, RS485Transmit);                                //setting the controller to transmitter mode.
    
    RS485.write(rxValue);                                                       //current data entering into the transmission state.
                                  
    //Wait for the transmission to complete.
    //This delay is dependant on the baud rate set on the arduino. To calculate.
    //delay = (baudrate/10)*2
    delayMicroseconds(buffer_delay);
    digitalWrite(LEDPin, LOW);  
    digitalWrite(RS485inout, RS485Recive);     //getting back into reciver mode.
    */}
    if(rx_array[0] =='<' && rx_array[2] == ',' && rx_array[6] == '>' && check() == 1){
      //convert_to_integer();
      //convert_to_integer_2();
      convert_to_integer_3();
      Serial.print("  ");
      Serial.print(state_bit);
      Serial.print("  ");
      Serial.print(command_decimal);
      Serial.print("    ,");
      decimal_to_binary();
      print_commandbyte();
      
     
      allot_commands();
      Serial.print(constant_thrust_switch_interrupt);
      Serial.print("     /");
      Serial.print(set_constant_thrust_interrupt);
      Serial.print("     /");
      Serial.print(thrust);
      Serial.print("     /");
    }
    print_recievedarray();
  }
    
}

void print_recievedarray(){
  for(int b = 0; b < 7; b++){
    Serial.print(rx_array[b]);
  }
  Serial.println(" ");
}

void print_commandbyte(){
  for(int e = 0; e < 8; e++){
    Serial.print(command_byte[e]);
  }
  Serial.print("   :");
}

/*void convert_to_integer(){
  command_decimal = 0;
  state_bit = rx_array[1] - '0';   //getting state bit out of the character array.
  for(int s = 0; s<3; s++){       //converting the character number into integer number which is our command_decimal.
    float b, a = command_decimal;
    Serial.print((rx_array[5-s] - '0')* pow(10,s));
    Serial.print(" ");
    Serial.print(a);
    Serial.print(" ");
    b = (((rx_array[5-s] - '0'))* pow(10,s))+ a;
    command_decimal = (int)(b);
    Serial.print(command_decimal);
    Serial.print("| ");
  }
}*/

/*void convert_to_integer_2(){
  state_bit = rx_array[1] - '0';
  for(int q = 0; q<3; q++){
    decimal[q] = (rx_array[5-q] - '0')* pow(10, q);
  }
  command_decimal = decimal[0] + decimal[1] + decimal[2];
}*/

/*The check function checks whether the byte sent by the controller is correct/ whether the command received makes sense or not..
for example the highest value that can be sent by the controller is 255  this means that no number greater than 2 will be on the hundreds place.
also no character will be sent in place of the numbers, if no number is to be sent then zero will be sent...
considering all those things, this check funtion is created to check the state bit, then will check the first number character to check values.
then will check 2nd character for values other than no., then will check 3rd character for values other than no.
if no error or noise value is found then the function will return 1 or else it will return 0.*/
int check(){
  if(rx_array[1] == '0' || rx_array[1] == '1'){
    if(rx_array[3] == '0' || rx_array[3] == '2' || rx_array[3] == '1'){
      if(rx_array[4] == '0' || rx_array[4] == '2' || rx_array[4] == '1' ||rx_array[4] == '3' ||rx_array[4] == '4' ||rx_array[4] == '5' ||rx_array[4] == '6' ||rx_array[4] == '7' ||rx_array[4] == '8' ||rx_array[4] == '9'){
        if(rx_array[5] == '0' || rx_array[5] == '1' || rx_array[5] == '2' || rx_array[5] == '3' || rx_array[5] == '4' || rx_array[5] == '5' || rx_array[5] == '6' || rx_array[5] == '7' || rx_array[5] == '8' || rx_array[5] == '9'){
          return 1;
        }
        else{return 0;}
      }
      else{return 0;}
    }
    else{return 0;}
  }
  else{ return 0;}
}

int same(){
  if(prev_state_bit != state_bit){return 1;}
  for(int r=0; r<8; r++){
    if(command_byte[r] != prev_command_byte[r]){return 1;}
  }
  return 0;
}
void convert_to_integer_3(){
  state_bit = rx_array[1] - '0';
  for(int q = 0; q < 3; q++){
    switch(rx_array[5-q]){
      case '0':
      decimal[q] = 0;
      break;
      case '1':
      decimal[q] = 1;
      break;
      case '2':
      decimal[q] = 2;
      break;
      case '3':
      decimal[q] = 3;
      break;
      case '4':
      decimal[q] = 4;
      break;
      case '5':
      decimal[q] = 5;
      break;
      case '6':
      decimal[q] = 6;
      break;
      case '7':
      decimal[q] = 7;
      break;
      case '8':
      decimal[q] = 8;
      break;
      case '9':
      decimal[q] = 9;
      break;
    }
  }
  
    command_decimal = decimal[0] + (decimal[1] * 10) + (decimal[2] * 100);
  
    /*Serial.print(command_decimal);*/
}
void decimal_to_binary(){             //taking the command_decimal converting it to binary and saving it in command_byte.
  for(int t = 0; t<8; t++){
  command_byte[7-t] = command_decimal % 2;
  command_decimal = command_decimal/2;
}
}

//this functions takes the command bit and according to given bit values controlls which motors are to be operated in what directions.
/*void allot_commands(){
  //if rotatory action is to be done.(but up down linear action can take place simultaneously.) 
  //-------------------------------------------------------------------------------------------------
  if(state_bit == 1){
    LRM_a.write(no_thrust);     //setting left right actuators to zero because no action is to be taken by
    LRM_b.write(no_thrust);     // those motors, when submarine is in rotatory state. 
    if(command_byte[4] == 0){
      FBM_a.write(no_thrust);    //for zero angular rotation.
      FBM_b.write(no_thrust);
    }
    else{
      if(command_byte[5] == 0){
        FBM_a.write(reverse_motionthrust);   //for anti_clockwise rotation.
        FBM_b.write(forward_motionthrust);
      }
      else{
        FBM_a.write(forward_motionthrust);  //for clockwise rotation.
        FBM_b.write(reverse_motionthrust);
      }
    }
    if(command_byte[6] == 0){
      if(thrust_variable <= 512){
        UDM_a.write(no_thrust);
        UDM_b.write(no_thrust);
      }
    }
    else{
      if(command_byte[7] == 0){
        if(thrust_variable <=512 ){
          UDM_a.write(down_motionthrust);      //for down motion.
          UDM_b.write(down_motionthrust);
        }
        else{
          thrust_variable = thrust_variable - 1;
          UDM_a.write(map(thrust_variable, 0, 1023, 0, 180));
          UDM_b.write(map(thrust_variable, 0, 1023, 0, 180));
        }
      }
      else{
        if(thrust_variable >= 780){
          UDM_a.write(up_motionthrust);    //for up motion.
          UDM_a.write(up_motionthrust);
        }
        else{
          thrust_variable = thrust_variable + 1;
          UDM_a.write(map(thrust_variable, 0, 1023, 0, 180));
          UDM_b.write(map(thrust_variable, 0, 1023, 0, 180));
        }
      }
    }
  }


  //if linear action is to be done.(linear motion in all six direction.)
 //------------------------------------------------------------------------------------------- 
  else{
    if(command_byte[2] == 0){
      FBM_a.write(no_thrust);    //no motion in front or back.
      FBM_b.write(no_thrust);
    }
    else{
      if(command_byte[3] == 0){
        FBM_a.write(reverse_motionthrust);    //motion in reverse direction.
        FBM_b.write(reverse_motionthrust);
      }
      else{
        FBM_a.write(forward_motionthrust);  //motion in forward direction.
        FBM_b.write(forward_motionthrust);
      }
    }
    if(command_byte[0] == 0){
      LRM_a.write(no_thrust);    //no motion in left or right direction.
      LRM_b.write(no_thrust);
    }
    else{
      if(command_byte[1] == 0){
        LRM_a.write(left_motionthrust);    //motion in left direction.
        LRM_b.write(left_motionthrust);
      }
      else{
        LRM_a.write(right_motionthrust);    //motion in right direction.
        LRM_b.write(right_motionthrust);
      }
    }
    if(command_byte[6] == 0){
      if(thrust_variable <= 512){
        UDM_a.write(no_thrust);
        UDM_b.write(no_thrust);
      }
    }
    else{
      if(command_byte[7] == 0){
        if(thrust_variable <=512 ){
          UDM_a.write(down_motionthrust);      //for down motion.
          UDM_b.write(down_motionthrust);
        }
        else{
          thrust_variable = thrust_variable - 1;
          UDM_a.write(map(thrust_variable, 0, 1023, 0, 180));
          UDM_b.write(map(thrust_variable, 0, 1023, 0, 180));
        }
      }
      else{
        if(thrust_variable >= 780){
          UDM_a.write(up_motionthrust);    //for up motion.
          UDM_b.write(up_motionthrust);
        }
        else{
          thrust_variable = thrust_variable + 1;
          UDM_a.write(map(thrust_variable, 0, 1023, 0, 180));
          UDM_b.write(map(thrust_variable, 0, 1023, 0, 180));
        }
      }
    }
  }
}*/
void allot_commands(){
  if(set_constant_thrust_interrupt == 0){
    if(state_bit == 1){
      if(prev_state_bit != state_bit){
        LRM_a.write(no_thrust);     //setting left right actuators to zero because no action is to be taken by
        LRM_b.write(no_thrust); 
        FBM_a.write(no_thrust);    //no motion in front or back.
        FBM_b.write(no_thrust);
      }

       
      //for constant thrust switch on and off setting.
      if(command_byte[2] == prev_command_byte[2]){
        if(command_byte[2] == 1){
          if(command_byte[3] != prev_command_byte[3]){
            if(command_byte[3] == 1){
              Time_a_1 = millis();
              Serial.print("Note_kiya");
            }
            else{
              Time_a_1 = millis();
              Serial.print("Note_kiya");
            }//doing nothing since the joystick is still in same position.
          }
        }
      }
      else{
        if(command_byte[2] == 1){
          if(command_byte[3] == 1){
            Time_a_1 = millis();
            Serial.print("Note_kiya");
          }
          else{
            Time_a_1 = millis();
            Serial.print("Note_kiya");
          }
        }
        else{
          Time_a_2 = millis();
          if(Time_a_2 - Time_a_1 >= 5000){
            if(constant_thrust_switch_interrupt == 0){
            constant_thrust_switch_interrupt = 1;
             Serial.print("zero ko 1 kiya");
            }
            else{
              constant_thrust_switch_interrupt = 0;
               Serial.print("1 ko zero kiya");
            }
          }
          else{
            Serial.print("ignore kiya");
          }
        }
      }

      //increasing or decreasing the constant thrust.......
      if(command_byte[0] == prev_command_byte[0]){
        if(command_byte[0] == 1){
          if(command_byte[1] != prev_command_byte[1]){
            if(command_byte[1] == 1){
              Time_b_1 = millis();
            }
            else{
              Time_b_1 = millis();
            }
          }
        }
      }
      else{
        if(command_byte[0] == 1){
          if(command_byte[1] == 1){
            Time_b_1 = millis();
          }
          else{
            Time_b_1 = millis();
          }
        }
        else{
          Time_b_2 = millis();
          if(Time_b_2 - Time_b_1 >= 5000){
            if(prev_command_byte[1] == 0){
              set_constant_thrust_interrupt = 1;
            }
            else if(prev_command_byte[1] == 1){
              set_constant_thrust_interrupt = 2;
            }
          }
        }
      }
      
      //for clock wise and counter-clock wise rotation
      if(command_byte[4]  == prev_command_byte[4]){
        if(command_byte[4] == 1){
          if(command_byte[5] != prev_command_byte[5]){
            if(command_byte[5] == 1){
              FBM_a.write(forward_motionthrust);  //for clockwise rotation.
              FBM_b.write(reverse_motionthrust);
              Serial.print("clockwise");
            }
            else{
              FBM_a.write(forward_motionthrust);  //for counter-clockwise rotation.
              FBM_b.write(reverse_motionthrust);
              Serial.print("counter_clockwise");
            }
          }//if prev_command_byte[5] == command_byte[5]; we need not change any pwm hence nothing is to be changed.
        }//if command_byte[4] == 0; the pwm is already on zero thrust state we need not change that since no change in input done.
      }
   
      else{//when command_byte[4] != prev_command_byte[4];
        if(command_byte[4] == 1){
          if(command_byte[5] == 1){
            FBM_a.write(forward_motionthrust);  //for clockwise rotation.
            FBM_b.write(reverse_motionthrust);
            Serial.print("clockwise");
          }
          else{
              FBM_a.write(reverse_motionthrust);  //for counter-clockwise rotation.
              FBM_b.write(forward_motionthrust);
              Serial.print("counter_clockwise");
          }
        }
        else{
          FBM_a.write(no_thrust);  //for counter-clockwise rotation.
          FBM_b.write(no_thrust);
          Serial.print("zero");
        }
      }


    //up down ka thrust.......!!!!!!!!!!!!!!!!!!!!!!!!!
    if(command_byte[6]  == prev_command_byte[6]){
        if(command_byte[6] == 1){
          if(command_byte[7] != prev_command_byte[5]){
            if(command_byte[7] == 1){
              UDM_a.write(up_motionthrust);      //for up motion.
              UDM_b.write(up_motionthrust);
            }
            else{
              UDM_a.write(down_motionthrust);      //for down motion.
              UDM_b.write(down_motionthrust);
            }
          }//if prev_command_byte[7] == command_byte[7]; we need not change any pwm hence nothing is to be changed.
        }//if command_byte[6] == 0; the pwm is already on zero thrust state we need not change that since no change in input done.
      }
   
      else{//when command_byte[6] != prev_command_byte[4];
        if(command_byte[6] == 1){
          if(command_byte[7] == 1){
            UDM_a.write(up_motionthrust);      //for up motion.
            UDM_b.write(up_motionthrust);
          }
          else{
            UDM_a.write(down_motionthrust);      //for down motion.
            UDM_b.write(down_motionthrust);
          }
        }
        else{
          if(constant_thrust_switch_interrupt == 0){
            UDM_a.write(no_thrust);  //for no up down motion.
            UDM_b.write(no_thrust);
          }
          else{
            UDM_a.write(constant_thrust);
            UDM_b.write(constant_thrust);
          }
        }
      }
  }


  //for  linear state of submarine.
    if(state_bit == 0){
      if(prev_state_bit != state_bit){
        FBM_a.write(no_thrust);  //for no rotation and no forward backward motion.
        FBM_b.write(no_thrust);
      }

      //for forward and reverse command going for element 2 and 3 from command_byte.
      if(command_byte[2] == prev_command_byte[2]){
        if(command_byte[2] == 1){
          if(command_byte[3] != prev_command_byte[3]){
            if(command_byte[3] == 1){
              FBM_a.write(forward_motionthrust);
              FBM_b.write(forward_motionthrust);
              Serial.print("forward");
            }
            else{
              FBM_a.write(reverse_motionthrust);
              FBM_b.write(reverse_motionthrust);
              Serial.print("Reverse");
            }//doing nothing since the joystick is still in same position.
          }
        }
      }

      //prev_command_byte not equal to current command_byte
      else{
        if(command_byte[2] == 1){
          if(command_byte[3] == 1){
            FBM_a.write(forward_motionthrust);
            FBM_b.write(forward_motionthrust);
            Serial.print("forward");
          }
          else{
            FBM_a.write(reverse_motionthrust);
            FBM_b.write(reverse_motionthrust);
            Serial.print("Reverse");
          }
        }
        else{
          FBM_a.write(no_thrust);
          FBM_b.write(no_thrust);
          Serial.print("zero");
        }
      }



      //working for  left right motion.!!!!!!!!!!!!!!!!!!!!!
      if(command_byte[0] == prev_command_byte[0]){
        if(command_byte[0] == 1){
          if(command_byte[1] != prev_command_byte[1]){
            if(command_byte[1] == 1){
              LRM_a.write(right_motionthrust);    //motion in right direction.
              LRM_b.write(right_motionthrust);
            }
            else{
              LRM_a.write(left_motionthrust);    //motion in left direction.
              LRM_b.write(left_motionthrust);
            }
          }
        }
      }
      else{
        if(command_byte[0] == 1){
          if(command_byte[1] == 1){
            LRM_a.write(right_motionthrust);    //motion in right direction.
            LRM_b.write(right_motionthrust);
          }
          else{
            LRM_a.write(left_motionthrust);    //motion in left direction.
            LRM_b.write(left_motionthrust);
          }
        }
        else{
          LRM_a.write(no_thrust);    //no motion in left or right direction.
          LRM_b.write(no_thrust);
        }
      }

      //up down ka thrust.......!!!!!!!!!!!!!!!!!!!!!!!!!
    if(command_byte[6]  == prev_command_byte[6]){
        if(command_byte[6] == 1){
          if(command_byte[7] != prev_command_byte[5]){
            if(command_byte[7] == 1){
              UDM_a.write(up_motionthrust);      //for up motion.
              UDM_b.write(up_motionthrust);
            }
            else{
              UDM_a.write(down_motionthrust);      //for down motion.
              UDM_b.write(down_motionthrust);
            }
          }//if prev_command_byte[7] == command_byte[7]; we need not change any pwm hence nothing is to be changed.
        }//if command_byte[6] == 0; the pwm is already on zero thrust state we need not change that since no change in input done.
     }
   
      else{//when command_byte[6] != prev_command_byte[4];
        if(command_byte[6] == 1){
          if(command_byte[7] == 1){
             UDM_a.write(up_motionthrust);      //for up motion.
            UDM_b.write(up_motionthrust);
          }
          else{
            UDM_a.write(down_motionthrust);      //for down motion.
            UDM_b.write(down_motionthrust);
          }
        }
        else{
          if(constant_thrust_switch_interrupt == 0){
            UDM_a.write(no_thrust);  //for no up down motion.
            UDM_b.write(no_thrust);
          }
          else{
            UDM_a.write(constant_thrust);
            UDM_b.write(constant_thrust);
          }
        }
      }
    }
  }
else{
  
      //for constant thrust increase/decrease.
      if(command_byte[2] == prev_command_byte[2]){
        if(command_byte[2] == 1){
          if(command_byte[3] != prev_command_byte[3]){
            if(command_byte[3] == 1){
              Time_c_1 = millis();
              Serial.print("Note_kiya");
            }
            else{
              Time_c_1 = millis();
              Serial.print("Note_kiya");
            }//doing nothing since the joystick is still in same position.
          }
        }
      }
      else{
        if(command_byte[2] == 1){
          if(command_byte[3] == 1){
            Time_c_1 = millis();
            Serial.print("Note_kiya");
          }
          else{
            Time_c_1 = millis();
            Serial.print("Note_kiya");
          }
        }
        else{
          Time_c_2 = millis();
          if(Time_c_2 - Time_c_1 >= 5000){
            if(prev_command_byte[3] == 0){
              if(set_constant_thrust_interrupt == 1){
                thrust = thrust - 10;
                constant_thrust = map(thrust, 0, 1023, 0, 180);
              }
              else if(set_constant_thrust_interrupt == 2){
                thrust = thrust - 1;
                constant_thrust = map(thrust, 0, 1023, 0, 180);
              }
            }
            else if(prev_command_byte[3] == 1){
              if(set_constant_thrust_interrupt == 1){
                thrust = thrust + 10;
                constant_thrust = map(thrust, 0, 1023, 0, 180);
              }
              else if(set_constant_thrust_interrupt == 2){
                thrust = thrust + 1;
                constant_thrust = map(thrust, 0, 1023, 0, 180);
              }
            }
            set_constant_thrust_interrupt = 0;
          }
          else{
            Serial.print("ignore kiya");
          }
        }
      }
  }
  prev_state_bit = state_bit;
  prev_command_byte[0] = command_byte[0];
  prev_command_byte[1] = command_byte[1];
  prev_command_byte[2] = command_byte[2];
  prev_command_byte[3] = command_byte[3];
  prev_command_byte[4] = command_byte[4];
  prev_command_byte[5] = command_byte[5];
  prev_command_byte[6] = command_byte[6];
  prev_command_byte[7] = command_byte[7];
}
