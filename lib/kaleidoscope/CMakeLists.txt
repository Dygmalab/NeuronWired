add_library(kaleidoscope INTERFACE)

target_include_directories(kaleidoscope
        INTERFACE
		kaleidoscope/src
        )

target_sources(kaleidoscope
        INTERFACE
        kaleidoscope/src/kaleidoscope/device/dygma/defyWN/universalModules/Hand.cpp
        kaleidoscope/src/kaleidoscope/device/dygma/defyWN/universalModules/SettingsConfigurator.cpp
        kaleidoscope/src/kaleidoscope/device/dygma/defyWN/DefyWN.cpp
        kaleidoscope/src/kaleidoscope/key_events.cpp
        kaleidoscope/src/kaleidoscope/layers.cpp
        kaleidoscope/src/kaleidoscope/plugin/DynamicMacros.cpp
        kaleidoscope/src/kaleidoscope/plugin/SuperkeysHandler.cpp
        kaleidoscope/src/kaleidoscope/plugin/Superkeys/Timeline/Timeline.cpp
        kaleidoscope/src/kaleidoscope/plugin/Superkeys/Actions/ActionsDriver.cpp
        kaleidoscope/src/kaleidoscope/plugin/Superkeys/Superkey/Superkey.cpp
        kaleidoscope/src/kaleidoscope/plugin/EEPROM-Keymap.cpp
        kaleidoscope/src/kaleidoscope/plugin/EEPROM-Settings.cpp
        kaleidoscope/src/kaleidoscope/plugin/EEPROM-Settings/crc.cpp
        kaleidoscope/src/kaleidoscope/plugin/Escape-OneShot.cpp
        kaleidoscope/src/kaleidoscope/plugin/FocusSerial.cpp
        kaleidoscope/src/kaleidoscope/plugin/HostPowerManagement.cpp
        kaleidoscope/src/kaleidoscope/plugin/IdleLEDs.cpp
        kaleidoscope/src/kaleidoscope/plugin/LayerFocus.cpp
        kaleidoscope/src/kaleidoscope/plugin/LEDControl/LEDUtils.cpp
        kaleidoscope/src/kaleidoscope/plugin/LEDControl.cpp
        kaleidoscope/src/kaleidoscope/plugin/LED-Stalker.cpp
        kaleidoscope/src/kaleidoscope/plugin/MagicCombo.cpp
        kaleidoscope/src/kaleidoscope/plugin/MouseKeys.cpp
        kaleidoscope/src/kaleidoscope/plugin/MouseKeys/MouseWrapper.cpp
        kaleidoscope/src/kaleidoscope/plugin/OneShot.cpp
        kaleidoscope/src/kaleidoscope/plugin/PersistentLEDMode.cpp
        kaleidoscope/src/kaleidoscope/plugin/Qukeys.cpp
        kaleidoscope/src/kaleidoscope/plugin/USB-Quirks.cpp
        kaleidoscope/src/kaleidoscope/Runtime.cpp
        kaleidoscope/src/kaleidoscope_internal/device.cpp
        kaleidoscope/src/kaleidoscope_internal/LEDModeManager.cpp
        )

target_link_libraries(kaleidoscope
        INTERFACE
        )
