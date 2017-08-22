INCLUDE(../../../cmake/macros.cmake)

SET(ROBOT_NAME "LaneDriver")
SET(ROBOT_INTERFACE LEGACY_MIN)

SET(ROBOT_SOURCES
        ${ROBOT_NAME}.cpp
        CLaneDriver.cpp
        CLaneDriver.h
        )

set (CMAKE_CXX_STANDARD 11)

ADD_SDLIB_INCLUDEDIR(robottools tgfclient)

ROBOT_MODULE(NAME ${ROBOT_NAME} VERSION 1.0.0 SOVERSION 1.0.0
             INTERFACE ${ROBOT_INTERFACE}
             SOURCES ${ROBOT_SOURCES})

ADD_SDLIB_LIBRARY(${ROBOT_NAME} robottools tgfclient)

# For data associated to the robot module, see data/drivers/human/CMakeLists.txt

