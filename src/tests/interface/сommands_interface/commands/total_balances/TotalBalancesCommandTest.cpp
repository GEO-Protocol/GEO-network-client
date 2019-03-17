#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/total_balances/TotalBalancesCommand.h"

TEST_CASE("Testing TotalBalancesCommand")
{
    REQUIRE_NOTHROW(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\n"));

    SECTION("No input")
    {
        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));

        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t"));

        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\n"));

        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t"));

        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\n"));

        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\t"));
    }

    SECTION("Double separator")
    {
        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n1\n"));

        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t1\t"));

        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t1\n"));

        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n1\t"));

    }

    SECTION("Character instead of integer & after EOL")
    {
        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "ddf\n"));

        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\n\t"));

        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\nsdfd"));

    }

    SECTION("No EOL")
    {
        REQUIRE_THROWS(TotalBalancesCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "333"));
    }
}
