MeArmController
===============

This is an arduino project which provides serial control of 4 servos

Intended for control of the MeArm: http://www.thingiverse.com/thing:298820

Accepts commands from serialin the for of: <A-D><angle>
Such as:
 A90 <- sets the angle of the A servo to 90
 A10B40C10 <- Sets A servo to 10, B servo to 40 and C servo to 10

Connect data pin for the servos A-D to arduino pins 9-12
