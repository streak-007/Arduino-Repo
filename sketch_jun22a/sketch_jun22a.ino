#include<Servo.h>
Servo servo;

int x_axis;

int servo_val;

void setup()

{

pinMode(A1,INPUT);

servo.attach(10);
Serial.begin(9600);

}

void loop()

{
x_axis=analogRead(A1);
servo_val=map(x_axis,0,1023,0,180);
 Serial.print(x_axis);
 Serial.print("\n");
 servo.write(servo_val);
// int i = Serial.read();
// if(i == 0 || i == 45 || i == 90 || i == 135 || i == 180 )
// {
//   servo.write(i);
//   Serial.print(i);
//   Serial.print("\n");
// }



// for (int i = 0; i < 180; i++) {
// delay(200);
// Serial.print(i);
// Serial.print("\n");
// }

}