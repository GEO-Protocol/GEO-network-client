#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/subsystems_controller/TrustLinesInfluenceCommand.h"

TEST_CASE("Testing TrustLinesInfluenceCommnad")
{
    REQUIRE_NOTHROW(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\n"));

    SECTION("No input")
    {
        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t"));
    }

    SECTION("Double separator")
    {
        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t2\t3\t4\n"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t\t2\t3\t4\n"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t\t3\t4\n"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t\t4\n"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\n"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\n"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\t"));
    }

    SECTION("Characters instead of input & after EOL")
    {
        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "dsfsfdd\n"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "dsfsfdd"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\n\t"));

        REQUIRE_THROWS(TrustLinesInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\ndfsd"));
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