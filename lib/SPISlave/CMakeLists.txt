add_library(SPISlave INTERFACE)

target_include_directories(SPISlave
        INTERFACE
        ./src)

target_sources(SPISlave
        INTERFACE
        ./src/Spi_slave.cpp
        ./src/SpiPort.cpp
        )

target_link_libraries(SPISlave
        INTERFACE
        )
