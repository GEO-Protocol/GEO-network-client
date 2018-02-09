#ifndef GEO_NETWORK_CLIENT_CLOSEINCOMINGTRUSTLINECOMMAND_H
#define GEO_NETWORK_CLIENT_CLOSEINCOMINGTRUSTLINECOMMAND_H

#include "../BaseUserCommand.h"

#include "../../../../common/exceptions/ValueError.h"

class CloseIncomingTrustLineCommand : public BaseUserCommand {

public:
    typedef shared_ptr<CloseIncomingTrustLineCommand> Shared;

public:
    CloseIncomingTrustLineCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier()
        noexcept;

    const NodeUUID &contractorUUID() const
        noexcept;

private:
    NodeUUID mContractorUUID;
};


#endif //GEO_NETWORK_CLIENT_CLOSEINCOMINGTRUSTLINECOMMAND_H
