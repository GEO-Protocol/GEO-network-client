#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/history/HistoryTrustLinesCommand.h"

TEST_CASE("Testing HistoryTrustLinesCommand")
{
    REQUIRE_NOTHROW(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\n"));

    SECTION("No input")
    {
        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));
    }

    SECTION("Characters instead of integers")
    {
        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a\t2\t3\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\ta\t3\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\ta\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\ta\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\ta\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "dsdfsdfs"));
    }

    SECTION("Overflow of integers")
    {
        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "99999999999999999\t2\t3\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t99999999999999999\t3\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t99999999999999999\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t99999999999999999\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t99999999999999999\n"));
    }

    SECTION("Float instead of integer")
    {
        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1.2\t2\t3\t4\t5\n"));
    }

    SECTION("Lost delimiter")
    {
        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\2\t3\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\3\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2.3\t3\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3.9\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4.8\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5.22\n"));
    }
}
