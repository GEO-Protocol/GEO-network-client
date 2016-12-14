#include <chrono>
#include "TransactionsManager.h"

TransactionsManager::TransactionsManager() {}

TransactionsManager::~TransactionsManager() {}

pair<bool, shared_ptr<Result>> TransactionsManager::acceptCommand(shared_ptr<Command> commandPointer) {
    if (commandPointer.get()->identifier() == string(kTrustLinesOpenIdentifier)) {
        return openTrustLine(commandPointer);
    }

    if (commandPointer.get()->identifier() == string(kTrustLinesCloseIdentifier)) {
        return closeTrustLine(commandPointer);
    }

    if (commandPointer.get()->identifier() == string(kTrustLinesUpdateIdentifier)) {
        return updateTrustLine(commandPointer);
    }

    if (commandPointer.get()->identifier() == string(kTransactionsUseCreditIdentifier)) {
        return useCredit(commandPointer);
    }

    if (commandPointer.get()->identifier() == string(kTransactionsMaximalAmountIdentifier)) {
        return maximalTransactionAmount(commandPointer);
    }

    return pair<bool, shared_ptr<Result>> (false, nullptr);
}

pair<bool, shared_ptr<Result>> TransactionsManager::openTrustLine(shared_ptr <Command> commandPointer) {
    OpenTrustLineCommand *openTrustLineCommand = dynamic_cast<OpenTrustLineCommand *>(commandPointer.get());

    Result *result = new OpenTrustLineResult(commandPointer.get(),
                                             resultCode(commandPointer.get()->commandsUUID()),
                                             currentTimestamp(),
                                             openTrustLineCommand->contractorUUID(),
                                             openTrustLineCommand->amount());

    return pair<bool, shared_ptr<Result>> (true, shared_ptr<Result>(result));
}

pair<bool, shared_ptr<Result>> TransactionsManager::closeTrustLine(shared_ptr <Command> commandPointer) {
    CloseTrustLineCommand *closeTrustLineCommand = dynamic_cast<CloseTrustLineCommand *>(commandPointer.get());

    Result *result = new CloseTrustLineResult(commandPointer.get(),
                                              resultCode(commandPointer.get()->commandsUUID()),
                                              currentTimestamp(),
                                              closeTrustLineCommand->contractorUUID());

    return pair<bool, shared_ptr<Result>> (true, shared_ptr<Result>(result));
}

pair<bool, shared_ptr<Result>> TransactionsManager::updateTrustLine(shared_ptr <Command> commandPointer) {
    UpdateOutgoingTrustAmountCommand *updateTrustLineCommand = dynamic_cast<UpdateOutgoingTrustAmountCommand *>(commandPointer.get());

    Result *result = new UpdateTrustLineResult(commandPointer.get(),
                                               resultCode(commandPointer.get()->commandsUUID()),
                                               currentTimestamp(),
                                               updateTrustLineCommand->contractorUUID(),
                                               updateTrustLineCommand->amount());

    return pair<bool, shared_ptr <Result>> (true, shared_ptr<Result>(result));
}

pair<bool, shared_ptr<Result>> TransactionsManager::maximalTransactionAmount(shared_ptr <Command> commandPointer) {
    MaximalTransactionAmountCommand *maximalTransactionAmountCommand = dynamic_cast<MaximalTransactionAmountCommand *>(commandPointer.get());

    Result *result = new MaximalTransactionAmountResult(commandPointer.get(),
                                                        resultCode(commandPointer.get()->commandsUUID()),
                                                        currentTimestamp(),
                                                        maximalTransactionAmountCommand->contractorUUID(),
                                                        500);

    return pair<bool, shared_ptr<Result>> (true, shared_ptr<Result>(result));
}

pair<bool, shared_ptr<Result>> TransactionsManager::useCredit(shared_ptr <Command> commandPointer) {
    UseCreditCommand *useCreditCommand = dynamic_cast<UseCreditCommand *>(commandPointer.get());

    Result *result = new PaymentResult(commandPointer.get(),
                                       resultCode(commandPointer.get()->commandsUUID()),
                                       currentTimestamp(),
                                       commandPointer.get()->commandsUUID(), //let's imagine that this is transaction uuid
                                       useCreditCommand->contractorUUID(),
                                       useCreditCommand->amount(),
                                       useCreditCommand->purpose());

    return pair<bool, shared_ptr<Result>> (true, shared_ptr<Result>(result));
}

string TransactionsManager::currentTimestamp() {
    long timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    return to_string(timestamp);
}

const uint16_t TransactionsManager::resultCode(const uuids::uuid &commandUUID) {
    uint16_t resultCode = 0;
    string commandUUIDAsStr = boost::lexical_cast<string>(commandUUID);
    char controlCharacter = commandUUIDAsStr.at(commandUUIDAsStr.size() - 1);
    if (controlCharacter == '0') {
        resultCode = 200;
    }
    if (controlCharacter == '1') {
        resultCode = 404;
    }
    if (controlCharacter == '2') {
        resultCode = 409;
    }
    if (controlCharacter == '3') {
        resultCode = 501;
    }
    return resultCode;
}

