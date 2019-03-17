#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines_list/GetTrustLineByAddressCommand.h"

TEST_CASE("Testing GetTrustLineByAddressCommand")
{
    REQUIRE_NOTHROW(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2006\t2000\n"));

    SECTION("Charater instead of int in number of addresses")
    {
        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1s\t127.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12w127.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\ts.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\t127.s.0.1:2007\t3\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\t127.0.s.1:2007\t3\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\t127.0.0.s:2007\t3\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\t127.0.0.0s2007\t3\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\t127.0.0.1:s\t3\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\t127.0.0.1:2007\ts\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12\t127.0.0.1:2007!3\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"12!127.0.0.1:2007!3\n"));
    }

    SECTION("Double separator")
    {

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2006\t2000\n\t"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2006\t2000\n\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t12\t127.0.0.1:2006\t2000\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t\t12\t127.0.0.1:2006\t2000\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t\t127.0.0.1:2006\t2000\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2006\t\t2000\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2006\t2000\t\n"));
    }

    SECTION("No input")
    {
        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,""));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\t"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\t"));
    }

    SECTION("Characters instead of input && after EOL")
    {
        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"dfsfdfsf\n"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2006\t2000\n\t"));

        REQUIRE_THROWS(GetTrustLineByAddressCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2006\t2000\ndsfsdf"));
    }
}