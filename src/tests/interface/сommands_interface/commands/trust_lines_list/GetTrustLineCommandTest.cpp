#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines_list/GetTrustLineByAddressCommand.h"

TEST_CASE("Testing GetTrustLineCommand")
{
    GetTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "12\t127.0.0.1:2006\t2000\n");

    SECTION("Charater instead of int in number of addresses")
    {
        REQUIRE_THROWS(GetTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1s\t127.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(GetTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12w127.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(GetTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\ts.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(GetTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\t127.s.0.1:2007\t3\n"));

        REQUIRE_THROWS(GetTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\t127.0.s.1:2007\t3\n"));

        REQUIRE_THROWS(GetTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\t127.0.0.s:2007\t3\n"));

        REQUIRE_THROWS(GetTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\t127.0.0.0s2007\t3\n"));

        REQUIRE_THROWS(GetTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\t127.0.0.1:s\t3\n"));

        REQUIRE_THROWS(GetTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\t127.0.0.1:2007\ts\n"));

        REQUIRE_THROWS(GetTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\t127.0.0.1:2007!3\n"));

        REQUIRE_THROWS(GetTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12!127.0.0.1:2007!3\n"));
    }

    SECTION("No input")
    {
        REQUIRE_THROWS(GetTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,""));

        REQUIRE_THROWS(GetTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\n"));
    }

    SECTION("Characters instead of input")
    {
        REQUIRE_THROWS(GetTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"dfsfdfsf\n"));
    }
}