#ifndef GEO_NETWORK_CLIENT_INITTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_INITTRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"

#include "../../../../common/exceptions/ValueError.h"

class InitTrustLineCommand : public BaseUserCommand {

public:
    typedef shared_ptr<InitTrustLineCommand> Shared;

public:
    InitTrustLineCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier()
    noexcept;

    const NodeUUID &contractorUUID() const
    noexcept;

    const SerializedEquivalent equivalent() const
    noexcept;

    const string contractorAddress() const
    noexcept;

private:
    NodeUUID mContractorUUID;
    SerializedEquivalent mEquivalent;
    string mContractorAddress;
};


#endif //GEO_NETWORK_CLIENT_INITTRUSTLINECOMMAND_H
