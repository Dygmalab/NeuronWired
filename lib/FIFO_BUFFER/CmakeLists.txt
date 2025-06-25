add_library(FIFO_BUFFER INTERFACE)

target_include_directories(FIFO_BUFFER
        INTERFACE
        ./src)

target_sources(FIFO_BUFFER
        INTERFACE
        ./src/Fifo_buffer.cpp
        )
