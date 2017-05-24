#ifndef GEO_NETWORK_CLIENT_GETTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_GETTRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"


class GetTrustLineCommand :
    public BaseUserCommand {

public:
    typedef shared_ptr<GetTrustLineCommand> Shared;

public:
    GetTrustLineCommand(
        const CommandUUID &uuid,
        const string &commandBuffer)
    noexcept;

    static const string &identifier();

    NodeUUID contractorUUID();

    CommandResult::SharedConst resultOk(
        string &neighbor) const;

protected:
    void parse(const string &command);

protected:
    NodeUUID mContractorUUID;
};

#endif //GEO_NETWORK_CLIENT_GETTRUSTLINECOMMAND_H
