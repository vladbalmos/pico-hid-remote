## HID Bluetooth Remote

HID Bluetooth (BLE) Remote based on a Pico W.

## Hardware requirements

* Pico W
* 5 x micro switches
* TP5100 charging module
* micro USB female
* 1 x 18650 lithium battery
* pico probe (optional)
* 1 x 3mm blue LED
* 3 x 3mm red LED

## Software dependencies

* pico-sdk (1.5.0)
* pico-extras
* opencd (optional)
* pico probe (optional)


## Build

    make build
    
## Install
Via pico probe:

    make install

## Buttons

* FN
    - long press - turn off
    - double press - show baterry level
    
* PLAY
    - wakeup from sleep if sleeping
    - toggle play/pause
    
* VOL UP
    - increase volume

* VOL DOWN
    - decrease volume

* RESET
    - reset PICO
    
## Behaviour
    
On startup the device is discoverable for one minute, after which, if not paired the device will go to sleep to preserve battery.

If the device is paired, discoverability is turned off until disconnected.

Once paired, the device will go to sleep if no button was pressed in 10 minutes to preserve battery.

### LEDs

* blue
    - flashes while not connected and in pairing mode
    - stable for 5 seconds after pairing
    
* 3 x red
    - show battery level