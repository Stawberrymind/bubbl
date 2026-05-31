


# bubbl- A Tamagotchi-inspired Desk Pet to keep friendships alive.

2SB is a custom USB-C Flash Drive, or more commonly called a Pendrive. To the computer, it behaves like a single USB Drive, but the content in it completely changes based on the position of the switch, which is used to change which NAND Flash we're using.



## How does it work?
To understand how it works, let's first understand the components of a Flash Drive (oversimplified);

1. A port- to connect to a device (USB-C).
2. USB Controller Chip- which controls the flash chips (Alcor AU6989SN-GT).
3. NAND Flash- The little storage chips that actually store the data (Samsung K9G8G08U0M).

So here, what really is happening is that we have two Independent Samsung NAND Flash chips (8 GB each), and both of them share the same data bus (shared connection highway) to the Alcor USB Controller. And there is a switch, a sliding switch, which controls which Chip's "CE Pin" is active; meanwhile, the other one gets pulled to LOW, meaning inactive, so the active chip responds to the commands from the Controller!
So this eliminates software partitions, just simple hardware magic!

Refer to this flowchart for a better understanding!



## Why did I make it?
Honestly, I recently got into the world of Linux distros, and I started carrying 3 USB Drives in my bag at all times of distros, documents, tools, and it got really annoying because I didn't know which was which, and I just wondered if I could get rid of the hassle and have 2 USB drives in the format of one and I could switch between them. I did a bit of research and realised partitions exist; however, for someone who doesn't wanna go through the hassle of that, a physical switch seemed like the best alternative. Then I went to see if I could buy it online, but nothing like that was available, so I built it for myself!

## Components Required for the Build
I have listed everything in the project; you can find the same in the BOM!

However, about the BOM,
The link for the NAND Flash chips is long because I can't shorten it, and if I change it, the link no longer works. It is also a pack of 5 pieces because it's the only available listing. Sorry for that.

NOTE: This is just a list of the parts. For the actual links to purchase and more details on the part, please visit the BOM_2SB.csv in the repo :)

| No.   | Quantity | Description                            | Part Number         | Cost Per Unit  | Total(USD) |
|-------|----------|----------------------------------------|---------------------|-------|------------|
| 1     | 9        | 100nF capacitor                        | 0805B104K500NT      | 0.01  | 0.09       |
| 2     | 1        | 1uF capacitor                          | CGA0402X5R105K100GT | 0.01  | 0.01       |
| 3     | 1        | 10uF capacitor                         | CGA0805X5R106K500MT | 0.07  | 0.07       |
| 4     | 2        | 1N4007 diode                           | 1N4007              | 0.01  | 0.02       |
| 5     | 1        | GZ1608D601TF ferrite bead              | GZ1608D601TF        | 0.02  | 0.02       |
| 6     | 1        | 330Ω resistor                          | FRC0603J331 TS      | 0.01  | 0.01       |
| 7     | 2        | 10kΩ resistor                          | RC0603JR-0710KL     | 0.01  | 0.02       |
| 8     | 2        | 5.1kΩ resistor                         | FRC0402F5101TS      | 0.01  | 0.02       |
| 9     | 1        | MSK12C02 switch                        | MSK12C02            | 0.06  | 0.06       |
| 10    | 1        | AMS1117-3.3 regulator                  | AMS1117-3.3         | 0.2   | 0.2        |
| 11    | 1        | AU6989SN-GT USB Controlller chip       | AU6989SN-GT-GHL 48  | 13.88 | 13.88      |
| 12    | 2        | K9G8G08U0M NAND Flash Chip             | K9G8G08U0M          | 24.03 | 24.03      |
| 13    | 1        | USB TYPE-C 24P-GTJB 040 Male connector | TYPE-C 24P-GTJB 040 | 0.136 | 0.136      |
| 14    | 2        | M1.6 X 5mm Phillips Round head Screw   | -                   | 0.067 | 0.134      |
| 15    | 5        | FR4-4 Layer PCB (JLCPCB)               | -                   | 7.1   | 7.1        |
| Total |          |                                        |                     |       | 45.8  USD     |


