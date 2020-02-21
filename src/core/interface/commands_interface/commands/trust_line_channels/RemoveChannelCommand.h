#ifndef GEO_NETWORK_CLIENT_REMOVECHANNELCOMMAND_H
#define GEO_NETWORK_CLIENT_REMOVECHANNELCOMMAND_H

#include "../BaseUserCommand.h"

class RemoveChannelCommand : public BaseUserCommand {

public:
    typedef shared_ptr<RemoveChannelCommand> Shared;

public:
    RemoveChannelCommand(
        const CommandUUID &commandUUID,
        const string &commandBuffer);

    static const string &identifier();

    const ContractorID contractorChannelID() const;

private:
    ContractorID mContractorChannelID;
};


#endif //GEO_NETWORK_CLIENT_REMOVECHANNELCOMMAND_H
