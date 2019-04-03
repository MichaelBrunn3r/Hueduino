cmake_minimum_required(VERSION 3.0)
project(Hueduino)

add_subdirectory(test)

add_custom_target(updateMockArduino
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test
    
    COMMAND rm -R -f MockArduino
    COMMAND rm -R -f ${CMAKE_SOURCE_DIR}/lib/MockArduinoNative

    COMMAND curl -s https://api.github.com/repos/MichaelBrunn3r/MockArduino/releases/latest 
                | grep "browser_download_url" 
                | cut -d : -f 2,3 
                | tr -d \\\" | wget -qi -
    COMMAND unzip MockArduino.zip
    COMMAND rm MockArduino.zip
    
    COMMAND mv MockArduino/native MockArduino/MockArduinoNative
    COMMAND mv MockArduino/MockArduinoNative ${CMAKE_SOURCE_DIR}/lib
)

add_custom_target(updateJStream
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/lib
    COMMAND rm -f -R ArduinoJStream
    COMMAND curl -s https://api.github.com/repos/MichaelBrunn3r/ArduinoJStream/releases/latest 
                | grep "browser_download_url" 
                | cut -d : -f 2,3 
                | tr -d \\\" | wget -qi -
    COMMAND unzip ArduinoJStream.zip
    COMMAND rm ArduinoJStream.zip
)

add_custom_target(updateDependencies DEPENDS updateJStream updateMockArduino)

add_custom_target(createRelease
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

    COMMAND rm -f Hueduino.zip
    COMMAND cp -R src Hueduino
    COMMAND cp LICENSE Hueduino
    COMMAND zip -r Hueduino.zip Hueduino
    COMMAND rm -r Hueduino
)