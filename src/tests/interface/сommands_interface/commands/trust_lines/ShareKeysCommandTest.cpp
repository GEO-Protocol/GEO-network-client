#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines/ShareKeysCommand.h"

TEST_CASE("Testing ShareKeysCommand")
{
    ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t0\n");

    SECTION("No input")
    {
        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));
    }

    SECTION("Characters instead of integer")
    {
        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "dfs\t0\n"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0s0\n"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0s0\n"));
    }

    SECTION("Float instead of integer value")
    {
        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1.2\t0.3\n"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t0.3\n"));
    }
}
