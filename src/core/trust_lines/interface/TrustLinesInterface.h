#ifndef GEO_NETWORK_CLIENT_TRUSTLINESINTERFACE_H
#define GEO_NETWORK_CLIENT_TRUSTLINESINTERFACE_H

#include "../manager/TrustLinesManager.h"

class TrustLinesInterface {

private:
    TrustLinesManager *mManager;

public:

    TrustLinesInterface(
            TrustLinesManager *manager);

    ~TrustLinesInterface();

    void open(
            const NodeUUID &contractorUUID,
            const trust_amount &amount);

    void accept(
            const NodeUUID &contractorUUID,
            const trust_amount &amount);

    void close(
            const NodeUUID &contractorUUID);

    void reject(
            const NodeUUID &contractorUUID);

};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESINTERFACE_H
