#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/history/HistoryTrustLinesCommand.h"

TEST_CASE("Testing HistoryTrustLinesCommand")
{
    REQUIRE_NOTHROW(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\n"));

    SECTION("No input & character after EOL")
    {
        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\t"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\n\t"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\ndfs"));
    }

    SECTION("Double separator")
    {
        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t\t2\t3\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t\t3\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t2\t3\t4\t5\n"));
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

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2.2\t3\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3.2\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4.3\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5.3\n"));
    }

    SECTION("Lost delimiter")
    {
        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\2\t3\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\3\t4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\4\t5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\5\n"));

        REQUIRE_THROWS(HistoryTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5"));
    }
}
