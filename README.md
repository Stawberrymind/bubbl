<img width="450" height="450" alt="bubbl_assembled" src="https://github.com/user-attachments/assets/760c4679-654a-4ba0-b6cc-c4b3d01c640f" />

# bubbl- A tamagotchi-inspired desk pet to keep friendships alive.

A friendship-powered Tamagotchi-inspired pet for your desk, which looks like a Japanese onigiri, keeps friend groups alive by maintaining a common pet and sending nudges to each other, just to keep the drifting friend group a little closer! 

## How does it work?
There is one shared pet for everyone. Every bubbl device in the same group (friend group) has the same pet, you can feed the pet, make it rest, and play with it. If no one in the friend group interacts with the pet, the pet's stats decay, causing harm to it :(, which incentivises the friend group to keep interacting with the pet. In the process, friends can send each other nudges, which are sound notifcations which also vibrate the device; this is useful when. This adds a fun component to the already interesting and interactive interconnected pet system. 

## Why did I make it?
In the current day and age, where friendships are basically just sending each other reels, friendships are dying fast. There is no real connection, and bubbl I hope helps solve that issue by having a pet to take care of; it makes sure all the members of the friend group have equal responsibility and are constantly reminded of each other, thus hopefully reducing friction in friendships.
 
## Components Required for the Build
I have listed everything in the project; you can find the same in the BOM

| No.   | Quantity | Comment                        | Footprint         | Link                                                                                                       | Price | Total (USD) |
| ----- | -------- | ------------------------------ | ----------------- | ---------------------------------------------------------------------------------------------------------- | ----- | ----------- |
| 1     | 1        | XIAO-ESP32-S3-SMD              | XIAO-ESP32-S3-SMD | https://www.aliexpress.com/item/1005006987905528.html                                                      | 6.77  | 6.77        |
| 2     | 2        | 10uF                           | C0603             | https://jlcpcb.com/partdetail/HRE-CGA0805X5R106K500MT/C6119889                                             | 0.01  | 0.02        |
| 3     | 2        | 100nF                          | C0603             | https://jlcpcb.com/partdetail/39123-0805B104K500NT/C38141                                                  | 0.01  | 0.02        |
| 4     | 1        | MAX98357A                      | \-                | https://www.aliexpress.com/item/1005007003802663.html                                                      | 0.65  | 0.65        |
| 5     | 1        | INMP441                        | \-                | https://www.aliexpress.com/item/1005008870682636.html                                                      | 0.75  | 0.75        |
| 6     | 1        | 1.3" OLED                      | \-                | https://www.aliexpress.com/item/1005008365029314.html                                                      | 2.03  | 2.03        |
| 7     | 1        | Analog Joystick Module         | \-                | https://www.aliexpress.com/item/1005012147405483.html                                                      | 1.92  | 1.92        |
| 8     | 1        |  Vibration Motor Module        | \-                | https://www.aliexpress.com/item/33009331080.html                                                           | 0.49  | 0.49        |
| 9     | 10       | M2 X 3mm Phillips Round head   | \-                | https://onlyscrews.in/products/m2-x-5mm-phillips-round-head-laptop-screw                                   | 0.04  | 0.4         |
| 10    | 5        | M1.4 X 2mm Phillips Round head | \-                | https://onlyscrews.in/products/m1-4-x-2mm-phillips-round-head-laptop-and-mobile-screw-dia-1-4mm-length-2mm | 0.023 | 0.115       |
| 11    | 6        | M2 X 10mm Hard Dowel Pins      | \-                | https://onlyscrews.in/products/m2-x-10mm-hard-dowel-pins-dia-2mm-length-10mm                               | 0.051 | 0.306       |
| 12    | 1        | 400mAh LiPo                    | \-                | https://robu.in/product/400mah-pcm-protected-micro-li-po-battery-2/                                        | 2.86  | 2.86        |
| 13    | 5        | 2 Layer PCB                    | \-                | https://cart.jlcpcb.com/quote?spm=jlcpcb.Public.2006                                                       | 2.1   | 2.1         |
| Total |          |                                |                   |                                                                                                            |       | 18.431 USD  |



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
Note: Please follow the wiring diagram :)
1. Take the bottom case and install the PCB, and screw it into the bottom case using a M2 x 3 mm screw. Before screwing it, wire the BAT+ and BAT- from LiPO to the PCB.

2. Screw in the Vibration motor after making the connections, as shown in the picture, using M2 x 3 mm

   <img width="282.2" height="323.5" alt="assembly_1" src="https://github.com/user-attachments/assets/2e2f0d22-dfcc-4465-a6bd-f06eada222eb" />
 
