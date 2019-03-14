#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines/CloseIncomingTrustLineCommandTest.h"


TEST_CASE("Testing CloseIncomingTrustLineCommand")
{

    REQUIRE_NOTHROW(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t1\n"));

    SECTION("Character in contractorID")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a\t0\n"));

    }

    SECTION("Character in equivalent")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\ta\n"));

    }

    SECTION("Character in both contractorID and equivalent")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a\ta\n"));

    }

    SECTION("Character instead of space after contractorID")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0!1\n"));

    }

    SECTION("Character instead of space after equivalent")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t1!"));

    }

    SECTION("Character instead of space in both cases")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0!1!"));

    }

    SECTION("Character instead of space and value in contractorID")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a!\t1\n"));

    }

    SECTION("Character instead of space and value in equivalent")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\ta!n"));

    }

    SECTION("Character instead of space and value in both cases")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a!a\n"));

    }

    SECTION("Float instead of int in contractorID")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1.2\t1\n"));

    }

    SECTION("Float instead of int in equivalent")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2.2\n"));

    }

    SECTION("Float instead of int in both cases")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1.2\t2.2\n"));

    }

    SECTION("No input")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

    }

    SECTION("Characters instead of command")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "asdfdf"));

    }

}
