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
    main.cpp
    MainWindow.cpp
    LoginDialog.cpp
    RegisterDialog.cpp
    global.cpp
    HttpMgr.cpp
    ConfigMgr.cpp
    TimerBtn.cpp
    ClickedLabel.cpp
    ResetDialog.cpp
    TcpMgr.cpp
    UserMgr.cpp
    ChatDialog.cpp
    ClickedBtn.cpp
    CustomsizeEdit.cpp
    ChatUserList.cpp
    ChatUserWidget.cpp
    ListItemBase.cpp
    LoadingDialog.cpp
    ChatPage.cpp
    ChatView.cpp
    BubbleFrame.cpp
    ChatItemBase.cpp
    MessageTextEdit.cpp
    TextBubble.cpp
    PictureBubble.cpp
    StateWidget.cpp
    SearchList.cpp
    UserData.cpp
    AddUserItem.cpp
    FindSuccessDialog.cpp
    ApplyFriend.cpp
    ApplyFriendPage.cpp
    ClickedOnceLabel.cpp
    FriendLabel.cpp
    ConUserItem.cpp
    GroupTipItem.cpp
    ContactUserList.cpp
    ApplyFriendItem.cpp
    ApplyFriendList.cpp
    FindFailDialog.cpp
)

# --- 6. 构建目标 ---
qt_add_executable(CHAT
    ${CPP_SOURCES}
    ${UI_FILES}
    ${PROTO_GEN_FILES}
    resources.qrc
)

# --- 7. 后置处理 (配置文件与静态资源) ---
# 使用更现代的 POST_BUILD 方式
add_custom_command(TARGET CHAT POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/${MY_CONFIG_FILE}"
        "$<TARGET_FILE_DIR:CHAT>/${MY_CONFIG_FILE}"
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
target_include_directories(CHAT PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

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
)