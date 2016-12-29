#include "OpenTrustLineTransaction.h"

OpenTrustLineTransaction::OpenTrustLineTransaction(
        OpenTrustLineCommand::Shared command) :
        BaseTransaction(BaseTransaction::TransactionType::OpenTrustLineTransaction), mCommand(command) {}

OpenTrustLineCommand::Shared OpenTrustLineTransaction::command() const {
    return mCommand;
}

void OpenTrustLineTransaction::setContext(
        Message::Shared message) {
    BaseTransaction::mContext = message;
}

pair<CommandResult::SharedConst, TransactionState::SharedConst> OpenTrustLineTransaction::run() {
    return make_pair(CommandResult::SharedConst(mCommand.get()->resultOk()),
                     nullptr);
}

pair<byte *, size_t> OpenTrustLineTransaction::serializeContext() {
    auto transactionType = (SerializedTransactionType)mType;
    size_t transactionTypeSize = sizeof(transactionType);

    size_t dataBlockSize = transactionTypeSize + TransactionUUID::kUUIDSize + CommandUUID::kUUIDSize + NodeUUID::kUUIDSize + TrustLinesManager::kTrustAmountPartSize;
    byte *data = (byte *) malloc(dataBlockSize);
    memset(data, 0, dataBlockSize);

    memcpy(data, &transactionType, transactionTypeSize);
    memcpy(data + transactionTypeSize, mTransactionUUID.data, TransactionUUID::kUUIDSize);
    memcpy(data + transactionTypeSize + TransactionUUID::kUUIDSize, mCommand.get()->uuid().data, CommandUUID::kUUIDSize);
    memcpy(data + transactionTypeSize + TransactionUUID::kUUIDSize + CommandUUID::kUUIDSize, mCommand.get()->contractorUUID().data, NodeUUID::kUUIDSize);

    vector<byte> trustAmountByteSet;
    export_bits(static_cast<boost::multiprecision::checked_uint256_t>(mCommand.get()->amount()), back_inserter(trustAmountByteSet), 8);
    size_t trustAmountByteSetSize = trustAmountByteSet.size();
    for (unsigned long i = 0; i < TrustLinesManager::kTrustAmountPartSize - trustAmountByteSetSize; ++i) {
        trustAmountByteSet.push_back(0);
    }

    memcpy(data + transactionTypeSize + TransactionUUID::kUUIDSize + CommandUUID::kUUIDSize + NodeUUID::kUUIDSize, trustAmountByteSet.data(), TrustLinesManager::kTrustAmountPartSize);

    return make_pair(data, dataBlockSize);
}
