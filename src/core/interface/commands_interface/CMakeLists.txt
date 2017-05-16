cmake_minimum_required(VERSION 3.6)

find_package(Boost COMPONENTS system REQUIRED)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/core/interface/)
set(SOURCE_FILES
        interface/CommandsInterface.cpp
        interface/CommandsInterface.h
        
        commands/BaseUserCommand.cpp
        commands/BaseUserCommand.h

        commands/trust_lines/OpenTrustLineCommand.cpp
        commands/trust_lines/OpenTrustLineCommand.h
        commands/trust_lines/CloseTrustLineCommand.cpp
        commands/trust_lines/CloseTrustLineCommand.h
        commands/trust_lines/SetTrustLineCommand.cpp
        commands/trust_lines/SetTrustLineCommand.h

        commands/payments/CreditUsageCommand.cpp
        commands/payments/CreditUsageCommand.h

        commands/max_flow_calculation/InitiateMaxFlowCalculationCommand.cpp
        commands/max_flow_calculation/InitiateMaxFlowCalculationCommand.h

        commands/total_balances/TotalBalancesCommand.cpp
        commands/total_balances/TotalBalancesCommand.h
        commands/total_balances/TotalBalancesRemouteNodeCommand.cpp
        commands/total_balances/TotalBalancesRemouteNodeCommand.h

        commands/history/HistoryPaymentsCommand.cpp
        commands/history/HistoryPaymentsCommand.h
        commands/history/HistoryTrustLinesCommand.cpp
        commands/history/HistoryTrustLinesCommand.h

        commands/trust_lines_list/GetFirstLevelContractorsCommand.h
        commands/trust_lines_list/GetFirstLevelContractorsCommand.cpp

        commands/trust_lines_list/GetTrustLinesCommand.h
        commands/trust_lines_list/GetTrustLinesCommand.cpp

        commands/find_path/FindPathCommand.h
        commands/find_path/FindPathCommand.cpp

        commands/cycle_closer/CycleCloserCommand.h
        commands/cycle_closer/CycleCloserCommand.cpp

        commands/routing_tables/UpdateRoutingTablesCommand.h
        commands/routing_tables/UpdateRoutingTablesCommand.cpp)

add_library(interface__commands ${SOURCE_FILES})
target_link_libraries(interface__commands
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
        common)
