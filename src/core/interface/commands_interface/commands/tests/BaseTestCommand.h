#ifndef BASETESTCOMMAND_H
#define BASETESTCOMMAND_H

#include "../../commands/BaseUserCommand.h"


#ifdef TESTS

/**
 * Base class for commands that are used for test purposes.
 * This classes would be automatically excluded from the production builds.
 */
class BaseTestCommand:
    public BaseUserCommand {

public:
    using BaseUserCommand::BaseUserCommand;
};

#endif

#endif // BASETESTCOMMAND_H
