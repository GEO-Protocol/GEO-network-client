#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/subsystems_controller/SubsystemsInfluenceCommand.h"

TEST_CASE("Testing SubsystemsInfluenceCommand")
{
    REQUIRE_NOTHROW(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2004\t1\n"));

    SECTION("Without UUID & amount")
    {
        REQUIRE_NOTHROW(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\n"));
    }

    SECTION("No input")
    {
        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));
    }

    SECTION("Characters instead of command")
    {
        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "sadsdads\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a\t12\t127.0.0.1:2004\t1\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\ta\t127.0.0.1:2004\t1\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\ta\t1\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:2004\ta\n"));
    }

    SECTION("Address")
    {
        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1\t2\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1.2004\t3\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.t.1:2004\t33\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.1:2004\t4\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.0.0.1:\t33\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t11\t127.0.0.1:2004\t44\n"));
    }

}
