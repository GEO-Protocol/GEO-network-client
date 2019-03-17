#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines/CloseIncomingTrustLineCommandTest.h"

TEST_CASE("Testing CloseIncomingTrustLineCommand")
{
    REQUIRE_NOTHROW(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t1\n"));

    SECTION("Double separator")
    {
        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t1\n\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t1\n\t"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t1\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t\t1\n"));
    }

    SECTION("Character instead proper value")
    {
        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a\t0\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\ta\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a\ta\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0!1\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t1!"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0!1!"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a!\t1\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\ta!n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a!a\n"));
    }

    SECTION("Float instead of int in contractorID")
    {
        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1.2\t1\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2.2\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1.2\t2.2\n"));
    }

    SECTION("No input")
    {
        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\t"));
    }

    SECTION("Characters instead of command & after EOL")
    {
        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "asdfdf"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t1\n\t"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t1\nsdfd"));
    }
}