3. Wire the Joystick Module to the PCB, then screw it in using M2 x 3 mm screws as shown in the picture.

   <img width="310" height="235" alt="assembly_2" src="https://github.com/user-attachments/assets/0cbfe255-37e9-4d22-8f17-4f457c09ee6b" />

5. Screw in the Speaker, with the diaphragm facing outward towards the speaker holes, using a M2 X 3 mm screw as shown in the picture.
 
   <img width="308" height="234" alt="assembly_3" src="https://github.com/user-attachments/assets/1128d4ed-6ca1-47fe-8aa3-4ca30eb71973" />

6. Connect the amplifier to the microcontroller, then screw the amp into the case using  M1.4 x 2 mm screws as shown. Then connect the speaker to the amplifier.
   
   <img width="285" height="208" alt="assembly_4" src="https://github.com/user-attachments/assets/f320be90-6768-496d-8e1f-9a74ac440ee2" />
   
7. The bottom of the case should now look as shown below.
   
   <img width="320" height="241" alt="assembly_5" src="https://github.com/user-attachments/assets/29431dd5-5ce9-484f-9330-d087a30ad550" />

8. Insert M2 x 10 mm hard metal dowel pins into the bottom case as shown below.
   
   <img width="268" height="228" alt="assembly_6" src="https://github.com/user-attachments/assets/c8960940-0ccb-4962-ae7e-a6ee0f30206a" />

9. Now it's time to assemble the top case, screw in the OLED to the underside of the top case using M2 x 3 mm screws as shown and wire it to the microcontroller.

   <img width="313" height="279" alt="assembly_7" src="https://github.com/user-attachments/assets/37cfa003-aea0-4b44-a69c-1dbc0a6fef15" />

10. Install the INMP441 I2S Microphone with M1.4 x 2 mm screws into the case as shown and wire it to the microcontroller.
 
   <img width="283" height="204" alt="assembly_8" src="https://github.com/user-attachments/assets/91ab84ec-8c45-4bab-84e9-2c0283265543" />

11. Lower the top case onto the bottom one, applying firm pressure equally from all sides to make sure the top locks in with the bottom using the dowel pins.
12. Your own personal bubbl is now complete!

## Setting it up
Section I: Setting up Supabase:
1. Go to https://supabase.com/ and create a new account
2. Create a new project

   <img width="182" height="152" alt="image" src="https://github.com/user-attachments/assets/e76cbfa2-75d0-4843-92f5-af39da6d6c31" />

4. Go to the SQL Editor Tab and paste the content of the bubbl_supabase file in Firmware into the editor

   <img width="356" height="218" alt="image" src="https://github.com/user-attachments/assets/2c5e036c-82f1-482c-9932-c57d885fc9d5" />

5. Run the script, which will lead to the creation of the following tables: pet, users, events, and nudges
6. In the SQL Editor, enter your friends' names in place of user1...4

7. Go to your Supabase settings, then to API and copy your:
   * Project URL
   * Anom public key

Section II: Setting up the firmware:

7.  In the bubbl.ino file, replace the SSID and password fields with your credentials, enter the Supabase URL and Anon Key below, and set up your name.
   * Note: All friends must use the same project
   * Be listed in the users table
   * Have a unique username

8. Install the required libraries and upload the firmware to bubbl

9. Wohooooo! You and your friend group now have a working bubbl of your own!


## Files
```text
    └── bubbl/
        ├── CAD_Files/
        │   ├── bubbl_case.step
        |   ├── bubbl.stl
        │   ├── bubbl_assembled.f3z
        │   └── bubbl_assembled.step
        ├── PCB_Files/
        │   ├── bubbl_gerber.zip
        │   └── bubbl_source.epro2
        ├── Firmware_Files/
        │   ├── bubbl.ino
        │   ├── bubbl_supabase.mdf
        │   └── bubbl.wav
        ├── bubbl_BOM.csv
        ├── bubbl_Zine.pdf
        └── README.md
        └── Assets/
        
```
## Future Additions
In upcoming software versions, I will include support to send short voice notes to your buddies!

## Credits

Abhinav [goat]

The idea: https://github.com/TaniWanKenobi/tamagotchi

Vibration motor module: https://grabcad.com/library/vibration-motors-1

OLED module: https://grabcad.com/library/oled-display-1-3-2

Amplifier: https://grabcad.com/library/max98357a-3w-i2s-amplifier-1

Microphone: https://grabcad.com/library/inmp441-microphone-module-1

Joystick: https://grabcad.com/library/dual-axis-xy-joystick-module-with-push-button-1


## Zine

<img width="1410" height="2000" alt="bubbl_zine" src="https://github.com/user-attachments/assets/03033711-a8e1-4723-8ae2-41cc92df9b25" />