## PCB and Schematic
I designed it in EasyEDA, you can find the gerber files in the repo, the schematic is:


And the PCB is a 4-layer PCB with a ground plane as layer 4:


3D Render of the PCB:


## Case
Top and bottom view of the Case:


The case features a Snap-Fit design, so you just have to press the top and bottom together. The only time you need screws is to screw the PCB into the bottom case to hold it in place!

## Assembly 
Please note this isn't final and it can change. I’ve yet to build it, and when I do, I'll update this section to be more accurate :)

1. Get the parts and the PCB (You can order the stencil with the PCB if you wish to solder with hot air or reflow, I prefer hand soldering SMD components :) )

2. Start by soldering the USB Controller onto the PCB, aligning the dot on the chip with the silkscreen on the board.
Carefully solder the NAND Flash packages while aligning the chip's dot with the silkscreen. Make sure you don't bridge the pins while soldering. It's highly recommended to use an ultra-fine soldering tip while you solder.

3. Now solder on the AMS117 3.3V Voltage regulator, be generous with the solder :) 

4. Time to solder the two diodes onto the board
Now solder all the capacitors, resistors and the ferrite bead onto the board along with the USB-C header (Remember it has pins on both sides of the board!)

5. Now, give everything a general inspection with ur eyes, take some time, make sure everything is soldered properly, and remember when you solder, use flux!

6. Use some rubbing alcohol/ IPA to clean the flux off the board
Before connecting it to your computer, test the voltages with a multimeter.

7. Connect the USB-C with a charger or power bank (5V) and measure:

* Flash Chip VCC should read 3.3V

* USB Controller pin 15 (V12) should read 1.2V

* USB Controller pin 13 (AVDD5V) should read 5V

Hopefully everything should be correct, but if it's not, just check if all the pins are soldered properly :) 

## Setting it up
1. Now we plug it into the computer, the AU6987SN-GT comes pre-programmed from the factory with USB mass storage firmware, so we don't need to program the chip. However, we need to format the NAND Flashes
2. Install the Alcor MP software from ” https://www.usbdev.ru/files/alcor/alcormp/ “. Install the latest AlcorMP_UFD 6.15.00 from the website, extract the .rar file and run the software. It should auto-detect the Controller Chip.

* Select the flash chip model: K9F1G08U0E
* Click Start/Format
* Wait for it to complete
* Plug it out, switch the switch to Flash 2
* Repeat!

Now everything should be done :)
Please note, if you have any issues with this step, feel free to contact me Aarav Kohli at aaravkohli9@gmail.com 
## Setting up the case
Now put the set-up PCB into the Bottom case, aligning it with the USB- C slot and the screw holes in the bottom case, then we screw in two M1.6 x 5 mm Phillips Round head Screws, make sure you don't screw them in all the way; gently screw them in till the board seems secured. Now put the top case on and tada! You now have a two-in-one working USB Drive! 


## Files

```text
    └── 2SB/
        ├── CAD_Files/
        │   ├── 2SB_Case.f3d
        │   ├── 2SB_Case.step
        │   ├── 2SB_Fully_Assembled.f3z
        │   └── 2SB_Fully_Assembled.step
        ├── PCB_Files/
        │   ├── 2SB_Gerber.zip
        │   └── 2SB_Source.epro
        ├── 2SB_BOM.csv
        ├── 2SB_Zine.pdf
        └── README.md
        └── Assets/
        
```   


## Datasheets
AU6989SN-GT-http://am.adianshi.com:6805/%E5%BC%80%E5%8D%A1%E8%BD%AF%E4%BB%B6/%E6%96%87%E6%A1%A3/au6989sn.pdf

K9F1G08U0E-https://www.alldatasheet.com/datasheet-pdf/view/1132608/SAMSUNG/K9F1G08U0E.html

AlcorMP-https://www.usbdev.ru/files/alcor/alcormp/

## Zine
<img width="1410" height="2000" alt="zine" src="https://github.com/user-attachments/assets/0b0ad288-0bad-4b84-b21d-be3f43604880" />


