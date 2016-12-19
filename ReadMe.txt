ATmega328p-Powered Nixie Tube Clock

This clock uses the ATmega328p to show the current time on 6 nixie tubes. The ATmega328p pulls time data from the RTC and pushes it to the shift registers. The shift registers then transmit this data to the nixie tube drivers. The ATmega328p also controls a boost converter that increases the 5V power source to 170-200V. The duty cycle of the PWM pin controlling the boost converter can be adjusted by a potentiometer. There is also an option to turn the tubes completely off. A switch on the back adds or subtracts one hour from the current time for easy DST adjustment.

Schematics can be found here: https://easyeda.com/emcauliffe/Nixie_Tube_Clock-bb9cbc4c464f4db7bebc2738000192aa