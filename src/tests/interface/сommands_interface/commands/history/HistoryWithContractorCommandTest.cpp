#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/history/HistoryWithContractorCommand.h"

TEST_CASE("Testing HistoryWithContractorCommand")
{
    REQUIRE_NOTHROW(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\t127.0.0.1:2007\t2\n"));

    SECTION("Two addresses")
    {
        REQUIRE_NOTHROW(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t2\t12\t127.0.0.1:2007\t12\t127.0.0.1:2007\t2\n"));
    }

    SECTION("Double separator")
    {
        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t\t1\t1\t12\t127.0.0.1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t\t1\t12\t127.0.0.1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t\t12\t127.0.0.1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\t\t127.0.0.1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\t127.0.0.1:2007\t\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\t\t1\t1\t12\t127.0.0.1:2007\t2\n"));
    }

    SECTION("Second address whithout type")
    {
        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t2\t12\t127.0.0.1:2007\t127.0.0.1:2007\t2\n"));
    }

    SECTION("Address number more than amount of addresses")
    {
        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t4\t12\t127.0.0.1:2007\t2\n"));
    }

    SECTION("Character instead of integer")
    {
        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"s\t1\t1\t12\t127.0.0.1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0s1\t1\t12\t127.0.0.1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1s1\t12\t127.0.0.1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\ts\t12\t127.0.0.1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\ts\t127.0.0.1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12s127.0.0.1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\ts.0.0.1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\t127s0.0.1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\t127.s.0.1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\t127.0s0.1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\t127.0.s.1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\t127.0.0s1:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\t127.0.0.s:2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\t127.0.0.1s2007\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\t127.0.0.1:s\t2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\t127.0.0.1:2007s2\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\t127.0.0.1:2007\ts\n"));
    }

    SECTION("No input")
    {
        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,""));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\t"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\t\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\t\t"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\n\n"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\n\t"));
    }

    SECTION("Character instead of command & after EOL")
    {
        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"sdfdsf"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\t127.0.0.1:2007\t2\n\t"));

        REQUIRE_THROWS(HistoryWithContractorCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"0\t1\t1\t12\t127.0.0.1:2007\t2\nsdfs"));
    }
}