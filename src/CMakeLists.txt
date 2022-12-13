# the subdirectory gl contains a small library abstracting
# OpenGL calls for this project
add_subdirectory(gl)
add_executable(main 
    main.cpp
    main_application.hpp main_application.cpp
)
target_compile_features(main PRIVATE cxx_std_20)
target_link_libraries(main PRIVATE gl)
set_target_properties(main PROPERTIES CXX_EXTENSIONS OFF)

add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:main>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:main>/assets
)