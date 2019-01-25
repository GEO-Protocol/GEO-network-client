cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TestIncludes.h

        interface/сommands_interface/commands/history/HistoryAdditionalPaymentsCommandTest.cpp
        interface/сommands_interface/commands/history/HistoryPaymentsCommandTest.cpp
        interface/сommands_interface/commands/history/HistoryTrustLinesCommandTest.cpp
        interface/сommands_interface/commands/history/HistoryWithContractorCommandTest.cpp

        interface/сommands_interface/commands/max_flow_calculation/InitiateMaxFlowCalculationCommandTest.cpp
        interface/сommands_interface/commands/max_flow_calculation/InitiateMaxFlowCalculationFullyCommandTest.cpp

        interface/сommands_interface/commands/payments/CreditUsageCommandTest.cpp

        interface/сommands_interface/commands/total_balances/TotalBalancesCommandTest.cpp

        interface/сommands_interface/commands/transactions/PaymentTransactionByCommandUUIDCommandTest.cpp

        interface/сommands_interface/commands/trust_lines_list/GetFirstLevelContractorsCommandTest.cpp
        interface/сommands_interface/commands/trust_lines_list/GetTrustLineCommandTest.cpp
        interface/сommands_interface/commands/trust_lines_list/GetTrustLinesCommandTest.cpp

        interface/сommands_interface/commands/subsystems_controller/SubsystensInfluenceCommandTest.cpp
        interface/сommands_interface/commands/subsystems_controller/TrustLinesInfluenceCommandTest.cpp

        interface/сommands_interface/commands/trust_lines/CloseIncTrustLineCommandTest.cpp
        interface/сommands_interface/commands/trust_lines/InitTrustLineTest.cpp
        interface/сommands_interface/commands/trust_lines/SetOutgoingTrustLineCommandTest.cpp
        interface/сommands_interface/commands/trust_lines/ShareKeysCommandTest.cpp
    )
