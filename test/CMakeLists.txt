file(GLOB_RECURSE ArduinoJStream_SRC "${CMAKE_SOURCE_DIR}/lib/ArduinoJStream/*.cpp")

file(GLOB_RECURSE Hueduino_SRC "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM Hueduino_SRC "${CMAKE_SOURCE_DIR}/src/main.cpp")

file(GLOB_RECURSE MockArduino_SRC "${CMAKE_SOURCE_DIR}/test/MockArduino/*.cpp" "${CMAKE_SOURCE_DIR}/lib/MockArduinoNative/*.cpp")

set(CMAKE_BUILD_TYPE DEBUG)

#############
# Libraries #
#############

add_library(MockArduino ${MockArduino_SRC})
target_include_directories(MockArduino PUBLIC ${CMAKE_SOURCE_DIR}/test/MockArduino ${CMAKE_SOURCE_DIR}/lib/MockArduinoNative) # MockArduino Headers

add_library(ArduinoJStream ${ArduinoJStream_SRC})
target_link_libraries(ArduinoJStream MockArduino)
target_include_directories(ArduinoJStream PUBLIC ${CMAKE_SOURCE_DIR}/lib/ArduinoJStream) # Lib Headers

add_library(Catch2 tests-main.cpp)

add_library(Hueduino ${Hueduino_SRC})
target_link_libraries(Hueduino MockArduino ArduinoJStream)
target_include_directories(Hueduino PUBLIC ${CMAKE_SOURCE_DIR}/src) # Src Headers

###############
# Executables #
###############

add_executable(TestGroup TestGroup.cpp)
target_link_libraries(TestGroup Hueduino Catch2)

add_executable(TestScene TestScene.cpp)
target_link_libraries(TestScene Hueduino Catch2)

add_executable(TestGroupStream TestGroupStream.cpp)
target_link_libraries(TestGroupStream Hueduino Catch2)

add_executable(TestSceneStream TestSceneStream.cpp)
target_link_libraries(TestSceneStream Hueduino Catch2)

###########
# Targets #
###########

add_custom_target(testGroup ./TestGroup -d yes DEPENDS TestGroup)
add_custom_target(testScene ./TestScene -d yes DEPENDS TestScene)
add_custom_target(testGroupStream ./TestGroupStream -d yes DEPENDS TestGroupStream)
add_custom_target(testSceneStream ./TestSceneStream -d yes DEPENDS TestSceneStream)