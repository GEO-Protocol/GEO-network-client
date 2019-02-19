#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/subsystems_controller/TrustLinesInfluenceCommand.h"

TEST_CASE("Testing TrustLinesInfluenceCommnad")
{
    TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\n");

    SECTION("No input")
    {
        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));
    }

    SECTION("Characters instead of input")
    {
        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "dsfsfdd\n"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "dsfsfdd"));
    }

    SECTION("Characters instead of integer")
    {
        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a\t2\t3\t4\n"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1a2\t3\t4\n"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\ta\t3\t4\n"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2a3\t4\n"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3a4\n"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\ta\n"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4a"));
    }
}