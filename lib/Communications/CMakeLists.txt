add_library(Communications INTERFACE)

target_include_directories(Communications
        INTERFACE
        Communications/src
        )

target_sources(Communications
        INTERFACE
        Communications/src/Communications_protocol_rf.cpp
        Communications/src/device/defyWN/CommunicationsWN.cpp
        )

target_link_libraries(Communications
        INTERFACE
        )
