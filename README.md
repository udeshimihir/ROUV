# ROUV
This project consists of two programms one is to be uploaded on arduino nano which been used for remote.
The other code is to be uploaded on arduino uno which is the controller of the ROUV.

Programm outline for transmitter aka remote.
Values from two joystick and a potentiometer is read then command byte is formed.
 Formation of command_byte. Command_byte is a array containing 8 space where every two space determine the motion.
 First two element will tell whetjer Front or Back motion is to be done, if yes than is it front motion or is it back motion.
 Similarly next two element will tell about right left motion, the next two about the up-down motion and last two about the 
 clock_anti-clock wise motion.
 Once the command_byte is formed, it is converted into a decimal  no. then it is converted into character and saved in a string.
 Then a string is created which will be transmitted to the arduino inside the submarine, the string will have opening tag '<' and then the state_bit,
 then a differentiater ','comma, then it will have the string in which decimal is saved in the character form '000', then closing tag '>'.
 The opening and closing tag helps to determine whether proper set is received or not.
 This created string is transmitted character by character over the wires to the arduino inside the ROUV.
 
 
Programm outline for the Reciever/Controller
First esc is calibrated and the then it waits for the serial communication the be available.
As the communication is achieved the microcontroller starts reading the data and then check whether correct data is transferred or not.
If the dataset seems wrong then it is ignored and starts to read the next data set.
If the dataset seems correct it will change the pwm given to the motor according to the dataset received.
There are few interrupts in the programm which will change the mode of the ROUV, into linear, rotatory, or calibrating.
In linear it will have linear motion in all direction, in rotatory motion it will have linear motion in up down direction and rotatory motion along the vertical axis.
In calibrating mode the constant thrust can be switched on or off, it can also change the value of constant thrust by increasing or decreasing the pwm percentage by 10 or 1 unit.
The calibrating mode helps us to find a better constant thrust, if water density is changed or the volume of ROUV is changed. Doesnt give very accurate thrust due basic electronics and no sensor but is better than no constant thrust or same constant thrust.

Issues which can arrise.
Baud rate for both arduinos should be same. Delay between transmission of two character should be appropriate enough that it doesnt overlap the next or previous transmitted character.
The delay is decided according the baud rate and the lenght of wire used. Trial and error method works fine while determining the delay.

While calibrating esc note that the zero thrust is given on 50% duty cycle of pwm not a 0% duty cycle, also it better to use servo library to send pwm rather than using analogwrite function.
Note that you should only use pwm pin for signalling the esc and not the digital pins.

The programm has one problem still that when the communication is interrupted or broken midway the submarine will continue to do what it was doing which is a very big problem
will soon resolve this problem.
 
