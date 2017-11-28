#ifndef GEO_NETWORK_CLIENT_GETBLACKLISTCOMMAND_H
#define GEO_NETWORK_CLIENT_GETBLACKLISTCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

class GetBlackListCommand :
    public BaseUserCommand {

public:
    typedef shared_ptr<GetBlackListCommand> Shared;

public:
    GetBlackListCommand(
        const CommandUUID &uuid,
        const string &commandBuffer)
    noexcept;

    static const string &identifier();

    CommandResult::SharedConst resultOk(string &bannedUsers) const;

protected:
    void parse(
        const string &command);
};

#endif //GEO_NETWORK_CLIENT_GETBLACKLISTCOMMAND_H
