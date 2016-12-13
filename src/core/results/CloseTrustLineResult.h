#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINERESULT_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINERESULT_H

#include "Result.h"

class CloseTrustLineResult : public Result {
private:
    NodeUUID mContractorUUID;

public:
    CloseTrustLineResult(Command *command,
                         const uint16_t &resultCode,
                         const string &timestampCompleted,
                         const NodeUUID &contractorUUID);

    const uuids::uuid &commandUUID() const;

    const string &id() const;

    const uint16_t &resultCode() const;

    const string &timestampExcepted() const;

    const string &timestampCompleted() const;

    const NodeUUID &contractorUUID() const;

    string serialize();
};

#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINERESULT_H
