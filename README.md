# Coal-Hopper
This is an Arduino based stepper motor controller for a model railway coal-hopper.

When triggered, the stepper motor will move the hopper to the Away position. After a delay (when the hopper is emptied), the hopper will move back to the Home position.

The design is based around an Arduino Nano and TMC2209 Silent Stepper motor controller. There are connection for end-stop switches, but only the Home switch is required. There are trigger input and busy output connections protected by opto-couplers. These can be removed if required and direct connections used instead (i.e relay / switch for input). Trimmers can be used to set the speed of the hopper, length or travel (if not using Away switch) and delay time at the Away position before moving home.

## Arduino Code
The code is held under the Arduino folder. Any changes to the connections can be made at the top of the file. The code is written 'long-hand' to make it readable and maintainable.

## Kicad Files
A Custom PCB was made for this and can be found under the Kicad folder.

![Image of the PCB](Assets/pcb-blank.png)

## Completed Boards
The images below show completed boards. 
This one has the Opto-Couplers installed.

![Image of PCB with Opto-Couplers](Assets/pcb-opto.png)

This one has direct links and can be used for relay contacts or switch. It also has headers for the LEDs so they can be mounted remotely.

![Image of PCB with Opto-Couplers](Assets/pcb-direct.png)


