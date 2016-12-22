#ifndef GEO_NETWORK_CLIENT_TOTALBALANCE_H
#define GEO_NETWORK_CLIENT_TOTALBALANCE_H

#include "BaseUserCommand.h"
#include "../../../trust_lines/TrustLine.h"

class TotalBalanceCommand:
    public BaseUserCommand {
public:
    typedef shared_ptr<TotalBalanceCommand> Shared;

public:
    TotalBalanceCommand(
        const CommandUUID &uuid);

    static const string &identifier();

    void deserialize(const string &buffer){};

    const CommandResult *resultOk(
            trust_amount &totalIncomingTrust,
            trust_amount &totalIncomingTrustUsed,
            trust_amount &totalOutgoingTrust,
            trust_amount &totalOutgoingTrustUsed) const;
};

#endif //GEO_NETWORK_CLIENT_TOTALBALANCE_H
