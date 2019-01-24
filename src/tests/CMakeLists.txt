cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        TestIncludes.h

        interface/сommands_interface/commands/history/HistoryAdditionalPaymentsCommandTest.cpp
        interface/сommands_interface/commands/history/HistoryPaymentsCommandTest.cpp
        interface/сommands_interface/commands/history/HistoryTrustLinesCommandTest.cpp
        interface/сommands_interface/commands/history/HistoryWithContractorCommandTest.cpp

        interface/сommands_interface/commands/trust_lines/CloseIncomingTrustLineCommandTest.cpp
        interface/сommands_interface/commands/trust_lines/InitTrustLineTest.cpp
        interface/сommands_interface/commands/trust_lines/SetOutgoingTrustLineCommandTest.cpp
        interface/сommands_interface/commands/trust_lines/ShareKeysCommandTest.cpp
    )
