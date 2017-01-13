#include "AcceptTrustLineMessage.h"

AcceptTrustLineMessage::AcceptTrustLineMessage(
    byte* buffer) {

    deserialize(buffer);
}

pair<ConstBytesShared, size_t> AcceptTrustLineMessage::serialize() {

    throw NotImplementedError("AcceptTrustLineMessage::serialize: "
                                  "Method not implemented.");
}

void AcceptTrustLineMessage::deserialize(
    byte* buffer) {

    //Message::deserialize(buffer);
    //------------------------------
    memcpy(
        mTransactionUUID.data,
        buffer,
        TransactionUUID::kUUIDSize
    );
    //------------------------------
    vector<byte> amountBytes(
        buffer + TransactionUUID::kUUIDSize,
        buffer + TransactionUUID::kUUIDSize + kTrustLineAmountSize);


    vector<byte> amountNotZeroBytes;
    amountNotZeroBytes.reserve(kTrustLineAmountSize);

    for (auto &item : amountBytes) {
        if (item != 0) {
            amountNotZeroBytes.push_back(item);
        }
    }

    if (amountNotZeroBytes.size() > 0) {
        import_bits(
            mTrustLineAmount,
            amountNotZeroBytes.begin(),
            amountNotZeroBytes.end()
        );

    } else {
        import_bits(
            mTrustLineAmount,
            amountBytes.begin(),
            amountBytes.end()
        );
    }
    //------------------------------
}

const Message::MessageTypeID AcceptTrustLineMessage::typeID() const {

    return Message::MessageTypeID::AcceptTrustLineMessageType;
}
