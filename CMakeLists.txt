cmake_minimum_required(VERSION 3.19)
project(CHAT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. 自动处理配置 ---
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 性能优化：限制 AUTOMOC 扫描范围，减小内存压力
set(CMAKE_AUTOMOC_RELAXED_MODE ON)

set(MY_CONFIG_FILE "config.json")
set(MY_STATIC_DIR "static")

# --- 2. 寻找包 ---
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Gui)
find_package(Boost REQUIRED COMPONENTS system)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(spdlog CONFIG REQUIRED)

qt_standard_project_setup()

# --- 3. 处理辅助文件 ---
file(GLOB_RECURSE UI_FILES "*.ui")

# --- 4. 自动编译 Proto (增强版) ---
set(PROTO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

# 动态获取插件路径，避免硬编码导致的环境差异
get_target_property(GRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin LOCATION)

set(PROTO_FILE "${PROTO_SRC_DIR}/chat.proto")
set(PROTO_GEN_FILES
    "${PROTO_GEN_DIR}/chat.pb.cc"
    "${PROTO_GEN_DIR}/chat.pb.h"
    "${PROTO_GEN_DIR}/chat.grpc.pb.cc"
    "${PROTO_GEN_DIR}/chat.grpc.pb.h"
)

# 使用确定的输出命令，确保依赖链条清晰
add_custom_command(
    OUTPUT ${PROTO_GEN_FILES}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${PROTO_GEN_DIR}
         --grpc_out=${PROTO_GEN_DIR}
         --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
         -I ${PROTO_SRC_DIR}
         ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Running C++ protocol buffer compiler on ${PROTO_FILE}"
    VERBATIM
)

# --- 5. 源文件清单 ---
# 建议：如果文件很多，手动列出虽然麻烦，但能加快 CMake 预处理速度
set(CPP_SOURCES
    common/Log.cpp
    main.cpp
    src/core/global.cpp
    src/core/ConfigMgr.cpp
    src/model/UserModels.cpp
    src/network/HttpMgr.cpp
    src/network/TcpMgr.cpp
    src/network/HeartBeatMgr.cpp
    src/service/UserMgr.cpp
    src/view/mainwindow/MainWindow.cpp
    src/view/login/LoginDialog.cpp
    src/view/login/RegisterDialog.cpp
    src/view/login/ResetDialog.cpp
    src/view/login/TimerBtn.cpp
    src/view/chat/ChatDialog.cpp
    src/view/chat/ChatPage.cpp
    src/view/chat/ChatView.cpp
    src/view/chat/ChatItemBase.cpp
    src/view/chat/BubbleFrame.cpp
    src/view/chat/TextBubble.cpp
    src/view/chat/PictureBubble.cpp
    src/view/chat/MessageTextEdit.cpp
    src/view/chat/CustomsizeEdit.cpp
    src/view/sidebar/ChatUserList.cpp
    src/view/sidebar/ChatUserWidget.cpp
    src/view/sidebar/AnimatedStateWidget.cpp
    src/view/sidebar/FriendLabel.cpp
    src/view/contacts/ContactUserList.cpp
    src/view/contacts/ConUserItem.cpp
    src/view/contacts/GroupTipItem.cpp
    src/view/contacts/FriendInfoPage.cpp
    src/view/contacts/FriendRequestDialog.cpp
    src/view/apply/ApplyFriendPage.cpp
    src/view/apply/ApplyFriendList.cpp
    src/view/apply/ApplyFriendItem.cpp
    src/view/apply/AddUserItem.cpp
    src/view/common/ListItemBase.cpp
    src/view/common/StatusDialog.cpp
    src/view/search/SearchList.cpp
    src/view/selfinfo/SelfInfomation.cpp
)

# --- 6. 构建目标 ---
qt_add_executable(CHAT
    ${CPP_SOURCES}
    ${UI_FILES}
    ${PROTO_GEN_FILES}
    resources.qrc
)

# 固定可执行文件输出到 build 根目录，与服务器保持一致
set_target_properties(CHAT PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
)

# --- 7. 后置处理 (配置文件与静态资源) ---
# 使用更现代的 POST_BUILD 方式
add_custom_command(TARGET CHAT POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/${MY_CONFIG_FILE}"
        "$<TARGET_FILE_DIR:CHAT>/${MY_CONFIG_FILE}"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:CHAT>/src/logdir"
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${MY_STATIC_DIR}")
    add_custom_command(TARGET CHAT POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/${MY_STATIC_DIR}"
            "$<TARGET_FILE_DIR:CHAT>/${MY_STATIC_DIR}"
    )
endif()

# --- 8. 包含路径与链接 ---
# 使用 SYSTEM 关键字标记生成代码目录，编译器会减少对这些巨型文件的警告处理，稍微提升速度
target_include_directories(CHAT SYSTEM PRIVATE ${PROTO_GEN_DIR})
target_include_directories(CHAT PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model
    ${CMAKE_CURRENT_SOURCE_DIR}/src/network
    ${CMAKE_CURRENT_SOURCE_DIR}/src/service
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/mainwindow
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/login
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/chat
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/contacts
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/apply
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/sidebar
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/search
    ${CMAKE_CURRENT_SOURCE_DIR}/src/view/selfinfo
)

target_link_libraries(CHAT
    PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Core
    Qt6::Gui
    Boost::system
    protobuf::libprotobuf
    gRPC::grpc++
    jsoncpp_lib  # 注意：在很多 Linux 发行版中，find_package(jsoncpp) 导出的目标是这个，请根据实际报错调整
    spdlog::spdlog
)