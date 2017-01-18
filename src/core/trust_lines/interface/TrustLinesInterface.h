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

    const bool isExist(
        const NodeUUID &contractorUUID);

    const bool isDirectionOutgoing(
        const NodeUUID &contractorUUID);

    const bool isDirectionIncoming(
        const NodeUUID &contractorUUID);

    const bool checkOutgoingAmount(
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount
    );

    const bool checkIncomingAmount(
        const NodeUUID &contractorUUID,
        const TrustLineAmount &amount
    );

};


#endif //GEO_NETWORK_CLIENT_TRUSTLINESINTERFACE_H
