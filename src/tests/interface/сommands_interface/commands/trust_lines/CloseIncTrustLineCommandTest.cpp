#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines/CloseIncomingTrustLineCommand.h"

TEST_CASE("Testing CloseIncomingTrustLineCommand")
{
    REQUIRE_NOTHROW(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t2000\n"));

    SECTION("Character in contracorID")
    {
        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a\t2000\n"));
    }

    SECTION("Double separatro")
    {

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2000\n\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2000\n\t"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t2000\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t\t2000\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t2000\t\n"));
    }

    SECTION("Character & symbols in input")
    {
        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\ta\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a\ta\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0!2000\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t2000!"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0!2000\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a!2000\n"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\ta!"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a!a!0\n"));
    }

    SECTION("Float instead of int in contractorID")
    {
        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1.2\t2000\n"));

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

    SECTION("Charcters instead of command")
    {
        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "asdfdf"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t2000\n\t"));

        REQUIRE_THROWS(CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t2000\nsdfsf"));
    }
}
