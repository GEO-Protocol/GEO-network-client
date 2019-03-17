#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines/SetOutgoingTrustLineCommand.h"

TEST_CASE("Testing SetOutgoingTrustLineCommand")
{
    REQUIRE_NOTHROW(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t10\t0\n"));

    SECTION("Charater instead of integer and without amount & equivalent")
    {
        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\tsads\n"));
    }

    SECTION("Amount start & equal to zero")
    {
        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t01\t0\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t0\t0\n"));
    }

    SECTION("Double separator")
    {
        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t10\t0\n\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t10\t0\n\t"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t10\t0\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t\t10\t0\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t10\t\t\n"));
    }

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

    SECTION("Charater instead cinput & after EOL")
    {
        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "asdf\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t10\t0\n\t"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t10\t0\nsdfdfsfs"));
    }

    SECTION("No input")
    {
        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\t"));
    }

    SECTION("Max number in input (78 digits)")
    {
        REQUIRE_NOTHROW(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t115792089237316195423570985008687907853269984665640564039457584007913129639935\t0\n"));
    }

    SECTION("Overflow in input (2^256 + 1 & 79 digits)")
    {
        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t115792089237316195423570985008687907853269984665640564039457584007913129639936\t0\n"));

        REQUIRE_THROWS(SetOutgoingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t11579208923731619542357098500868790785326998466564056403945758400791312963993650\t0\n"));
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
