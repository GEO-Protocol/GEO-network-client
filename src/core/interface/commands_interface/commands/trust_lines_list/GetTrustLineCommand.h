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

    const SerializedEquivalent equivalent() const;

    CommandResult::SharedConst resultOk(
        string &neighbor) const;

protected:
    NodeUUID mContractorUUID;
    SerializedEquivalent mEquivalent;
};

#endif //GEO_NETWORK_CLIENT_GETTRUSTLINECOMMAND_H
