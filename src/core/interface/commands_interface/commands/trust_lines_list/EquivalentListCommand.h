#ifndef GEO_NETWORK_CLIENT_EQUIVALENTLISTCOMMAND_H
#define GEO_NETWORK_CLIENT_EQUIVALENTLISTCOMMAND_H

#include "../BaseUserCommand.h"

class EquivalentListCommand : public BaseUserCommand {

public:
    typedef shared_ptr<EquivalentListCommand> Shared;

public:
    EquivalentListCommand(
        const CommandUUID &uuid,
        const string &commandBuffer)
    noexcept;

    static const string &identifier();

    CommandResult::SharedConst resultOk(
        string &equivalents) const;
};


#endif //GEO_NETWORK_CLIENT_EQUIVALENTLISTCOMMAND_H
