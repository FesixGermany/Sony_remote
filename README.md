I wanted a remote control for my Sony a6500 camera which has an IR receiver. There is a dedicated remote control with tons of buttons I don't need so I thought this would be a nice little project as all I need is a button for instant action and for a two second delay.  
Additionally the command for starting and stopping video recording is in the firmware so the functionality of the buttons can be changed.  
Exchange `ShutterCode` or `TwoSecsCode` with `VideoCode`.

```c
#define ButtonRightCode ShutterCode
#define ButtonLeftCode	TwoSecsCode
```

The whole circuit is powered by a 2032 button cell.  
The controller generates the necessary 40kHz modulation and sends the command set in the firmware when the button is pressed. As long as the command is transmitted the green LED is lit.
After that the controller goes into sleep mode to save battery.

In order to work with the camera the remote function needs to be enabled in the system menu.

The original code is from http://www.technoblogy.com/show?VFT
