#ifndef GEO_NETWORK_CLIENT_CONFLICTRESOLVERMESSAGE_H
#define GEO_NETWORK_CLIENT_CONFLICTRESOLVERMESSAGE_H

#include "../base/transaction/TransactionMessage.h"
#include "../../../io/storage/record/audit/AuditRecord.h"
#include "../../../io/storage/record/audit/ReceiptRecord.h"

class ConflictResolverMessage : public TransactionMessage {

public:
    typedef shared_ptr<ConflictResolverMessage> Shared;

public:
    ConflictResolverMessage(
        const SerializedEquivalent equivalent,
        const NodeUUID &senderUUID,
        const TransactionUUID &transactionUUID,
        AuditRecord::Shared auditRecord,
        vector<ReceiptRecord::Shared> &incomingReceipts,
        vector<ReceiptRecord::Shared> &outgoingReceipts);

    ConflictResolverMessage(
        BytesShared buffer);

    AuditRecord::Shared auditRecord() const;

    const vector<ReceiptRecord::Shared> incomingReceipts() const;

    const vector<ReceiptRecord::Shared> outgoingReceipts() const;

    const MessageType typeID() const;

    const bool isAddToConfirmationRequiredMessagesHandler() const;

    const bool isCheckCachedResponse() const override;

    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

private:
    AuditRecord::Shared mAuditRecord;
    vector<ReceiptRecord::Shared> mIncomingReceipts;
    vector<ReceiptRecord::Shared> mOutgoingReceipts;
};


#endif //GEO_NETWORK_CLIENT_CONFLICTRESOLVERMESSAGE_H
