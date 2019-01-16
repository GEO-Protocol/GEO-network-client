#include "InitTrustLineCommand.h"

InitTrustLineCommand::InitTrustLineCommand(
        const CommandUUID &commandUUID,
        const string &command) :

        BaseUserCommand(
                commandUUID,
                identifier()) {

    std::string address("");
    uint32_t addressType, addressCount, equivalentID;
    auto address_Type = [&](auto &ctx) { addressType += _attr(ctx); };
    auto address_add = [&](auto &ctx) { address += _attr(ctx); };
    auto address_Count = [&](auto &ctx) { addressCount += _attr(ctx); };
    auto equivalentID_add = [&](auto &ctx) { equivalentID += _attr(ctx); };

    parse(command.begin(), command.end(),
          (
                  +(int_[address_Count] - space) >> space
                  >> +(int_[address_Type] - "-") >> "-"
                  >> +(char_[address_add ] - space) >> space
                  >> +(int_[equivalentID_add] - space)
          )
    );

    mContractorAddresses.reserve(addressCount);
    switch (addressType) {
        case BaseAddress::IPv4_IncludingPort: {
            mContractorAddresses.push_back(
                    make_shared<IPv4WithPortAddress>(
                            address));
            break;
        }
        default:
            throw ValueError(
                    "InitTrustLineCommand: can't parse command. "
                    "Error occurred while parsing 'Contractor Address' token.");
    }
    mEquivalent = equivalentID;
}

const string &InitTrustLineCommand::identifier()
noexcept
{
    static const string identifier = "INIT:contractors/trust-line";
    return identifier;
}

const SerializedEquivalent InitTrustLineCommand::equivalent() const
noexcept
{
    return mEquivalent;
}

vector<BaseAddress::Shared> InitTrustLineCommand::contractorAddresses() const
noexcept
{
    return mContractorAddresses;
}
