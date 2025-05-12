add_library(Time_counter INTERFACE)

target_include_directories(Time_counter
        INTERFACE
        ./src)

target_sources(Time_counter
        INTERFACE
        ./src/Time_counter.c
        )

target_link_libraries(Time_counter
        INTERFACE
        )
