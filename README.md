# Coal-Hopper
This is an Arduino based stepper motor controller for a model railway coal-hopper.

*** The board and software has been updated as more requirements were uncovered. The original files can be found under the V1 folder ***

The design is based around an Arduino Nano and a TMC2209 Silent Stepper motor controller. There are 6 general I/O pins that are used for the home & optional away microswitches, trigger input, busy output, Train detection and Conveyor control. Trimmers are used to set the speed of the hopper, length or travel (if not using Away switch) and delay time at the Away position before moving home.

Note. The 5v supply on the I/O pins comes from the Nano voltage regulator. It will not be enough to drive relays and servos. In this instance, an external power source is required.

The end-stop switches can be Normally-Open or Normally Closed types and set by configuration variables in the software. 

When a train is detected (reed switch), the conveyor motor, via a relay board, is turned on for a period of time.

When triggered by the level of coal in the hopper or the train leaving the unloading section, and after a delay, the stepper motor will move the hopper to the Away position. Once at the top and after a delay (when the hopper is emptied), the hopper will move back to the Home position. During this time, the busy output is used via a relay to disable track power in the unloading section. 

At initialisation time, the hopper is moved back to Home. If the hopper is already Home, it is moved away for a few turns and then back Home.

The Arduino will normally be powered through the Stepper Motor supply. There is a jumper provided so this can be isolated if the Arduino is connected by its USB port to a PC. This will avoid conflicting power on the Nano.

## Arduino Code
The code is held under the Arduino folder. Any changes to the connections can be made at the top of the file. The code is written 'long-hand' to make it readable and maintainable.

## Kicad Files
A Custom PCB was made for this and can be found under the Kicad folder.

![Image of the PCB Front](Images/pcb-v2-front.png)
![Image of the PCB Rear](Images/pcb-v2-back.png)

## Completed Boards
The image below shows a completed board. 

![Image of PCB Built](Images/pcb-v2-built.png)

