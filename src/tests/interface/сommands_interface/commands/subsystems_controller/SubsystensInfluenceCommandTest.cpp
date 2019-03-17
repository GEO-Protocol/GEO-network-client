#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/subsystems_controller/SubsystemsInfluenceCommand.h"

TEST_CASE("Testing SubsystemsInfluenceCommand")
{
    REQUIRE_NOTHROW(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2004\t1\n"));

    SECTION("Amount start & equal to zero")
    {
        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2004\t01\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2004\t0\n"));
    }

    SECTION("Double separator")
    {
        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t12\t127.0.0.1:2004\t1\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t\t12\t127.0.0.1:2004\t1\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t\t127.0.0.1:2004\t1\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2004\t\t1\n"));
    }

    SECTION("Without address & amount")
    {
        REQUIRE_NOTHROW(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\n"));
    }

    SECTION("No input")
    {
        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\t"));
    }

    SECTION("Characters instead of  & after EOL")
    {
        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "sadsdads\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a\t12\t127.0.0.1:2004\t1\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\ta\t127.0.0.1:2004\t1\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\ta\t1\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2004\ta\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2004\t1\n\t"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2004\t1\nsdfsd"));

    }

    SECTION("Address corrupt")
    {
        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1\t2\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1.2004\t3\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.t.1:2004\t33\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.1:2004\t4\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:\t33\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t11\t127.0.0.1:2004\t44\n"));
    }

    SECTION("Max number in input(78 digits)")
    {
        REQUIRE_NOTHROW(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.3.3.3:2007\t115792089237316195423570985008687907853269984665640564039457584007913129639935\n"));
    }

    SECTION("Overflow in input (2^256 + 1 & 79 digits)")
    {
        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.3.3.3:2007\t115792089237316195423570985008687907853269984665640564039457584007913129639936\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.3.3.3:2007\t1157920892373161954235709850086879078532699846656405640394575840079131296399360\n"));
    }
}
