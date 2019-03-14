#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/transactions/PaymentTransactionByCommandUUIDCommand.h"

TEST_CASE("Testing PaymentTransactionByCommandUUIDCommand")
{
    REQUIRE_NOTHROW(PaymentTransactionByCommandUUIDCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "11111111-1111-1111-1111-111111111111\n"));

    SECTION("UUID 7-4-4-12 -> 8-3-4-12 -> 8-4-3-12 -> 8-4-4-11 -> 4-4-12 -> 9-4-4-12 -> 8-5-4-12 -> 8-4-5-12 -> 8-4-4-13")
    {
        REQUIRE_THROWS(PaymentTransactionByCommandUUIDCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "4718323-2574-4bfd-b411-99ed177d3e43\n"));

        REQUIRE_THROWS(PaymentTransactionByCommandUUIDCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "47183823-257-4bfd-b411-99ed177d3e43\n"));

        REQUIRE_THROWS(PaymentTransactionByCommandUUIDCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "47183823-2574-4bfd-b11-99ed177d3e43\n"));

        REQUIRE_THROWS(PaymentTransactionByCommandUUIDCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "47183823-2574-4bfd-b411-99ed177d343\n"));

        REQUIRE_THROWS(PaymentTransactionByCommandUUIDCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "2574-4bfd-b411-99ed177d3e43\n"));

        REQUIRE_THROWS(PaymentTransactionByCommandUUIDCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "471838273-2574-4bfd-b411-99ed177d3e43\n"));

        REQUIRE_THROWS(PaymentTransactionByCommandUUIDCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "7183823-25574-4bfd-b411-99ed177d3e43\n"));

        REQUIRE_THROWS(PaymentTransactionByCommandUUIDCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "47183823-2574-4bfd-b4511-99ed177d3e43\n"));

        REQUIRE_THROWS(PaymentTransactionByCommandUUIDCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "47183823-25574-4bfd-b411-99ed177dd3e43\n"));
    }

    SECTION("No input")
    {
        REQUIRE_THROWS(PaymentTransactionByCommandUUIDCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(PaymentTransactionByCommandUUIDCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));
    }

    SECTION("Character instead of UUID")
    {
        REQUIRE_THROWS(PaymentTransactionByCommandUUIDCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "fgdgfgdgfdd"));
    }

    SECTION("Lost EOL")
    {
        REQUIRE_THROWS(PaymentTransactionByCommandUUIDCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "47183823-25574-4bfd-b411-99ed177dd3e43"));
    }
}
