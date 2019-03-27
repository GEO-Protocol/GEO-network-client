#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/history/HistoryAdditionalPaymentsCommand.h"

TEST_CASE("Testing HistoryAdditionalPaymentsCommand")
{
    REQUIRE_NOTHROW(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t1\t1\t1\t1\t1\t1\n"));

    SECTION("Amount start & equal to zero")
    {
        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t1\t1\t1\t01\t1\t1\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t1\t1\t1\t1\t01\t1\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t1\t1\t1\t0\t1\t1\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t1\t1\t1\t1\t0\t1\n"));
    }

    SECTION("Double separator")
    {
        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t\t1\t1\t1\t1\t1\t1\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t1\t\t1\t1\t1\t1\t1\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t1\t1\t\t1\t1\t1\t1\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t1\t1\t1\t\t1\t1\t1\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t1\t1\t1\t1\t\t1\t1\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t1\t1\t1\t1\t1\t\t1\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t1\t1\t1\t1\t1\t1\n"));
    }

    SECTION("No input")
    {
        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\t"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\n"));
    }

    SECTION("Characters in input & after EOL")
    {
        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "sdfsfdsffs"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t1\t1\t1\t1\t1\t1\n\t"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t1\t1\t1\t1\t1\t1\ndfs"));
    }

    SECTION("Forget delimeter")
    {
        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\34543\t45128987\t69779\t0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\45128987\t69779\t0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\69779\t0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t69779\0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t69779\t0192847\39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t69779\t0192847\t39381728\34029\n"));
    }

    SECTION("Max number in input (78 digits)")
    {
        REQUIRE_NOTHROW(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"999\t99\t999\t999\t115792089237316195423570985008687907853269984665640564039457584007913129639935\t999\t999\n"));

        REQUIRE_NOTHROW(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"999\t99\t999\t999\t99\t115792089237316195423570985008687907853269984665640564039457584007913129639935\t999\n"));
    }

    SECTION("Overflow in input (2^256 + 1 78 digits overflow ) & ( 79 digits )")
    {
        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"999\t99\t999\t999\t115792089237316195423570985008687907853269984665640564039457584007913129639936\t999\t999\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"999\t99\t999\t999\t99\t115792089237316195423570985008687907853269984665640564039457584007913129639936\t999\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"999\t99\t999\t999\t115792089237316195423570985008687907853269984665640564039457584007913129639936\t115792089237316195423570985008687907853269984665640564039457584007913129639936\t999\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"999\t99\t999\t999\t1157920892373161954235709850086879078532699846656405640394575840079131296399350\t999\t999\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"999\t99\t999\t999\t99\t1157920892373161954235709850086879078532699846656405640394575840079131296399350\t999\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"999\t99\t999\t999\t1157920892373161954235709850086879078532699846656405640394575840079131296399350\t1157920892373161954235709850086879078532699846656405640394575840079131296399350\t999\n"));
    }

    SECTION("Characters in integer values")
    {
        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"342d3\t34543\t45128987\t69779\t0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34s543\t45128987\t69779\t0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t451289d87\t69779\t0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t697d79\t0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t69779\t01928d47\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t69779\t0192847\t393817d28\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t69779\t0192847\t39381728\t340d29\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"342!!3\t34543\t45128987\t69779\t0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34#543\t45128987\t69779\t0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128@987\t69779\t0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t6977$9\t0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t69779\t019^2847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t69779\t0192847\t3938()1728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t69779\t0192847\t39381728\t3402+_9\n"));
    }

    SECTION("Float in integer values")
    {
        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"342.3\t34543\t45128987\t69779\t0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t3454.3\t45128987\t69779\t0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128.987\t69779\t0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t697.79\t0192847\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t69779\t01928.47\t39381728\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t69779\t0192847\t393817.28\t34029\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t69779\t0192847\t39381728\t34.029\n"));
    }
}

