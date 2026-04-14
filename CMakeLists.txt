cmake_minimum_required(VERSION 3.19)
project(CHAT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

# 1. 寻找包
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Boost REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(jsoncpp REQUIRED)

qt_standard_project_setup()

# --- 2. 自动处理辅助文件 (非 CPP 部分) ---
# 自动搜集头文件和 UI 文件（这些通常不需要手动精确控制）
file(GLOB_RECURSE UI_FILES "*.ui")
file(GLOB_RECURSE HEADERS "*.h" "*.hpp")

# --- 3. 自动编译 Proto ---
set(PROTO_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${PROTO_GEN_DIR})

set(PROTO_FILE "${PROTO_SRC_DIR}/chat.proto")
set(PROTO_GEN_FILES
    "${PROTO_GEN_DIR}/chat.pb.cc"
    "${PROTO_GEN_DIR}/chat.pb.h"
    "${PROTO_GEN_DIR}/chat.grpc.pb.cc"
    "${PROTO_GEN_DIR}/chat.grpc.pb.h"
)

add_custom_command(
    OUTPUT ${PROTO_GEN_FILES}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${PROTO_GEN_DIR}
         --grpc_out=${PROTO_GEN_DIR}
         --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin
         -I ${PROTO_SRC_DIR}
         ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
)

# --- 4. 手动管理 CPP 源文件 ---
# 这里是你唯一需要手动维护的地方
set(CPP_SOURCES
    main.cpp
    mainwindow.cpp
    LoginDialog.cpp
    RegisterDialog.cpp
    global.cpp
    # 以后新加的类，只需把 .cpp 往这丢即可
)

# --- 5. 构建可执行文件 ---
qt_add_executable(CHAT
    WIN32 MACOSX_BUNDLE
    ${CPP_SOURCES}      # 手动控制的核心逻辑
    ${HEADERS}          # 自动扫描的头文件
    ${UI_FILES}         # 自动扫描的界面文件
    ${PROTO_GEN_FILES}   # 自动生成的代码
    resources.qrc          # 资源文件
)

# --- 6. 链接与配置 ---
target_include_directories(CHAT PRIVATE ${PROTO_GEN_DIR})

target_link_libraries(CHAT
    PRIVATE
        Qt::Core
        Qt::Widgets
        Boost::boost
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
        jsoncpp_lib
)