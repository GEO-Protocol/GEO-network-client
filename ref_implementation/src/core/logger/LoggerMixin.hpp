/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef LOGGERMIXIN_H
#define LOGGERMIXIN_H

#include "Logger.h"


/**
 * This class implements simlpe logging extension
 * for the classes, to which it would be mixed.
 */
class LoggerMixin {
public:
    LoggerMixin(
        Logger &logger)
        noexcept:
        mLog(logger){}

    virtual const string logHeader() const = 0;

    LoggerStream info() const
        noexcept {
        return mLog.info(logHeader());
    }

    LoggerStream warning() const
        noexcept {
        return mLog.warning(logHeader());
    }

    LoggerStream debug() const
        noexcept {
        return mLog.debug(logHeader());
    }

protected:
    Logger &mLog;
};

#endif // LOGGERMIXIN_H
