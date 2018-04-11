#ifndef GEO_NETWORK_CLIENT_VISUALINTERFACE_H
#define GEO_NETWORK_CLIENT_VISUALINTERFACE_H

#include "../../BaseFIFOInterface.h"
#include "../../../logger/Logger.h"

class VisualInterface : public BaseFIFOInterface {

public:
    explicit VisualInterface(
        Logger &logger);

    ~VisualInterface();

    void writeResult(
        const char *bytes,
        const size_t bytesCount);

private:
    virtual const char* FIFOName() const;

public:
    static const constexpr char *kFIFOName = "visual.fifo";
    static const constexpr unsigned int kPermissionsMask = 0755;

private:
    Logger &mLog;

};


#endif //GEO_NETWORK_CLIENT_VISUALINTERFACE_H
