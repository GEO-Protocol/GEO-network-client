#ifndef LOGGERMIXIN_H
#define LOGGERMIXIN_H

#include "Logger.h"


/**
 * This class implements simple logging extension
 * for the classes, to which it would be mixed.
 */
class LoggerMixin {
public:
    LoggerMixin(
        Logger &logger):
        mLog(logger){}

    virtual const string logHeader() const = 0;

    LoggerStream info() const {
        return mLog.info(logHeader());
    }

    LoggerStream warning() const {
        return mLog.warning(logHeader());
    }

    LoggerStream debug() const {
        return mLog.debug(logHeader());
    }

protected:
    Logger &mLog;
};

#endif // LOGGERMIXIN_H
