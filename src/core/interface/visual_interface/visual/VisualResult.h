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
        OutgoingTrustLine = 2,
        IncomingTrustLine = 3,
        PaymentTopology = 4,
        PaymentPaths = 5,
        ActualPaymentPaths = 6
    };

private:
    string mVisualIdentifier;
    string mVisualInformation;
};


#endif //GEO_NETWORK_CLIENT_VISUALRESULT_H
