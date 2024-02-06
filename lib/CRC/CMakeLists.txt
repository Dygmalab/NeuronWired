add_library(CRC INTERFACE)

target_include_directories(CRC
        INTERFACE
        ./src)

target_sources(CRC
        INTERFACE
        ./src/CRC.cpp
        )

target_link_libraries(CRC
        INTERFACE
        hardware_dma
        )
