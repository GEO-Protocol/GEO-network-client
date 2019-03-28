#ifndef GEO_NETWORK_CLIENT_COMMAND_H
#define GEO_NETWORK_CLIENT_COMMAND_H

#include "../../../common/memory/MemoryUtils.h"
#include "../../../common/exceptions/ValueError.h"
#include "../../results_interface/result/CommandResult.h"
#include "../../../contractors/addresses/IPv4WithPortAddress.h"

#include <boost/uuid/uuid.hpp>
#include <boost/spirit/home/x3.hpp>

using boost::spirit::x3::int_;
using boost::spirit::x3::char_;
using boost::spirit::x3::_attr;
using boost::spirit::x3::repeat;
using boost::spirit::x3::lexeme;
using boost::spirit::x3::eol;
using boost::spirit::x3::eoi;
using boost::spirit::x3::ascii::space;
using boost::spirit::x3::ascii::digit;
using boost::spirit::x3::ascii::alpha;
using boost::spirit::x3::ascii::punct;
using boost::spirit::x3::expect;

namespace parserString = boost::spirit::x3;

namespace uuids = boost::uuids;

class BaseUserCommand {
public:
    typedef shared_ptr<BaseUserCommand> Shared;
public:
    BaseUserCommand(
        const string& identifier);

    BaseUserCommand(
        const CommandUUID &commandUUID,
        const string& identifier);

    virtual ~BaseUserCommand() = default;

    const CommandUUID &UUID() const;

    const string &identifier() const;

    template<typename addChar, typename addNumber, typename addType, typename addToVector>
    inline auto addressLexeme(size_t mContractorAddressesCount, addChar addressChar,
                              addNumber addressNumber, addType addresType, addToVector addressToVector) {
        return lexeme[expect[
            repeat(mContractorAddressesCount)
            [
                parserString::string(std::to_string(BaseAddress::IPv4_IncludingPort))[addresType]
                > *(char_[addresType] - char_(kTokensSeparator))
                > char_(kTokensSeparator)
                > repeat(3)
                [
                    int_[addressNumber]
                    > char_('.')[addressChar]
                ]
                > int_[addressNumber]
                > char_(':')[addressChar]
                > int_[addressNumber]
                > (char_(kTokensSeparator)|eol)[addressToVector]

//                                         | //OR
//
//                  parserString::string(std::to_string(<NEW_ADDRESS_TYPE>) [addressTypeParse]
//                  > *(char_[addressTypeParse] -char_(kTokensSeparator))
//                  > char_(kTokensSeparator)
//                  > <NEW_PARSE_RULE>
                ]
        ]];
    }

    template<typename UUID8Digits, typename UUID4Digits, typename UUID12Digits>
    inline auto UUIDLexeme(UUID8Digits addUUID8Digits, UUID4Digits addUUID4Digits, UUID12Digits addUUID12Digits) {
        return lexeme[*(char_[addUUID8Digits] - char_(kUUIDSeparator)) > char_(kUUIDSeparator)[addUUID8Digits]
        > *(char_[addUUID4Digits] - char_(kUUIDSeparator)) > char_(kUUIDSeparator)[addUUID4Digits]
        > *(char_[addUUID4Digits] - char_(kUUIDSeparator)) > char_(kUUIDSeparator)[addUUID4Digits]
        > *(char_[addUUID4Digits] - char_(kUUIDSeparator)) > char_(kUUIDSeparator)[addUUID4Digits]
        > *(char_[addUUID12Digits] - char_(kTokensSeparator)) > char_(kTokensSeparator)[addUUID12Digits]];
    }
    // TODO: remove noexcept
    // TODO: split methods into classes
    CommandResult::SharedConst responseOK() const
        noexcept;
    CommandResult::SharedConst responseCreated() const
        noexcept;
    CommandResult::SharedConst responsePostponedByReservations() const
        noexcept;
    CommandResult::SharedConst responseProtocolError() const
        noexcept;
    CommandResult::SharedConst responseTrustLineIsAbsent() const
        noexcept;
    CommandResult::SharedConst responseThereAreNoKeys() const
        noexcept;
    CommandResult::SharedConst responseNoConsensus() const
        noexcept;
    CommandResult::SharedConst responseInsufficientFunds() const
        noexcept;
    CommandResult::SharedConst responseInsufficientFundsDueToKeysAbsent() const
        noexcept;
    CommandResult::SharedConst responseInsufficientFundsDueToParticipantsKeysAbsent() const
        noexcept;
    CommandResult::SharedConst responseConflictWithOtherOperation() const
        noexcept;
    CommandResult::SharedConst responseRemoteNodeIsInaccessible() const
        noexcept;
    CommandResult::SharedConst responseNoRoutes() const
        noexcept;
    CommandResult::SharedConst responseUnexpectedError() const
        noexcept;
    // this response used during disabling start payment and trust line transactions
    CommandResult::SharedConst responseForbiddenRunTransaction() const
        noexcept;

    CommandResult::SharedConst responseEquivalentIsAbsent() const
        noexcept;

protected:
    CommandResult::SharedConst makeResult(
        const uint16_t code) const;

private:
    const CommandUUID mCommandUUID;
    const string mCommandIdentifier;
};

#endif //GEO_NETWORK_CLIENT_COMMAND_H
