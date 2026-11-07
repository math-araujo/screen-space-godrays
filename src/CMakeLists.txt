# the subdirectory gl contains a small library abstracting
# OpenGL calls for this project
add_subdirectory(gl)
add_executable(main 
    main.cpp
    main_application.hpp main_application.cpp
)
target_compile_features(main PRIVATE cxx_std_23)
target_link_libraries(main PRIVATE gl)
set_target_properties(main PROPERTIES CXX_EXTENSIONS OFF)