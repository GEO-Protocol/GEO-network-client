#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines_list/GetFirstLevelContractorsCommand.h"

TEST_CASE("Testing GetFirstLevelContractorsCommand")
{
    GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\n");

    SECTION("No input")
    {
        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));
    }

    SECTION("Character instead of integer")
    {
        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "ddf\n"));
    }

    SECTION("No EOL")
    {
        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "333"));
    }
}
