# Kaleidoscope
Kaleidoscope is a library that has all the plugins, code and tools necessary to develop a fully programable keyboard.
The library has been created to fulfill the necessities of https://shop.keyboard.io/ and his keyboards.

The library has been used to develop the Raise and now the Defy with some modifications of the original due to the limitation of the wired communication.
## Plugins

The most important part to understand about kaleidoscope is the use of **plugins** to capture and edit the keys pressed by the user.

As a first introduction about how to create a plugin read the following link:
https://kaleidoscope.readthedocs.io/en/latest/customization/plugin-authors-guide.html

Here is all the list of plugins available in kaleidoscope, currently we are not using all of them.
https://kaleidoscope.readthedocs.io/en/latest/customization/core-plugins.html

### Current list of plugins being use
```cpp
KALEIDOSCOPE_INIT_PLUGINS(
FirmwareVersion,
Upgrade,
USBQuirks,
MagicCombo,
IdleLEDs,
EEPROMSettings,
EEPROMKeymap,
FocusSettingsCommand,
FocusEEPROMCommand,
LEDCapsLockLight,
LEDControl,
PersistentLEDMode,
FocusLEDCommand,
LEDPaletteThemeDefy,
JointPadding,
ColormapEffectDefy,
LEDRainbowWaveEffectDefy,LEDRainbowEffectDefy,stalkerDefy,solidRedDefy, solidGreenDefy, solidBlueDefy,solidWhiteDefy,
PersistentIdleLEDs,
SettingsConfigurator,
Qukeys,
DynamicSuperKeys,
DynamicMacros,
Focus,
MouseKeys,
OneShot,
EscapeOneShot,
LayerFocus,
EEPROMUpgrade,
IntegrationTest,
HostPowerManagement);
```

## Event handlers

Here you can see all the events handlers that are provied with kaleidoscope, take into account take those are the event handlers of the kaleidoscope version 2.0 and in the defy we are currently in the version 1.1 so some of them will be different.
https://kaleidoscope.readthedocs.io/en/latest/api-reference/event-handler-hooks.html