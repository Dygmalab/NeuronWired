add_library(Communications INTERFACE)

target_include_directories(Communications
        INTERFACE
        Communications/src
        Communications/src/device/neuron/common
        Communications/src/device/neuron/defyWN
        Communications/src/device/neuron/defyWN/com_models
        )

target_sources(Communications
        INTERFACE
        Communications/src/device/neuron/common/Communication_model.cpp
        Communications/src/device/neuron/common/Communications_side.cpp
        Communications/src/device/neuron/defyWN/com_models/Communication_model_wired.cpp
        Communications/src/device/neuron/defyWN/CommunicationsWN.cpp
        Communications/src/device/neuron/defyWN/CommunicationsWN_side.cpp
        )

target_link_libraries(Communications
        INTERFACE
        )
