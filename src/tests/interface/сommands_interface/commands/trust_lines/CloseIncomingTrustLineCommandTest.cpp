#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines/CloseIncomingTrustLineCommandTest.h"


TEST_CASE("Testing CloseIncomingTrustLineCommand")
{

    BaseUserCommand *command = nullptr;
    command = new CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0 2000 0");

    SECTION("Character in contracorID")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a 2000 0"));

    }

    SECTION("Character in equivalent")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0 a 0"));

    }

    SECTION("Character in both contractorID and equivalent")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a a 0"));

    }

    SECTION("Charater instead of space after contractorID")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0!2000 0"));

    }

    SECTION("Charater instead of space after equivalent")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0 2000!0"));

    }

    SECTION("Charater instead of space in both cases")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0!2000!0"));

    }

    SECTION("Charater instead of space and value in contractorID")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a!2000 0"));

    }

    SECTION("Charater instead of space and value in equivalent")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0 a!0"));

    }

    SECTION("Charater instead of space and value in both cases")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a!a!0"));

    }

    SECTION("Float instead of int in contractorID")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1.2 2000 0"));

    }

    SECTION("Float instead of int in equivalent")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1 2.2 0"));

    }

    SECTION("Float instead of int in both cases")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1.2 2.2 0"));

    }

    SECTION("No input")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

    }

    SECTION("Charcters instead of command")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "asdfdf"));

    }

}
