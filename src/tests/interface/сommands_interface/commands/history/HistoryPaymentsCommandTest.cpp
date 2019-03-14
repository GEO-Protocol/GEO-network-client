#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/history/HistoryPaymentsCommand.h"

TEST_CASE("Testing HistoryPaymentsCommand")
{
    REQUIRE_NOTHROW(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

    SECTION("No input")
    {
        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));
    }

    SECTION("Characters instead of input")
    {
        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "asdsasa"));
    }

    SECTION("Lost delimeter")
    {
        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\2\t3\t4\t5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\3\t4\t5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\4\t5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\8\n"));
    }

    SECTION("UUID 7-4-4-12 -> 8-3-4-12 -> 8-4-3-12 -> 8-4-4-11 -> 4-4-12 -> 9-4-4-12 -> 8-5-4-12 -> 8-4-5-12 -> 8-4-4-13")
    {
        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t4718323-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-257-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-2574-4bfd-b11-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-2574-4bfd-b411-99ed177d343\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t471838273-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-25574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-2574-4bfd-b4511-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-25574-4bfd-b411-99ed177dd3e43\t8\n"));
    }

    SECTION("Max number in input")
    {
        REQUIRE_NOTHROW(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t2\t3\t4\t340282366920938463463374607431768211456\t6\t47183823-2574-4bfd-b411-99ed177dd3e4\t8\n"));

        REQUIRE_NOTHROW(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t2\t3\t4\t5\t340282366920938463463374607431768211456\t47183823-2574-4bfd-b411-99ed177de435\t8\n"));
    }

    SECTION("Overflow in input (40 digits)")
    {
        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t2\t3\t4\t3402823669209384634633746074317682114560\t6\t47183823-2574-4bfd-b411-99ed177de43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t2\t3\t4\t5\t3402823669209384634633746074317682114560\t47183823-2574-4bfd-b411-99ed177d343\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t2\t3\t4\t3402823669209384634633746074317682114560\t3402823669209384634633746074317682114560\t47183823-2574-4bfd-b411-99ed17dde463\t8\n"));
    }
}

