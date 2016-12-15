#ifndef GEO_NETWORK_CLIENT_TOTALBALANCE_H
#define GEO_NETWORK_CLIENT_TOTALBALANCE_H

#include "Command.h"

class TotalBalanceCommand : public Command {
private:
    string mCommandBuffer;

public:
    TotalBalanceCommand(const uuids::uuid &commandUUID, const string &identifier,
                        const string &timestampExcepted);

    const uuids::uuid &commandUUID() const;

    const string &id() const;

    const string &exceptedTimestamp() const;


private:
    void deserialize();
};

#endif //GEO_NETWORK_CLIENT_TOTALBALANCE_H
