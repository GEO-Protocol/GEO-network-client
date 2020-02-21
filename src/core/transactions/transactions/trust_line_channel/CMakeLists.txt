cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES

    InitChannelTransaction.h
    InitChannelTransaction.cpp
    ConfirmChannelTransaction.h
    ConfirmChannelTransaction.cpp

    GetContractorListTransaction.h
    GetContractorListTransaction.cpp
    GetChannelInfoTransaction.h
    GetChannelInfoTransaction.cpp
    GetChannelInfoByAddressesTransaction.h
    GetChannelInfoByAddressesTransaction.cpp

    UpdateChannelAddressesInitiatorTransaction.h
    UpdateChannelAddressesInitiatorTransaction.cpp
    UpdateChannelAddressesTargetTransaction.h
    UpdateChannelAddressesTargetTransaction.cpp
    SetChannelContractorAddressesTransaction.h
    SetChannelContractorAddressesTransaction.cpp
    SetChannelContractorCryptoKeyTransaction.h
    SetChannelContractorCryptoKeyTransaction.cpp
    RegenerateChannelCryptoKeyTransaction.h
    RegenerateChannelCryptoKeyTransaction.cpp

    RemoveChannelTransaction.h
    RemoveChannelTransaction.cpp)

add_library(transactions__channels ${SOURCE_FILES})
target_link_libraries(transactions__channels
    transactions__base
    common
    exceptions)
