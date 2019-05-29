#ifndef GEO_NETWORK_CLIENT_HISTORYWITHCONTRACTORCOMMAND_H
#define GEO_NETWORK_CLIENT_HISTORYWITHCONTRACTORCOMMAND_H

#include "../BaseUserCommand.h"

class HistoryWithContractorCommand : public BaseUserCommand {

public:
    typedef shared_ptr<HistoryWithContractorCommand> Shared;

public:
    HistoryWithContractorCommand(
        const CommandUUID &uuid,
        const string &commandBuffer);

    static const string &identifier();

    CommandResult::SharedConst resultOk(
        string &historyWithContractorStr) const;

    const size_t historyFrom() const;

    const size_t historyCount() const;

    const vector<BaseAddress::Shared> &contractorAddresses() const;

    const SerializedEquivalent equivalent() const;

private:
    size_t mHistoryFrom;
    size_t mHistoryCount;
    vector<BaseAddress::Shared> mContractorAddresses;
    SerializedEquivalent mEquivalent;
};

#endif //GEO_NETWORK_CLIENT_HISTORYWITHCONTRACTORCOMMAND_H
