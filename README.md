# Spintrak-and-buttons-Arduino-encoder
>Turns the ultimarc spintrak spinners into either USB mouse x-axis mode or left-stick-x-axis mode, using an Arduino Pro Micro. 
>The arduino also works as an arcade button+stick encoder as well. Giving you stick+spinner+8buttons+hotkey functionality.

>The spintrak is tuned to have the same sensitivity as the original included ultimarc encoder.
>If you want to tune the sensitivity to your own liking, edit the values in this line:

>```accumulator -= (delta * 6) / 5;```
>For example: accumulator -= (delta * 3) / 5; would be half as sensitive

>Stick mode sensitivity:
>```int32_t temp = (int32_t)stickX + ((int32_t)move * 80);```
>(80 is default, higher = more sensitive)

>The hotkey shifts buttons into a higher button number.

- Gamepad Mode: Default mode, holding the **Hotkey Enable** button shifts buttons into the numbers in RED until released
- Spinner Mouse Mode: Activates automatically as soon as the spinner is detected (spun), **Hotkey+Down** returns to Gamepad Mode.
- Spinner Stick Mode: Activate by holding **Hotkey+Left**, makes the spinner simulate an analog stick instead, pressing up will immediately recenter the stick. 
Works well with racing games. Hotkey+Down returns to Gamepad Mode.

>Pinout for the build are shown in the image below. Pay attention to the wire colors of the Spintrak.

>Remember to copy over the Library dependencies into your Documents/Arduino/Libraries folder before flashing the .ino to the Arduino Pro Micro.

---

<img src="docs/Build.jpg" width="400"> <img src="docs/Build2.jpg" width="400"> <img src="docs/ArduinoPinout.png" width="400"> <img src="docs/ButtonFunctions.png" width="400"> 

---
