#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines/SetOutgoingTrustLineCommand.h"

TEST_CASE("Testing SetOutgoingTrustLineCommand")
{
    SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t10\t0\n");

    SECTION("Charater instead of integer and without amount & equivalent")
    {
        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\tsads\n"));
    }

    SECTION("Charater instead of integer")
    {
        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "s\t0\t0\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0s0\t0\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\ts\t0\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t0s0\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t0\ts\n"));
    }

    SECTION("Charater instead command")
    {
        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "asdf\n"));
    }

    SECTION("No input")
    {
        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));
    }

    SECTION("Max number in input")
    {
        REQUIRE_NOTHROW(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t200000000000000000000000000000000000000\t0\n"));
    }

    SECTION("Overflow in input (40 digits)")
    {
        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t2000000000000000000000000000000000000000\t0\n"));
    }

    SECTION("Floats instead of integers")
    {
        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "2.3\t10\t0\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t1.3\t0\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t10\t1.1\n"));

    }

    SECTION("Without amount and equivalent")
    {
        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t"));
    }

    SECTION("Float instead of integer")
    {
        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1.2\t0\t0\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t1.2\t0\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t0\t1.2\n"));
    }
}
