#ifndef GEO_NETWORK_CLIENT_OPERATIONSHISTORYSTORAGETEST_H
#define GEO_NETWORK_CLIENT_OPERATIONSHISTORYSTORAGETEST_H

#include "../../core/common/Types.h"
#include "../../core/common/NodeUUID.h"
#include "../../core/common/multiprecision/MultiprecisionUtils.h"

#include "../../core/db/operations_history_storage/record/base/Record.h"
#include "../../core/db/operations_history_storage/record/trust_line/TrustLineRecord.h"
#include "../../core/db/operations_history_storage/record/payment/PaymentRecord.h"
#include "../../core/db/operations_history_storage/storage/OperationsHistoryStorage.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <memory>
#include <utility>
#include <vector>
#include <stdint.h>

namespace st = db::operations_history_storage;
namespace uuids = boost::uuids;

using namespace std;

class OperationsHistoryStorageTest {

public:
    OperationsHistoryStorageTest();

    st::Record::Shared createTrustLineRecord(
        st::TrustLineRecord::TrustLineOperationType operationType,
        TrustLineAmount amount);

    st::Record::Shared createPaymentRecord(
        st::PaymentRecord::PaymentOperationType operationType,
        TrustLineAmount amount);

    void compareDataTestCase();

    void run();

private:
    unique_ptr<st::OperationsHistoryStorage> mStorage;
    vector<st::Record::Shared> mRecords;
};


#endif //GEO_NETWORK_CLIENT_OPERATIONSHISTORYSTORAGETEST_H