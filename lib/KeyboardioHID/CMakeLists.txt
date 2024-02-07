add_library(KeyboardioHID INTERFACE)

target_include_directories(KeyboardioHID
        INTERFACE
        KeyboardioHID/src
        KeyboardioHID/src/MultiReport
        )

target_sources(KeyboardioHID
        INTERFACE
        KeyboardioHID/src/HID.cpp
        KeyboardioHID/src/HIDReportObserver.cpp
        KeyboardioHID/src/MultiReport/ConsumerControl.cpp
        KeyboardioHID/src/MultiReport/Keyboard.cpp
        KeyboardioHID/src/MultiReport/Mouse.cpp
        KeyboardioHID/src/MultiReport/SystemControl.cpp
        )

target_link_libraries(KeyboardioHID
        INTERFACE
        )
