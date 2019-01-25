#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/history/HistoryAdditionalPaymentsCommand.h"

TEST_CASE("Testing HistoryAdditionalPaymentsCommand")
{
    HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t1\t1\t1\t1\t1\t1\n");

    SECTION("Different number in input")
    {
        REQUIRE_NOTHROW(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\t34543\t45128987\t69779\t192847\t39381728\t34029\n"));
    }

    SECTION("No input")
    {
        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));
    }

    SECTION("Characters in input")
    {
        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "sdfsfdsffs"));
    }

    SECTION("Forget delimeter")
    {
        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"3423\34543\t45128987\t69779\t0192847\39381728\t34029\n"));
    }

    SECTION("Max number in input")
    {
        REQUIRE_NOTHROW(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"999\t99\t999\t999\t340282366920938463463374607431768211456\t999\t999\n"));

        REQUIRE_NOTHROW(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"999\t99\t999\t999\t99\t340282366920938463463374607431768211456\t999\n"));
    }

    SECTION("Overflow in input (40 digits)")
    {
        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"999\t99\t999\t999\t3402823669209384634633746074317682114560\t999\t999\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"999\t99\t999\t999\t99\t3402823669209384634633746074317682114560\t999\n"));

        REQUIRE_THROWS(HistoryAdditionalPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"999\t99\t999\t999\t3402823669209384634633746074317682114560\t3402823669209384634633746074317682114560\t999\n"));
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

