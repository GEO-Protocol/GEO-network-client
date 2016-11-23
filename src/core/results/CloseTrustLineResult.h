#ifndef GEO_NETWORK_CLIENT_CLOSETRUSTLINERESULT_H
#define GEO_NETWORK_CLIENT_CLOSETRUSTLINERESULT_H

#include "Result.h"
#include "../trust_lines/TrustLine.h"

class CloseTrustLineResult : public Result {
private:
    uuids::uuid mContractorUUID;

public:
    CloseTrustLineResult(boost::uuids::uuid contractorUUID,
                  Command *command,
                  uint16_t resultCode,
                  string timestampExcepted,
                  string timestampCompleted);

    ~CloseTrustLineResult();

    uint16_t getResultCode();

    string getTimestampExcepted();

    string getTimestampCompleted();

    boost::uuids::uuid getContractorUUID();

    string serialize();
};

#endif //GEO_NETWORK_CLIENT_CLOSETRUSTLINERESULT_H
