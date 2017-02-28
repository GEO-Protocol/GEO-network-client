cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        RoutingTablesTransaction.cpp
        RoutingTablesTransaction.h
        propagate/FromInitiatorToContractorRoutingTablePropagationTransaction.cpp
        propagate/FromInitiatorToContractorRoutingTablePropagationTransaction.h
        accept/FromInitiatorToContractorRoutingTablesAcceptTransaction.cpp
        accept/FromInitiatorToContractorRoutingTablesAcceptTransaction.h
        propagate/FromContractorToFirstLevelRoutingTablePropagationTransaction.cpp
        propagate/FromContractorToFirstLevelRoutingTablePropagationTransaction.h
        accept/FromContractorToFirstLevelRoutingTableAcceptTransaction.cpp
        accept/FromContractorToFirstLevelRoutingTableAcceptTransaction.h
        propagate/FromFirstLevelToSecondLevelRoutingTablePropagationTransaction.cpp
        propagate/FromFirstLevelToSecondLevelRoutingTablePropagationTransaction.h
        accept/FromFirstLevelToSecondLevelRoutingTableTransaction.cpp
        accept/FromFirstLevelToSecondLevelRoutingTableTransaction.h)

add_library(transactions__routing_tables ${SOURCE_FILES})
target_link_libraries(transactions__routing_tables
        transactions__base

        common
        time
        exceptions

        network__messages)
