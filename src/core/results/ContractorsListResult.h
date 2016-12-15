#ifndef GEO_NETWORK_CLIENT_CONTRACTORSLISTRESULT_H
#define GEO_NETWORK_CLIENT_CONTRACTORSLISTRESULT_H

#include "Result.h"

class ContractorsListResult : public Result {
private:
    vector<NodeUUID> mContractorsList;

public:
    ContractorsListResult(Command *command,
                          const uint16_t &resultCode,
                          const string &timestampCompleted,
                          vector<NodeUUID> &contractorsList);

private:
    const uuids::uuid &commandUUID() const;

    const string &id() const;

    const uint16_t &resultCode() const;

    const string &timestampExcepted() const;

    const string &timestampCompleted() const;

    const vector<NodeUUID> &contractorsList() const;

    string serialize();
};

#endif //GEO_NETWORK_CLIENT_CONTRACTORSLISTRESULT_H
