cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        ObservingMessage.hpp

        ObservingClaimAppendRequestMessage.h
        ObservingClaimAppendRequestMessage.cpp
        ObservingClaimAppendResponseMessage.h
        ObservingClaimAppendResponseMessage.cpp

        ObservingParticipantsVotesRequestMessage.h
        ObservingParticipantsVotesRequestMessage.cpp
        ObservingParticipantsVotesResponseMessage.h
        ObservingParticipantsVotesResponseMessage.cpp

        ObservingTransactionsRequestMessage.h
        ObservingTransactionsRequestMessage.cpp
        ObservingTransactionsResponseMessage.h
        ObservingTransactionsResponseMessage.cpp

        ObservingBlockNumberRequest.h
        ObservingBlockNumberRequest.cpp
        ObservingBlockNumberResponse.h
        ObservingBlockNumberResponse.cpp

        ObservingParticipantsVotesAppendRequestMessage.h
        ObservingParticipantsVotesAppendRequestMessage.cpp
        ObservingParticipantsVotesAppendResponseMessage.h
        ObservingParticipantsVotesAppendResponseMessage.cpp)

add_library(observing__messages ${SOURCE_FILES})

target_link_libraries(observing__messages)