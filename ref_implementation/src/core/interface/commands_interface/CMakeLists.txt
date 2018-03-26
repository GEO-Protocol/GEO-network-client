# This file is part of GEO Project.
# It is subject to the license terms in the LICENSE.md file found in the top-level directory
# of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
#
# No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
# except according to the terms contained in the LICENSE.md file.

cmake_minimum_required(VERSION 3.6)

find_package(Boost COMPONENTS system REQUIRED)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/core/interface/)
set(SOURCE_FILES
        interface/CommandsInterface.cpp
        interface/CommandsInterface.h
        
        commands/BaseUserCommand.cpp
        commands/BaseUserCommand.h

        commands/trust_lines/SetOutgoingTrustLineCommand.h
        commands/trust_lines/SetOutgoingTrustLineCommand.cpp
        commands/trust_lines/CloseIncomingTrustLineCommand.h
        commands/trust_lines/CloseIncomingTrustLineCommand.cpp

        commands/payments/CreditUsageCommand.cpp
        commands/payments/CreditUsageCommand.h

        commands/max_flow_calculation/InitiateMaxFlowCalculationCommand.cpp
        commands/max_flow_calculation/InitiateMaxFlowCalculationCommand.h

        commands/total_balances/TotalBalancesCommand.cpp
        commands/total_balances/TotalBalancesCommand.h

        commands/history/HistoryPaymentsCommand.cpp
        commands/history/HistoryPaymentsCommand.h
        commands/history/HistoryTrustLinesCommand.cpp
        commands/history/HistoryTrustLinesCommand.h
        commands/history/HistoryWithContractorCommand.h
        commands/history/HistoryWithContractorCommand.cpp
        commands/history/HistoryAdditionalPaymentsCommand.h
        commands/history/HistoryAdditionalPaymentsCommand.cpp

        commands/trust_lines_list/GetFirstLevelContractorsCommand.h
        commands/trust_lines_list/GetFirstLevelContractorsCommand.cpp

        commands/trust_lines_list/GetTrustLinesCommand.h
        commands/trust_lines_list/GetTrustLinesCommand.cpp

        commands/trust_lines_list/GetTrustLineCommand.h
        commands/trust_lines_list/GetTrustLineCommand.cpp

        commands/subsystems_controller/SubsystemsInfluenceCommand.h
        commands/subsystems_controller/SubsystemsInfluenceCommand.cpp

        commands/blacklist/AddNodeToBlackListCommand.cpp
        commands/blacklist/AddNodeToBlackListCommand.h

        commands/blacklist/RemoveNodeFromBlackListCommand.h
        commands/blacklist/RemoveNodeFromBlackListCommand.cpp

        commands/blacklist/GetBlackListCommand.h
        commands/blacklist/GetBlackListCommand.cpp

        commands/blacklist/CheckIfNodeInBlackListCommand.h
        commands/blacklist/CheckIfNodeInBlackListCommand.cpp

        commands/transactions/PaymentTransactionByCommandUUIDCommand.h
        commands/transactions/PaymentTransactionByCommandUUIDCommand.cpp commands/max_flow_calculation/InitiateMaxFlowCalculationFullyCommand.cpp commands/max_flow_calculation/InitiateMaxFlowCalculationFullyCommand.h)

add_library(interface__commands ${SOURCE_FILES})
target_link_libraries(interface__commands
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_FILESYSTEM_LIBRARY}
        common)
