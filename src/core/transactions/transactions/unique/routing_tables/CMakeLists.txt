cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        RoutingTablesTransaction.cpp
        RoutingTablesTransaction.h
        propagate/FromInitiatorToContractorRoutingTablesPropagationTransaction.cpp
        propagate/FromInitiatorToContractorRoutingTablesPropagationTransaction.h
        accept/FromInitiatorToContractorRoutingTablesAcceptTransaction.cpp
        accept/FromInitiatorToContractorRoutingTablesAcceptTransaction.h
        propagate/FromContractorToFirstLevelRoutingTablesPropagationTransaction.cpp
        propagate/FromContractorToFirstLevelRoutingTablesPropagationTransaction.h
        accept/FromContractorToFirstLevelRoutingTablesAcceptTransaction.cpp
        accept/FromContractorToFirstLevelRoutingTablesAcceptTransaction.h
        propagate/FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction.cpp
        propagate/FromFirstLevelToSecondLevelRoutingTablesPropagationTransaction.h
        accept/FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction.cpp
        accept/FromFirstLevelToSecondLevelRoutingTablesAcceptTransaction.h)

add_library(transactions__routing_tables ${SOURCE_FILES})
target_link_libraries(transactions__routing_tables
        transactions__base

        common
        time
        exceptions

        network__messages)
