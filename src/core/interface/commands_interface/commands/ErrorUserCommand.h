#ifndef GEO_NETWORK_CLIENT_COMMAND_ERROR_H
#define GEO_NETWORK_CLIENT_COMMAND_ERROR_H

#include "BaseUserCommand.h"

class ErrorUserCommand : public BaseUserCommand {

public:
    ErrorUserCommand(
        const CommandUUID &commandUUID,
        const string& identifier,
        const string& error) :
        BaseUserCommand(
            commandUUID,
            identifier),
        mError(error)
    {}

    inline const string &error() const { return mError; }

    CommandResult::SharedConst responseError() const
    {
        return CommandResult::SharedConst(
            new CommandResult(
                identifier(),
                UUID(),
                606,
                mError));
    }

protected:
    std::string mError;
};

#endif //GEO_NETWORK_CLIENT_COMMAND_ERROR_H
