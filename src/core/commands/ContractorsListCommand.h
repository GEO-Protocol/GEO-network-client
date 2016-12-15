#ifndef GEO_NETWORK_CLIENT_CONTRACTORSLIST_H
#define GEO_NETWORK_CLIENT_CONTRACTORSLIST_H

#include "Command.h"

class ContractorsListCommand : public Command {
public:
    ContractorsListCommand(const uuids::uuid &commandUUID, const string &identifier,
                           const string &timestampExcepted);

    const uuids::uuid &commandUUID() const;

    const string &id() const;

    const string &exceptedTimestamp() const;

private:
    void deserialize();
};

#endif //GEO_NETWORK_CLIENT_CONTRACTORSLIST_H
