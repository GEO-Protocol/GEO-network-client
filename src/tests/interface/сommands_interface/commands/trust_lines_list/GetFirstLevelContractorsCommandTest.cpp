#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines_list/GetFirstLevelContractorsCommand.h"

TEST_CASE("Testing GetFirstLevelContractorsCommand")
{
    REQUIRE_NOTHROW(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\n"));

    SECTION("No input")
    {
        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));

        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t"));

        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\n"));

        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t"));

        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\n"));

        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\t"));

        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t\n"));
    }

    SECTION("Double separaotor")
    {
        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t1\n"));

        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t\n"));

        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n1\n"));

        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t1\t\n"));
    }

    SECTION("Character instead of integer")
    {
        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "ddf\n"));
    }

    SECTION("No EOL & character after EOL")
    {
        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "333"));

        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\n\t"));

        REQUIRE_THROWS(GetFirstLevelContractorsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\nsdfsd"));
    }
}
