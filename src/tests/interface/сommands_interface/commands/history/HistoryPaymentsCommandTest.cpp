#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/history/HistoryPaymentsCommand.h"

TEST_CASE("Testing HistoryPaymentsCommand")
{
    REQUIRE_NOTHROW(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

    SECTION("Amount start & equal to zero ")
    {
        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t05\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t06\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t0\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t0\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));
    }

    SECTION("Double separator")
    {
        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t2\t3\t4\t5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t\t3\t4\t5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t\t4\t5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t\t47183823-2574-4bfd-b411-99ed177d3e43\t\t8\n"));
    }

    SECTION("No input")
    {
        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, ""));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\t\t"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "\n\t"));
    }

    SECTION("Characters instead of input & after EOL")
    {
        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "asdsasa"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n\t"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\nasdfs"));

    }

    SECTION("Lost delimeter")
    {
        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\2\t3\t4\t5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\3\t4\t5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\4\t5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\6\t47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\47183823-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-2574-4bfd-b411-99ed177d3e43\8\n"));
    }

    SECTION("UUID 7-4-4-12 -> 8-3-4-12 -> 8-4-3-12 -> 8-4-4-11 -> 4-4-12 -> 9-4-4-12 -> 8-5-4-12 -> 8-4-5-12 -> 8-4-4-13")
    {
        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t4718323-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-257-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-2574-4bfd-b11-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-2574-4bfd-b411-99ed177d343\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t471838273-2574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-25574-4bfd-b411-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-2574-4bfd-b4511-99ed177d3e43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "1\t2\t3\t4\t5\t6\t47183823-25574-4bfd-b411-99ed177dd3e43\t8\n"));
    }

    SECTION("Max number in input (77 digits)")
    {
        REQUIRE_NOTHROW(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t2\t3\t4\t115792089237316195423570985008687907853269984665640564039457584007913129639935\t6\t47183823-2574-4bfd-b411-99ed177dd3e4\t8\n"));

        REQUIRE_NOTHROW(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t2\t3\t4\t5\t115792089237316195423570985008687907853269984665640564039457584007913129639935\t47183823-2574-4bfd-b411-99ed177de435\t8\n"));
    }

    SECTION("Overflow in input (78 digits)")
    {
        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t2\t3\t4\t115792089237316195423570985008687907853269984665640564039457584007913129639936\t6\t47183823-2574-4bfd-b411-99ed177de43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t2\t3\t4\t5\t115792089237316195423570985008687907853269984665640564039457584007913129639936\t47183823-2574-4bfd-b411-99ed177d343\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t2\t3\t4\t115792089237316195423570985008687907853269984665640564039457584007913129639936\t115792089237316195423570985008687907853269984665640564039457584007913129639936\t47183823-2574-4bfd-b411-99ed17dde463\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t2\t3\t4\t1157920892373161954235709850086879078532699846656405640394575840079131296399350\t6\t47183823-2574-4bfd-b411-99ed177de43\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t2\t3\t4\t5\t1157920892373161954235709850086879078532699846656405640394575840079131296399350\t47183823-2574-4bfd-b411-99ed177d343\t8\n"));

        REQUIRE_THROWS(HistoryPaymentsCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"1\t2\t3\t4\t1157920892373161954235709850086879078532699846656405640394575840079131296399350\t1157920892373161954235709850086879078532699846656405640394575840079131296399350\t47183823-2574-4bfd-b411-99ed17dde463\t8\n"));
    }
}

