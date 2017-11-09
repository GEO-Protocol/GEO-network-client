cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        AmountReservation.h
        AmountReservation.cpp
        AmountReservationsHandler.h
        AmountReservationsHandler.cpp)


add_library(reservations ${SOURCE_FILES})
target_link_libraries(reservations
        common)