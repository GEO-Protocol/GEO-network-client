#include "VisualInterface.h"

VisualInterface::VisualInterface(
    Logger &logger) :

    mLog(logger)
{
    if (!isFIFOExists()) {
        createFIFO(kPermissionsMask);
    }
    mFIFODescriptor = 0;
    signal(SIGPIPE, SIG_IGN);
}

VisualInterface::~VisualInterface()
{
    if (mFIFODescriptor != 0) {
        close(mFIFODescriptor);
    }
    if (remove(FIFOFilePath().c_str()) != 0) {
        mLog.warning("visual.fifo didn't delete");
    }
}

void VisualInterface::writeResult(
    const char *bytes,
    const size_t bytesCount)
{
    if (mFIFODescriptor == 0){
#ifdef LINUX
        mFIFODescriptor = open(
            FIFOFilePath().c_str(),
            O_WRONLY | O_RSYNC | O_DSYNC);
#endif

        if (mFIFODescriptor == -1) {
            throw IOError(
                    "VisualInterface::writeResult: "
                        "Can't open FIFO file.");
        }
    }
    if (write(mFIFODescriptor, bytes, bytesCount) != bytesCount) {
        close(mFIFODescriptor);

#ifdef LINUX
        mFIFODescriptor = open(
            FIFOFilePath().c_str(),
            O_WRONLY | O_RSYNC | O_DSYNC);
#endif

        if (mFIFODescriptor == -1) {
            throw IOError(
                    "VisualInterface::writeResult: "
                        "Can't open FIFO file.");
        }

        if (write(mFIFODescriptor, bytes, bytesCount) != bytesCount) {
            throw IOError(
                    "VisualInterface::writeResult: "
                        "Can't write result to the disk.");
        }
    }
}

const char *VisualInterface::FIFOName() const
{
    return kFIFOName;
}