add_executable(main main.cpp)
target_compile_features(main PRIVATE cxx_std_20)
set_target_properties(main PROPERTIES CXX_EXTENSIONS OFF)