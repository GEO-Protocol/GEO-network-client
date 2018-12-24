#ifndef GEO_NETWORK_CLIENT_INITTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_INITTRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../contractors/addresses/IPv4WithPortAddress.h"
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

    const SerializedEquivalent equivalent() const
    noexcept;

    BaseAddress::Shared contractorAddress() const
    noexcept;

private:
    SerializedEquivalent mEquivalent;
    BaseAddress::Shared mContractorAddress;
};


#endif //GEO_NETWORK_CLIENT_INITTRUSTLINECOMMAND_H
