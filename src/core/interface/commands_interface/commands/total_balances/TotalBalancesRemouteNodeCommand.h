#ifndef GEO_NETWORK_CLIENT_TOTALBALANCESREMOUTENODECOMMAND_H
#define GEO_NETWORK_CLIENT_TOTALBALANCESREMOUTENODECOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

class TotalBalancesRemouteNodeCommand : public BaseUserCommand {

public:
    typedef shared_ptr<TotalBalancesRemouteNodeCommand> Shared;

public:
    TotalBalancesRemouteNodeCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    const NodeUUID &contractorUUID() const;

    CommandResult::SharedConst responseOk(
        string &totalBalancesStr) const;

    [[deprecated("Remove it when parent class would be updated")]]
    void parse(
        const string &_){}

private:
    NodeUUID mContractorUUID;
    size_t mGatewaysCount;
    vector<NodeUUID> mGateways;
};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCESREMOUTENODECOMMAND_H
