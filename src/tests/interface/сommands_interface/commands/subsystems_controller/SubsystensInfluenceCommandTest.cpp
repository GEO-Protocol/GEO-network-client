#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/subsystems_controller/SubsystemsInfluenceCommand.h"

TEST_CASE("Testing SubsystemsInfluenceCommand")
{
    SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t11111111-1111-1111-1111-111111111111\t1\n");

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

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "a\t11111111-1111-1111-1111-111111111111\t1\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\ta\t1\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t11111111-1111-1111-1111-111111111111\ta\n"));
    }

    SECTION("UUID 7-4-4-12 -> 8-3-4-12 -> 8-4-3-12 -> 8-4-4-11 -> 4-4-12 -> 9-4-4-12")
    {
        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t4718323-2574-4bfd-b411-99ed177d3e43\t2\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t47183823-257-4bfd-b411-99ed177d3e43\t3\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t47183823-2574-4bfd-b11-99ed177d3e43\t33\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t47183823-2574-4bfd-b411-99ed177d343\t4\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2574-4bfd-b411-99ed177d3e43\t33\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t471838273-2574-4bfd-b411-99ed177d3e43\t44\n"));
    }

    SECTION("UUID 8-5-4-12 -> 8-4-5-12 -> 8-4-4-13")
    {
        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t7183823-25574-4bfd-b411-99ed177d3e43\t44\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t47183823-2574-4bfd-b4511-99ed177d3e43\t44\n"));

        REQUIRE_THROWS(SubsystemsInfluenceCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t47183823-25574-4bfd-b411-99ed177dd3e43\t33\n"));
    }
}
