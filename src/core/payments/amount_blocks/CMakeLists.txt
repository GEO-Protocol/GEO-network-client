cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES AmountReservation.cpp AmountReservation.h AmountReservationsHandler.cpp AmountReservationsHandler.h)


add_library(amount_blocks ${SOURCE_FILES})
target_link_libraries(amount_blocks
        common)