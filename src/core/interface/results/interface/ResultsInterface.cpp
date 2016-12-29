#include "ResultsInterface.h"

ResultsInterface::ResultsInterface(Logger *logger) :
    mLog(logger){

    if (!isFIFOExists()) {
        createFIFO(kPermissionsMask);
    }
    mFIFODescriptor = 0;
}

ResultsInterface::~ResultsInterface() {
    if (mFIFODescriptor != 0)
        close(mFIFODescriptor);
}

void ResultsInterface::writeResult(
    const char *bytes,
    const size_t bytesCount) {

#ifdef DEBUG_RESULTS_INTERFACE
    mLog->logInfo("Results interface: message received", bytes);
#endif

    if (mFIFODescriptor == 0){
        mFIFODescriptor = open(FIFOFilePath().c_str(), O_WRONLY | O_RSYNC | O_DSYNC);
        if (mFIFODescriptor == -1) {
            throw IOError(
                "ResultsInterface::ResultsInterface: "
                    "Can't open FIFO file.");
        }
    }

    if (write(mFIFODescriptor, bytes, bytesCount) != bytesCount) {
        throw IOError(
            "ResultsInterface::writeResult: "
                "can't write result to the disk.");
    }
}

const char *ResultsInterface::FIFOname() const {
    return kFIFOName;
}

