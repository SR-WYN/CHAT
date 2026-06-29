add_library(common INTERFACE)

target_include_directories(common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/constants
)

target_link_libraries(common INTERFACE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Gui
    jsoncpp_lib
    spdlog::spdlog
)