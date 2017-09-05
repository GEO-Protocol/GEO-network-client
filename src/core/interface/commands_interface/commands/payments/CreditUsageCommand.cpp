#include "CreditUsageCommand.h"


CreditUsageCommand::CreditUsageCommand(
    const CommandUUID &uuid,
    const string &commandBuffer) :

    BaseUserCommand(
        uuid,
        identifier())
{
    parse(commandBuffer);
}

CreditUsageCommand::CreditUsageCommand(
    BytesShared buffer) :

    BaseUserCommand(identifier())
{
    deserializeFromBytes(buffer);
}

const string& CreditUsageCommand::identifier()
{
    static const string identifier = "CREATE:contractors/transactions";
    return identifier;
}

const NodeUUID& CreditUsageCommand::contractorUUID() const
{
    return mContractorUUID;
}

const TrustLineAmount& CreditUsageCommand::amount() const
{
    return mAmount;
}

/**
 * @throws std::bad_alloc;
 */
pair<BytesShared, size_t> CreditUsageCommand::serializeToBytes()
{
    return make_pair(make_shared<byte>(), 0);

//    auto parentBytesAndCount = BaseUserCommand::serializeToBytes();
//    size_t bytesCount = parentBytesAndCount.second +
//        NodeUUID::kBytesSize +
//        kTrustLineAmountBytesCount +
//        mReason.size();
//    BytesShared dataBytesShared = tryCalloc(bytesCount);
//    size_t dataBytesOffset = 0;
//    //----------------------------------------------------
//    memcpy(
//        dataBytesShared.get(),
//        parentBytesAndCount.first.get(),
//        parentBytesAndCount.second
//    );
//    dataBytesOffset += parentBytesAndCount.second;
//    //----------------------------------------------------
//    memcpy(
//        dataBytesShared.get() + dataBytesOffset,
//        mContractorUUID.data,
//        NodeUUID::kBytesSize);
//    dataBytesOffset += NodeUUID::kBytesSize;
//    //----------------------------------------------------
//    vector<byte> buffer = trustLineAmountToBytes(mAmount);
//    memcpy(
//        dataBytesShared.get() + dataBytesOffset,
//        buffer.data(),
//        buffer.size()
//    );
//    dataBytesOffset += kTrustLineAmountBytesCount;
//    //----------------------------------------------------
//    if (!mReason.empty()) {
//        memcpy(
//            dataBytesShared.get() + dataBytesOffset,
//            mReason.c_str(),
//            mReason.size()
//        );
//    }
//    //----------------------------------------------------
//    return make_pair(
//        dataBytesShared,
//        bytesCount
//    );
}

void CreditUsageCommand::deserializeFromBytes(
    BytesShared buffer)
{
    BaseUserCommand::deserializeFromBytes(buffer);
    size_t bytesBufferOffset = kOffsetToInheritedBytes();
    //----------------------------------------------------
    memcpy(
        mContractorUUID.data,
        buffer.get() + bytesBufferOffset,
        NodeUUID::kBytesSize
    );
    bytesBufferOffset += NodeUUID::kBytesSize;
    //----------------------------------------------------
    vector<byte> amountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);

    mAmount = bytesToTrustLineAmount(amountBytes);
    bytesBufferOffset += kTrustLineAmountBytesCount;
}

void CreditUsageCommand::parse(
    const string &command)
{
    static const auto minCommandLength = CommandUUID::kHexSize + 2;
    if (command.size() < minCommandLength) {
        throw ValueError(
            "CreditUsageCommand::parse: "
                "can't parse command. "
                    "Received command is too short.");
    }

    try {
        string hexUUID = command.substr(0, CommandUUID::kHexSize);
        mContractorUUID = boost::lexical_cast<uuids::uuid>(hexUUID);

    } catch (...) {
        throw ValueError(
            "CreditUsageCommand::deserialize: "
                "Can't parse command. Error occurred while parsing 'Contractor UUID' token.");
    }

    for (size_t i = NodeUUID::kHexSize+1; i < command.length(); ++i) {
        if (command.at(i) == kTokensSeparator ||
            command.at(i) == kCommandsSeparator ||
            i == command.length()-1) {

            try {
                mAmount = TrustLineAmount(
                    command.substr(
                        NodeUUID::kHexSize+1,
                        i - NodeUUID::kHexSize-1));

            } catch (...) {
                throw ValueError(
                    "CreditUsageCommand::deserialize: "
                        "Can't parse command. Error occurred while parsing 'Amount' token.");
            }

            if (mAmount == TrustLineAmount(0)) {
                throw ValueError(
                    "CreditUsageCommand::deserialize: "
                        "Received 'Amount' can't be 0.");
            }

            // Command amount parsed well
            break;
        }
    }
}
// TODO: deprecated
//const size_t CreditUsageCommand::kMinRequestedBufferSize()
//{
//    static const size_t size =
//            kOffsetToInheritedBytes()
//            + NodeUUID::kBytesSize
//            + kTrustLineAmountBytesCount;
//
//    return size;
//}

CommandResult::SharedConst CreditUsageCommand::responseNoConsensus () const
{
    return makeResult(409);
}

CommandResult::SharedConst CreditUsageCommand::responseOK(
    string &transactionUUID) const
{
    return make_shared<const CommandResult>(identifier(), UUID(), 201, transactionUUID);
}