#ifndef GEO_NETWORK_CLIENT_VISUALRESULT_H
#define GEO_NETWORK_CLIENT_VISUALRESULT_H

#include "../../../common/Types.h"
#include <string>

class VisualResult {

public:
    VisualResult(
        const string &visualIdentifier,
        const string &visualInformation);

    const string serialize() const;

    enum VisualResultType {
        Topology = 0,
        // 1 - creating node
        OutgoingTrustLineOpen = 2,
        OutgoingTrustLineClose = 3,
        IncomingTrustLineOpen = 4,
        IncomingTrustLineClose = 5,
        CoordinatorOnPayment = 6,
        PaymentPaths = 7,
        ActualPaymentPaths = 8,
        ReceiverOnPayment = 9,
    };

private:
    string mVisualIdentifier;
    string mVisualInformation;
};


#endif //GEO_NETWORK_CLIENT_VISUALRESULT_H
