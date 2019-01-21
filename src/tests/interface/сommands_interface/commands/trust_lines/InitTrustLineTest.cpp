#include "../../../../catch.hpp"
#include "../../../../../core/interface/commands_interface/commands/trust_lines/InitTrustLineCommand.h"

TEST_CASE("Trying INIT trustline ")
{
    BaseUserCommand *command = nullptr;
    command = new InitTrustLineCommand("47183823-2574-4bfd-b411-99ed177d3e43"s,"2 12 127.0.0.1:2007 12 127.0.33.1:2007 2000 1\n");

}





