#ifndef GEO_NETWORK_CLIENT_UPDATETRUSTLINERESULT_H
#define GEO_NETWORK_CLIENT_UPDATETRUSTLINERESULT_H

#include "Result.h"
#include "../trust_lines/TrustLine.h"

class UpdateTrustLineResult : public Result {
private:
    trust_amount mAmount;
    uuids::uuid mContractorUUID;

public:
    UpdateTrustLineResult(trust_amount amount,
                        boost::uuids::uuid contractorUUID,
                        Command *command,
                        uint16_t resultCode,
                        string timestampExcepted,
                        string timestampCompleted);

    ~UpdateTrustLineResult();

    uint16_t getResultCode();

    string getTimestampExcepted();

    string getTimestampCompleted();

    trust_amount getAmount();

    boost::uuids::uuid getContractorUUID();

    string serialize();
};

#endif //GEO_NETWORK_CLIENT_UPDATETRUSTLINERESULT_H
