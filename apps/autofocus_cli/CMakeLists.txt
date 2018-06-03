project("autofocus_cli" CXX)

find_package(OpenCV REQUIRED)
link_libraries(${OpenCV_LIBS})

add_executable(${PROJECT_NAME} autofocus_cli.cpp)

target_compile_features(${PROJECT_NAME}
    PRIVATE
        cxx_auto_type
        cxx_nullptr
        cxx_range_for
)
