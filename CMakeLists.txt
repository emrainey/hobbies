cmake_minimum_required(VERSION 3.20.0)
project(hobbies
    VERSION 1.0.0
    DESCRIPTION "A repo full of hobby projects"
    LANGUAGES CXX
)
add_subdirectory(projects/basal)
add_subdirectory(projects/units_of_measure)
add_subdirectory(projects/fourcc)