project (node-shared-buffer)
include_directories(${CMAKE_JS_INC} rpmalloc/rpmalloc jemalloc/include)
file(GLOB SOURCE_FILES "src")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")

set(
    SOURCE
    ${SOURCE}
        rpmalloc/rpmalloc/rpmalloc.c
        src/cpp/SharedMemory.cpp
        src/cpp/SharedMemory.h
        src/cpp/utils/SpinLock.h
        src/cpp/utils/Configuration.h
        src/cpp/utils/PointerUtils.h
        src/cpp/ipc/IPC.cpp
        src/cpp/ipc/IPC.h
    src/cpp/addon/addon.cpp
        src/cpp/addon/Buffer.cpp
    src/cpp/addon/Buffer.h
    src/cpp/boot.cpp
        src/cpp/utils/Logger.h
        src/cpp/utils/Logger.cpp
)

set(ADDON_SOURCE ${SOURCE})
set(BOOT_SOURCE ${SOURCE} src/cpp/addon/Types.h src/cpp/allocator/STL.h src/cpp/allocator/STL.cpp src/cpp/addon/Utils.cpp src/cpp/addon/Utils.h src/cpp/addon/Store.cpp src/cpp/addon/Store.h src/cpp/platform/Unix.cpp src/cpp/platform/Unix.h src/cpp/platform/Platform.h src/cpp/addon/IPC_Caller.cpp src/cpp/addon/IPC_Caller.h src/cpp/addon/IPC_Callee.cpp src/cpp/addon/IPC_Callee.h src/cpp/ipc/Message.cpp src/cpp/ipc/Message.h src/cpp/ipc/Worker.h src/cpp/ipc/Worker.cpp src/cpp/allocator/Allocator.cpp src/cpp/allocator/Allocator.h src/cpp/allocator/PoolAllocator.cpp src/cpp/allocator/PoolAllocator.h src/cpp/utils/LinkedList.h src/cpp/utils/Configuration.cpp)
link_directories("deps")


add_executable(boot ${BOOT_SOURCE})
add_library(${PROJECT_NAME} SHARED ${ADDON_SOURCE})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})
target_link_libraries(boot node.64.dylib)