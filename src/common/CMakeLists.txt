add_library(common STATIC
    utils/utils.cpp
)

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/constants
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

target_link_libraries(common PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Gui
    jsoncpp_lib
    spdlog::spdlog
)