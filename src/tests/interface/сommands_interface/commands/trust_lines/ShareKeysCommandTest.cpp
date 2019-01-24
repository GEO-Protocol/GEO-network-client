#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines/ShareKeysCommand.h"

TEST_CASE("Testing ShareKeysCommand")
{

    BaseUserCommand *command = nullptr;
    command = new ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0\t0\n");

    SECTION("SHARKEYS")
    {

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "dfs\t0\n"));

    }

    SECTION("SHARKEYS")
    {

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0s0\n"));

    }

    SECTION("SHARKEYS")
    {

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0s0\n"));

    }

    SECTION("SHARKEYS")
    {

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

    }

    SECTION("SHARKEYS")
    {

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1.2\t0.3\n"));

    }
    SECTION("SHARKEYS")
    {

        REQUIRE_THROWS(ShareKeysCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t0.3\n"));

    }

}
