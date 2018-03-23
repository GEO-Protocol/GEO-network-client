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

private:
    string mVisualIdentifier;
    string mVisualInformation;
};


#endif //GEO_NETWORK_CLIENT_VISUALRESULT_H
