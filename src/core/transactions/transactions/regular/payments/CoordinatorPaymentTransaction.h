#ifndef GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
#define GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H


#include "../../BaseTransaction.h"


class CoordinatorPaymentTranscation:
    public BaseTransaction {

public:
    typedef shared_ptr<CoordinatorPaymentTranscation> Shared;
    typedef shared_ptr<const CoordinatorPaymentTranscation> ConstShared;

public:
    CoordinatorPaymentTranscation(
        NodeUUID &nodeUUID,
        Paym::Shared command,
        TransactionsScheduler *scheduler,
    TrustLinesManager *manager);

};


#endif //GEO_NETWORK_CLIENT_COORDINATORPAYMENTTRANSCATION_H
