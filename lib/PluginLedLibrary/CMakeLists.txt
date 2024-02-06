add_library(PluginLedLibrary INTERFACE)

target_include_directories(PluginLedLibrary
        INTERFACE
        PluginLedLibrary/src
        )

target_sources(PluginLedLibrary
        INTERFACE
        PluginLedLibrary/src/Colormap-Defy.cpp
       # PluginLedLibrary/src/DefaultColormap.cpp
       # PluginLedLibrary/src/LEDEffect-BatteryStatus.cpp
       # PluginLedLibrary/src/LEDEffect-Bluetooth-Pairing-Defy.cpp
       # PluginLedLibrary/src/LEDEffect-BreatheDefy.cpp        
        PluginLedLibrary/src/LEDEffect-Rainbow-Defy.cpp
        PluginLedLibrary/src/LEDEffect-SolidColor-Defy.cpp
        PluginLedLibrary/src/LEDEffect-Stalker-Defy.cpp
        PluginLedLibrary/src/LED-Palette-Theme-Defy.cpp
        )

target_link_libraries(PluginLedLibrary
        INTERFACE
        )
