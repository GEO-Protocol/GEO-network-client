#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines/InitTrustLineCommand.h"

TEST_CASE("Trying INIT trustline ")
{
    InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.1:2007\t3\n");

    SECTION("Two addresses")
    {
        REQUIRE_NOTHROW(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"2\t12\t127.0.0.1:2007\t12\t127.0.0.1:2007\t3\n"));
    }
    SECTION("Second address without type")
    {
        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"2\t12\t127.0.0.1:2007\t127.0.0.1:2007\t3\n"));
    }

    SECTION("Charater instead of int in number of addresses")
    {
        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"s\t12\t127.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"s\t12\t127.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\ts\t127.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\ts.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.s.0.1:2007\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.s.1:2007\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.s:2007\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.0s2007\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.1:s\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.1:2007\ts\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1!12\t127.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12!127.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.1:2007!3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1!12!127.0.0.1:2007!3\n"));
    }

    SECTION("No input")
    {
        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,""));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\n"));
    }

    SECTION("Characters instead of input")
    {
        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"dfsfdfsf\n"));
    }
}





