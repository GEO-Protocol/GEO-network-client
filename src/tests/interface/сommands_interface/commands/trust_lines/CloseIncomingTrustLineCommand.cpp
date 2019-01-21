#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines/CloseIncomingTrustLineCommand.h"

TEST_CASE("Trying CLOSE incomingtrustline ")
{
    BaseUserCommand *command = nullptr;
    command = new CloseIncomingTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s, "0 2000 0");
}