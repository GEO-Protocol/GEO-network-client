#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines/ShareKeysCommand.h"

TEST_CASE("Testing ShareKeysCommand")
{
    REQUIRE_NOTHROW(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t0\n"));

    SECTION("No input")
    {
        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\n"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\t"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\n"));
    }

    SECTION("Double separator")
    {
        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t0\n\t"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t0\n"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t0\n\n"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t0\n"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t\t0\n"));
    }

    SECTION("Characters instead of integer & after EOL")
    {
        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "dfs\t0\n"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0s0\n"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0s0\n"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t0\n\t"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t0\nsdfsd"));
    }

    SECTION("Float instead of integer value")
    {
        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1.2\t0.3\n"));

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t0.3\n"));
    }
}
