project (node-shared-buffer)
include_directories(${CMAKE_JS_INC})
file(GLOB SOURCE_FILES "src")
add_library(
    ${PROJECT_NAME}
    SHARED
    ${SOURCE_FILES}
        src/main.cpp
        src/Exceptions.h
        src/PosixSharedBuffer.cpp
        src/PosixSharedBuffer.h
)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})

