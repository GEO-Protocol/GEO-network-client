#include "InitTrustLineCommand.h"
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/include/qi_no_skip.hpp>
InitTrustLineCommand::InitTrustLineCommand(
        const CommandUUID &commandUUID,
        const string &command) :

        BaseUserCommand(
                commandUUID,
                identifier()) {

    using boost::spirit::x3::int_;
    using boost::spirit::x3::char_;
    using boost::spirit::x3::_attr;
    using boost::spirit::x3::ascii::space;

    auto first = command.begin(), last = command.end();
    std::string addressStr("");
    uint32_t addressType, addressCunt, equiv;
    auto adr_type = [&](auto &ctx) { addressType += _attr(ctx); };
    auto adr_str = [&](auto &ctx) { addressStr += _attr(ctx); };
    auto adr_cnt = [&](auto &ctx) { addressCunt += _attr(ctx); };
    auto equiv_add = [&](auto &ctx) { equiv += _attr(ctx); };

    static const auto minCommandLength = 7;

    if (command.size() < minCommandLength) {
        throw ValueError(
                "InitTrustLineCommand: can't parse command. "
                "Received command is to short.");
    }

    parse(first, last,
          (
                  +(int_[adr_cnt] - space) >> space
                                           >> +(int_[adr_type] - "-") >> "-"
                                           >> +(char_[adr_str] - space) >> space
                                           >> +(int_[equiv_add] - space)
          )
    );

    mContractorAddressesCount = addressCunt;
    mContractorAddresses.reserve(mContractorAddressesCount);
    switch (addressType) {
        case BaseAddress::IPv4_IncludingPort: {
            mContractorAddresses.push_back(
                    make_shared<IPv4WithPortAddress>(
                            addressStr));
            break;
        }
        default:
            throw ValueError(
                    "InitTrustLineCommand: can't parse command. "
                    "Error occurred while parsing 'Contractor Address' token.");
    }
    mEquivalent = equiv;
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
