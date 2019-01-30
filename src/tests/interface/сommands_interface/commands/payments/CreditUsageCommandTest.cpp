#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/payments/CreditUsageCommand.h"

TEST_CASE("Testing CreditUsageCommand")
{
    CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.3.3.3:2007\t2000\t3\n");




    SECTION("Two addresses")
    {
        REQUIRE_NOTHROW(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"2\t12\t127.0.0.1:2007\t12\t127.0.0.1:2007\t2000\t3\n"));
    }

    SECTION("Second address without type")
    {
        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"2\t12\t127.0.0.1:2007\t127.0.0.1:2007\t2000\t3\n"));
    }

    SECTION("Charater instead of int in number of addresses")
    {
        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"s\t12\t127.0.0.1:2007\t2000\t3\n"));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"s\t12\t127.0.0.1:2007\t200\t3\n"));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\ts\t127.0.0.1:2007\t200\t3\n"));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\ts.0.0.1:2007\t2000\t3\n"));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.s.0.1:2007\t2000\t3\n"));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.s.1:2007\t200\t3\n"));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.s:2008\t200\t3\n"));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.0s2007\t222\t3\n"));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.1:s\t222\t3\n"));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.1:2007\t2222\ts\n"));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1!12\t127.0.0.1:2007\t222\t3\n"));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12!127.0.0.1:2007\t2222\t3\n"));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t12\t127.0.0.1:2007\t2222!3\n"));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1!12!127.0.0.1:2007!2222!3\n"));
    }

    SECTION("Max number in input")
    {
        REQUIRE_NOTHROW(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.3.3.3:2007\t200000000000000000000000000000000000000\t3\n"));
    }

    SECTION("Overflow in input (40 digits)")
    {
        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.3.3.3:2007\t2000000000000000000000000000000000000000\t3\n"));
    }

    SECTION("Floats instead of integers")
    {
        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.3.3.3:2007\t34.6\t3\n"));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t12\t127.3.3.3:2007\t34\t3.3\n"));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1.2\t12\t127.3.3.3:2007\t34\t3\n"));

    }

    SECTION("No input")
    {
        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,""));

        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"\n"));
    }

    SECTION("Characters instead of input")
    {
        REQUIRE_THROWS(CreditUsageCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"dfsfdfsf\n"));
    }
}