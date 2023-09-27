# HID
In the keyboard we are using the following reports.
* Keyboard:
    * In charge of the letter, numbers, modifiers, led of caps, led of numpad etc..
    * There are two implementations:
        * Boot report: Works in the bios and only can send 6 keys at at time
            *   Check: NeuronWired/lib/KeyboardioHID/src/BootKeyboard/BootKeyboard.cpp  
        * NKRO report: Works only in the OS level and can send 113 at a time
            * Check: /NeuronWired/lib/KeyboardioHID/src/MultiReport/Keyboard.cpp
* Mouse:
    * Sends relative increments of movement to the pc
    *  Check: NeuronWired/lib/KeyboardioHID/src/MultiReport/Mouse.cpp
* Consumer control:
    * In charge of sending more especial keys as the media keys of, play, pause, brightness down or up.
    *  Check: NeuronWired/lib/KeyboardioHID/src/MultiReport/ConsumerControl.cpp
* System control:
    * Just especial keys to sleep, power down, and wake up the computer.
    *  Check: NeuronWired/lib/KeyboardioHID/src/MultiReport/SystemControl.cpp


For more info visit https://usb.org/sites/default/files/hut1_4.pdf