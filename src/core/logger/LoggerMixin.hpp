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

    LoggerStream error() const
        noexcept {
        return mLog.error(logHeader());
    }

    LoggerStream debug() const
        noexcept {
        return mLog.debug(logHeader());
    }

protected:
    Logger &mLog;
};

#endif // LOGGERMIXIN_H
