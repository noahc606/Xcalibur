### Project Settings ###
# Set min cmake version
cmake_minimum_required(VERSION 3.5)
set(PROJ_NAME "Xcalibur-test")

# Set executable target depending on OS...
if(WIN32)
    message("[NCH] Building Windows executable...")
    set(PROJ_OUT "${PROJ_NAME}-win-x86_64")
    # Show console (FALSE=show, TRUE=don't show)
    set(CMAKE_WIN32_EXECUTABLE TRUE)
endif()
if(UNIX)
    EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
    message("[NCH] Building unix-${ARCHITECTURE} executable...")
    set(PROJ_OUT "${PROJ_NAME}-unix-${ARCHITECTURE}")
endif()

# Project languages and flags
project(${PROJ_OUT} VERSION 0.1.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_FLAGS "-std=c++11")


### Create List of Sources, Headers, and Directories ###
# Get list of sources and headers
file (GLOB_RECURSE PROJ_SOURCES FOLLOW_SYMLINKS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/include/*.cpp" "${CMAKE_SOURCE_DIR}/include/*.c")
file (GLOB_RECURSE PROJ_HEADERS FOLLOW_SYMLINKS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.h" "${CMAKE_SOURCE_DIR}/include/*.hpp" "${CMAKE_SOURCE_DIR}/include/*.h")
# Build list of source directories based off of globbed headers
set (PROJ_SRC_DIRS "")
foreach (_headerFile ${PROJ_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND PROJ_SRC_DIRS ${_dir})
endforeach()
list (REMOVE_DUPLICATES PROJ_SRC_DIRS)


### Create Executable and add Links/Includes/DLLs ###
# Add executable
add_executable(${PROJ_OUT} ${PROJ_SOURCES})
set_target_properties(${PROJ_OUT} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)
# Add libraries + link them into target
target_link_libraries(${PROJ_OUT} PUBLIC "-lX11 -lXext -lxcb -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lQt5Widgets -lQt5Gui -lQt5Core")
get_target_property(TARGET_LIBS ${PROJ_OUT} LINK_LIBRARIES)
message("[NCH] Linked libraries: ${TARGET_LIBS}")
# Add include directories (everywhere there is a header file: libraries and PROJ src dirs)
target_include_directories(${PROJ_OUT} PRIVATE ${PROJ_SRC_DIRS})
target_include_directories(${PROJ_OUT} PRIVATE "include")
target_include_directories(${PROJ_OUT} PRIVATE "/usr/include/x86_64-linux-gnu/qt5")
target_include_directories(${PROJ_OUT} PRIVATE "/usr/include/x86_64-linux-gnu/qt5/QtGui")
target_include_directories(${PROJ_OUT} PRIVATE "/usr/include/x86_64-linux-gnu/qt5/QtWidgets")

message("[NCH] Finished config for ${PROJ_OUT}")
