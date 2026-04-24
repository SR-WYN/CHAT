cmake_minimum_required(VERSION 3.19)
project(CHAT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON) # 建议加上，处理 .ui 文件
set(CMAKE_AUTOMOC ON) # 建议加上，处理 Qt 信号槽
set(CMAKE_AUTORCC ON)
set(MY_CONFIG_FILE "config.json")

# --- 1. 寻找包 ---
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)
# Beast 依赖 system，如果用到多线程可能还需要 thread
find_package(Boost REQUIRED COMPONENTS system)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(jsoncpp REQUIRED)

qt_standard_project_setup()

# --- 2. 处理辅助文件 ---
file(GLOB_RECURSE UI_FILES "*.ui")
file(GLOB_RECURSE HEADERS "*.h" "*.hpp")

# --- 3. 自动编译 Proto (更通用的写法) ---
set(PROTO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

# 自动获取 grpc_cpp_plugin 的位置，不要硬编码 /usr/bin
get_target_property(GRPC_CPP_PLUGIN_EXECUTABLE gRPC::grpc_cpp_plugin LOCATION)

set(PROTO_FILE "${PROTO_SRC_DIR}/chat.proto")
set(PROTO_GEN_FILES
    "${PROTO_GEN_DIR}/chat.pb.cc"
    "${PROTO_GEN_DIR}/chat.pb.h"
    "${PROTO_GEN_DIR}/chat.grpc.pb.cc"
    "${PROTO_GEN_DIR}/chat.grpc.pb.h"
)

# --- 4. 源文件 ---
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
    # 如果 resources.qrc 没加入到下面的可执行文件里，也可以放这
)

# --- 5. 构建 ---
qt_add_executable(CHAT
    ${CPP_SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${PROTO_GEN_FILES}
    resources.qrc
)

add_custom_command(
    OUTPUT ${PROTO_GEN_FILES}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${PROTO_GEN_DIR}
    --grpc_out=${PROTO_GEN_DIR}
    --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN_EXECUTABLE}
    -I ${PROTO_SRC_DIR}
    ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/${MY_CONFIG_FILE}"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${MY_CONFIG_FILE}"
    COMMENT "Copying config.json to execution directory..."
)

# --- 6. 链接与配置 ---
# 必须把生成的目录加入包含路径，否则代码里 #include "chat.pb.h" 会报错
target_include_directories(CHAT PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROTO_GEN_DIR}
)

target_link_libraries(CHAT
    PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Core
    Boost::system # 建议链接 system
    protobuf::libprotobuf
    gRPC::grpc++ # 通常 C++ 开发主要链接 grpc++
    JsonCpp::JsonCpp # 注意大小写
)
