#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines_list/GetTrustLinesCommand.h"

TEST_CASE("Testing GetTrustLinesCommand")
{
    REQUIRE_NOTHROW(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\n"));

    SECTION("Characters instead of integer and tabulation")
    {
        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a\t2\t3\n"));

        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1a2\t3\n"));

        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\ta\t3\n"));

        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2a3\n"));

        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\ta\n"));

        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3a"));
    }

    SECTION("Double separator")
    {
        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\n\n"));

        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\n\t"));

        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t2\t3\n"));

        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t\t2\t3\n"));

        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t\t3\n"));
    }

    SECTION("No input")
    {
        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));

        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t"));

        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\n"));

        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t"));

        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\t"));

        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\n"));
    }

    SECTION("Characters instead of input")
    {
        REQUIRE_THROWS(GetTrustLinesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "asdfda\n"));
    }
}