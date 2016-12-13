#ifndef GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
#define GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H

#include <string>
#include "../commands/OpenTrustLineCommand.h"
#include "../commands/CloseTrustLineCommand.h"
#include "../commands/UpdateOutgoingTrustAmountCommand.h"
#include "../commands/UseCreditCommand.h"
#include "../results/OpenTrustLineResult.h"
#include "../results/CloseTrustLineResult.h"
#include "../results/UpdateTrustLineResult.h"
#include "../results/PaymentResult.h"

using namespace std;

class TransactionsManager {

private:
    const constexpr char* kTrustLinesOpenIdentifier = "CREATE:contractors/trust-lines";
    const constexpr char* kTrustLinesCloseIdentifier = "REMOVE:contractors/trust-lines";
    const constexpr char* kTrustLinesUpdateIdentifier = "SET:contractors/trust-lines";
    const constexpr char* kTransactionsUseCreditIdentifier = "CREATE:contractors/transations";

public:
    TransactionsManager();

    ~TransactionsManager();

    pair<bool, shared_ptr<Result>> acceptCommand(shared_ptr<Command> commandPointer);

private:
    pair<bool, shared_ptr<Result>> openTrustLine(shared_ptr<Command> commandPointer);

    pair<bool, shared_ptr<Result>> closeTrustLine(shared_ptr<Command> commandPointer);

    pair<bool, shared_ptr<Result>> updateTrustLine(shared_ptr<Command> commandPointer);

    pair<bool, shared_ptr<Result>> useCredit (shared_ptr<Command> commandPointer);

    string currentTimestamp();

    const uint16_t resultCode(const uuids::uuid &commandUUID);
};

#endif //GEO_NETWORK_CLIENT_TRANSACTIONSMANAGER_H
