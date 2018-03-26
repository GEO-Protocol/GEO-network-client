/**
 * This file is part of GEO Project.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Project/GEO-Project/blob/master/LICENSE.md
 *
 * No part of GEO Project, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_HISTORYWITHCONTRACTORCOMMAND_H
#define GEO_NETWORK_CLIENT_HISTORYWITHCONTRACTORCOMMAND_H

#include "../BaseUserCommand.h"
#include "../../../../common/exceptions/ValueError.h"

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

    const NodeUUID& contractorUUID() const;

private:
    size_t mHistoryFrom;
    size_t mHistoryCount;
    NodeUUID mContractorUUID;
};


#endif //GEO_NETWORK_CLIENT_HISTORYWITHCONTRACTORCOMMAND_H
