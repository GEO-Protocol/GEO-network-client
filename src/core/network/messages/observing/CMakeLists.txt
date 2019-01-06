cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        ObservingRequestMessage.h
        ObservingRequestMessage.cpp
        ObservingRequestCheckMessage.h
        ObservingRequestCheckMessage.cpp
        ObservingTransactionCheckMessage.h
        ObservingTransactionCheckMessage.cpp
        ObservingParticipantsVotesMessage.h
        ObservingParticipantsVotesMessage.cpp )

add_library(messages__observing ${SOURCE_FILES})

target_link_libraries(messages__observing)