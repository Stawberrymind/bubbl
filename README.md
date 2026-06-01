<img width="640" height="639" alt="bubbl_assembled" src="https://github.com/user-attachments/assets/760c4679-654a-4ba0-b6cc-c4b3d01c640f" />

# bubbl- A tamagotchi-inspired Desk Pet to keep friendships alive.

A friendship-powered Tamagotchi-inspired pet for your desk, which looks like a Japanese onigiri, keeps friend groups alive by maintaining a common pet and sending nudges to each other, just to keep the drifting friend group a little closer! 

## How does it work?
There is one shared pet for everyone. Every bubbl device in the same group (friend group) has the same pet, you can feed the pet, make it rest, and play with it. If no one in the friend group interacts with the pet, it decays the pet's stats, causing harm to it :(, and this incentivises the friend group to keep interacting with the pet. In the process, friends can send each other nudges, which are sound notifcations which also vibrate the device, this is useful when. This adds a fun component to the already interesting and interactive interconnected pet system. 

## Why did I make it?
In the current day and age, where friendships are basically just sending each other reels, friendships are dying fast. There is no real connection, and bubbl I hope helps solve that issue by having a pet to take care of, it makes sure all the members of the friend group have equal responsibility and are constantly reminded of each other, thus hopefully reducing friction in friendships.
 
## Components Required for the Build
I have listed everything in the project; you can find the same in the BOM

| No.   | Quantity | Comment                        | Footprint         | Link                                                                                                       | Price | Total (USD) | Note                            |
| ----- | -------- | ------------------------------ | ----------------- | ---------------------------------------------------------------------------------------------------------- | ----- | ----------- | ------------------------------- |
| 1     | 1        | XIAO-ESP32-S3-SMD              | XIAO-ESP32-S3-SMD | https://www.aliexpress.com/item/1005006987905528.html                                                      | 6.77  | 6.77        |  Select 1pc in listing          |
| 2     | 2        | 10uF                           | C0603             | https://jlcpcb.com/partdetail/HRE-CGA0805X5R106K500MT/C6119889                                             | 0.01  | 0.02        | \-                              |
| 3     | 2        | 100nF                          | C0603             | https://jlcpcb.com/partdetail/39123-0805B104K500NT/C38141                                                  | 0.01  | 0.02        | \-                              |
| 4     | 1        | MAX98357A                      | \-                | https://www.aliexpress.com/item/1005007003802663.html                                                      | 0.65  | 0.65        |  Select 1pc in listing          |
| 5     | 1        | INMP441                        | \-                | https://www.aliexpress.com/item/1005008870682636.html                                                      | 0.75  | 0.75        | \-                              |
| 6     | 1        | 1.3" OLED                      | \-                | https://www.aliexpress.com/item/1005008365029314.html                                                      | 2.03  | 2.03        | Select 4pinI2C-White in listing |
| 7     | 1        | Analog Joystick Module         | \-                | https://www.aliexpress.com/item/1005012147405483.html                                                      | 1.92  | 1.92        | \-                              |
| 8     | 1        |  Vibration Motor Module        | \-                | https://www.aliexpress.com/item/33009331080.html                                                           | 0.49  | 0.49        | \-                              |
| 9     | 10       | M2 X 3mm Phillips Round head   | \-                | https://onlyscrews.in/products/m2-x-5mm-phillips-round-head-laptop-screw                                   | 0.04  | 0.4         | \-                              |
| 10    | 5        | M1.4 X 2mm Phillips Round head | \-                | https://onlyscrews.in/products/m1-4-x-2mm-phillips-round-head-laptop-and-mobile-screw-dia-1-4mm-length-2mm | 0.023 | 0.115       | \-                              |
| 11    | 6        | M2 X 10mm Hard Dowel Pins      | \-                | https://onlyscrews.in/products/m2-x-10mm-hard-dowel-pins-dia-2mm-length-10mm                               | 0.051 | 0.306       | \-                              |
| 12    | 5        | 2 Layer PCB                    | \-                | https://cart.jlcpcb.com/quote?spm=jlcpcb.Public.2006                                                       | 2.1   | 2.1         | \-                              |
| Total |          |                                |                   |                                                                                                            |       | 15.571      |                                 |



## PCB and Schematic
I designed it in EasyEDA. You can find the Gerber files in the repo. The schematic is:

<img width="914" height="465" alt="schematic" src="https://github.com/user-attachments/assets/6e1c4cc6-730e-4b70-8799-2ce11445bd60" />


And the PCB is a 2-layer PCB with a ground plane as layer 2:

<img width="417" height="568" alt="pcb" src="https://github.com/user-attachments/assets/c04a72d8-f784-4000-b0a9-d6835a9dc2b6" />


3D Render of the PCB:

<img width="540" height="741" alt="pcb_3d" src="https://github.com/user-attachments/assets/56dab36e-8f00-48ec-be77-da117c3318d2" />


## Case
Top and bottom view of the Case:

<img width="702" height="347" alt="case_front_back" src="https://github.com/user-attachments/assets/86d234f9-9f33-4117-b6d8-ec81791c932f" />




## Assembly 
1. Take the bottom case and install the PCB, screw it into the bottom case using a M2 X 3mm screw. Before screwing it, wire the BAT+ and BAT- from LiPO to the PCB.
2. Screw in the Vibration motor after making the connections, as shown in the picture, using M2 x 3 mm
   <img width="282.2" height="323.5" alt="assembly_1" src="https://github.com/user-attachments/assets/2e2f0d22-dfcc-4465-a6bd-f06eada222eb" />
 
3. Wire the Joystick Module to the PCB, then screw it in using M2 X 3mm screws like shown in the picture
   <img width="310" height="235" alt="assembly_2" src="https://github.com/user-attachments/assets/0cbfe255-37e9-4d22-8f17-4f457c09ee6b" />


 


## Setting it 


## Files




## Datasheets


## Zine


