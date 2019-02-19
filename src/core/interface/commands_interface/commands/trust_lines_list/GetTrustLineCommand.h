#ifndef GEO_NETWORK_CLIENT_GETTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_GETTRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../contractors/addresses/IPv4WithPortAddress.h"
#include "../../../../common/exceptions/ValueError.h"

class GetTrustLineCommand : public BaseUserCommand {

public:
    typedef shared_ptr<GetTrustLineCommand> Shared;

public:
    GetTrustLineCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);


    static const string &identifier();

    BaseAddress::Shared contractorAddress() const;

    const SerializedEquivalent equivalent() const;

    CommandResult::SharedConst resultOk(
        string &neighbor) const;

protected:
    BaseAddress::Shared mContractorAddress;
    SerializedEquivalent mEquivalent;
};

#endif //GEO_NETWORK_CLIENT_GETTRUSTLINECOMMAND_H
