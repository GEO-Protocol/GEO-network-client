#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines/InitTrustLineCommand.h"

TEST_CASE("Trying INIT trustline ")
{
    REQUIRE_NOTHROW(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.1:2007\t3\n"));

    SECTION("Two addresses")
    {
        REQUIRE_NOTHROW(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"2\t12\t127.0.0.1:2007\t12\t127.0.0.1:2007\t3\n"));
    }
    SECTION("Address type corrupted")
    {
        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"2\t12\t127.0.0.1:2007\t127.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t13\t127.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"2\t12\t127.0.0.1:2007\t13\t127.0.0.1:2007\t3\n"));
    }

    SECTION("Double separator")
    {
        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.1:2007\t3\n\t"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.1:2007\t3\n\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\t\t12\t127.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t\t12\t127.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t\t127.0.0.1:2007\t3\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.1:2007\t\t3\n"));
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

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\t"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\t\t"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\t\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\n\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\n\t"));
    }

    SECTION("Characters instead of input & after EOL")
    {
        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"dfsfdfsf\n"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.1:2007\t3\n\t"));

        REQUIRE_THROWS(InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.1:2007\t3\nsdf"));
    }
}





