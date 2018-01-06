#ifndef GEO_NETWORK_CLIENT_TOTALBALANCESCOMMAND_H
#define GEO_NETWORK_CLIENT_TOTALBALANCESCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

class TotalBalancesCommand : public BaseUserCommand {

public:
    typedef shared_ptr<TotalBalancesCommand> Shared;

public:
    TotalBalancesCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    const vector<NodeUUID> &gateways() const;

    CommandResult::SharedConst resultOk(
        string &totalBalancesStr) const;

    [[deprecated("Remove it when parent class would be updated")]]
    void parse(
        const string &_){}

private:
    size_t mGatewaysCount;
    vector<NodeUUID> mGateways;
};


#endif //GEO_NETWORK_CLIENT_TOTALBALANCESCOMMAND_H
