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
            const TrustLineAmount &amount);

    void accept(
            const NodeUUID &contractorUUID,
            const TrustLineAmount &amount);

    void close(
            const NodeUUID &contractorUUID);

    void reject(
            const NodeUUID &contractorUUID);

};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESINTERFACE_H
