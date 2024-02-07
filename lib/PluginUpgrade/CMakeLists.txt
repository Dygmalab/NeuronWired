add_library(PluginUpgrade INTERFACE)

target_include_directories(PluginUpgrade
        INTERFACE
		PluginUpgrade/include
		PluginUpgrade/src
        )

target_sources(PluginUpgrade
        INTERFACE
        PluginUpgrade/src/KeyScannerFlasher.cpp
        PluginUpgrade/src/Upgrade.cpp
        )

target_link_libraries(PluginUpgrade
        INTERFACE
        )
