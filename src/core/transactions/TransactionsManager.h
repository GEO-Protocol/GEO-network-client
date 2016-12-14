#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H

#include <string>
#include <chrono>
#include <boost/lexical_cast.hpp>
#include "../commands/Command.h"
#include "../commands/OpenTrustLineCommand.h"
#include "../commands/CloseTrustLineCommand.h"
#include "../commands/UpdateOutgoingTrustAmountCommand.h"
#include "../commands/MaximalTransactionAmountCommand.h"
#include "../commands/UseCreditCommand.h"
#include "../results/Result.h"
#include "../results/OpenTrustLineResult.h"
#include "../results/CloseTrustLineResult.h"
#include "../results/UpdateTrustLineResult.h"
#include "../results/MaximalTransactionAmountResult.h"
#include "../results/PaymentResult.h"

using namespace std;

class TransactionsManager {

private:
    static const constexpr char* kTrustLinesOpenIdentifier = "CREATE:contractors/trust-lines";
    static const constexpr char* kTrustLinesCloseIdentifier = "REMOVE:contractors/trust-lines";
    static const constexpr char* kTrustLinesUpdateIdentifier = "SET:contractors/trust-lines";
    static const constexpr char* kTransactionsUseCreditIdentifier = "CREATE:contractors/transations";
    static const constexpr char* kTransactionsMaximalAmountIdentifier = "GET:contractors/transations/max";

public:
    TransactionsManager();

    ~TransactionsManager();

    pair<bool, shared_ptr<Result>> acceptCommand(shared_ptr<Command> commandPointer);

private:
    pair<bool, shared_ptr<Result>> openTrustLine(shared_ptr<Command> commandPointer);

    pair<bool, shared_ptr<Result>> closeTrustLine(shared_ptr<Command> commandPointer);

    pair<bool, shared_ptr<Result>> updateTrustLine(shared_ptr<Command> commandPointer);

    pair<bool, shared_ptr<Result>> maximalTransactionAmount(shared_ptr<Command> commandPointer);

    pair<bool, shared_ptr<Result>> useCredit (shared_ptr<Command> commandPointer);

    string currentTimestamp();

    const uint16_t resultCode(const uuids::uuid &commandUUID);
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
