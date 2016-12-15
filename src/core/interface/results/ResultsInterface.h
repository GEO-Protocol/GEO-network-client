#ifndef GEO_NETWORK_CLIENT_RESULTSINTERFACE_H
#define GEO_NETWORK_CLIENT_RESULTSINTERFACE_H

#include <string>
#include <boost/filesystem.hpp>
#include "../../common/exceptions/IOError.h"

using namespace std;

namespace fs = boost::filesystem;

typedef uint8_t byte;

class ResultsInterface{
private:
    FILE *mFileDescriptor;
    string mFileName = "results.bin";

    const string kModeCreate = "w+";
    const string kModeUpdate = "r+";

public:
    ResultsInterface();

    ~ResultsInterface();

    void writeResult(string &result);

private:

    void obtainFileDescriptor();

    void checkFileDescriptor();

    const bool isFileExists();

};

#endif //GEO_NETWORK_CLIENT_RESULTSINTERFACE_H
