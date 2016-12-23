#ifndef GEO_NETWORK_CLIENT_BASETRANSACTION_H
#define GEO_NETWORK_CLIENT_BASETRANSACTION_H


#include "../../../../../common/Types.h"
#include "../../../../../db/fields/common/AbstractRecordsHandler.h"

#include <utility>
#include <cstring>
#include <memory>
#include "malloc.h"
#include <assert.h>


namespace io {
namespace routing_tables {


using namespace std;
using namespace db;


class Operation:
    public AbstractRecordsHandler {

public:
    typedef shared_ptr<Operation> Shared;

    typedef byte SerializedOperationType;
    typedef byte SerializedTrustLineDirectionType;

public:
    enum TransactionType {
        Set = 1,
        RollbackSet,

        Remove,
        RollbackRemove,

        Update,
        RollbackUpdate,
    };
    const TransactionType type;

public:
    Operation(TransactionType type);

    virtual const pair<shared_ptr<byte>, size_t> serialize() const = 0;

protected:
    static const size_t kTrustLineDirectionSize = 1;
};


} // routing_tables
} // io

#endif //GEO_NETWORK_CLIENT_BASETRANSACTION_H
