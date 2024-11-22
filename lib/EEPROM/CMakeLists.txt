add_library(EEPROM INTERFACE)

target_include_directories(EEPROM
        INTERFACE
        ./src)

target_sources(EEPROM
        INTERFACE
        ./src/EEPROM.cpp
        )
