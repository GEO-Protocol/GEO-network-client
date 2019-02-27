#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/total_balances/TotalBalancesCommand.h"

TEST_CASE("Testing TotalBalancesCommand")
{
    TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\n");

    SECTION("No input")
    {
        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));
    }

    SECTION("Character instead of integer")
    {
        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "ddf\n"));
    }

    SECTION("No EOL")
    {
        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "333"));
    }
}
