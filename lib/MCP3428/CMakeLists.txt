add_library(MCP3428 INTERFACE)

target_include_directories(MCP3428
        INTERFACE
        ./src)

target_sources(MCP3428
        INTERFACE
        ./src/MCP3428.cpp
        )

target_link_libraries(MCP3428
        INTERFACE
        )
