#ifndef GEO_NETWORK_CLIENT_FILELOGGER_H
#define GEO_NETWORK_CLIENT_FILELOGGER_H

#include <cstdio>
#include <string>

#include <boost/filesystem.hpp>

#include "../common/exceptions/IOError.h"

using namespace std;

namespace fs = boost::filesystem;

class FileLogger {

private:
    const string kModeCreate = "w+";
    const string kModeUpdate = "r+";

private:
    FILE *mFileDescriptor;
    string kFileName = "logs/system_log.txt";

private:
    const bool isFileExist(){
        return fs::exists(fs::path(kFileName.c_str()));
    }

    void checkFileDescriptor(bool wasExist) {
        if (mFileDescriptor == NULL) {
            throw IOError(string("Unable to obtain file descriptor.").c_str());
        }
        if (wasExist) {
            fseek(mFileDescriptor, 0, SEEK_END);
            fputs("\n\n\n", mFileDescriptor);
            fflush(mFileDescriptor);
        }
    }

    void obtainFileDescriptor() {
        if (isFileExist()) {
            mFileDescriptor = fopen(kFileName.c_str(), kModeUpdate.c_str());
            checkFileDescriptor(true);
        } else {
            mFileDescriptor = fopen(kFileName.c_str(), kModeCreate.c_str());
            checkFileDescriptor(false);
        }
    }

public:
    FileLogger(){
        obtainFileDescriptor();
    }

    ~FileLogger(){
        if (mFileDescriptor != NULL) {
            fclose(mFileDescriptor);
        }
    }

    void writeLine(const char *sentence){
        fputs(sentence, mFileDescriptor);
        fflush(mFileDescriptor);
    }

    void writeEmptyLine(size_t count){
        for (size_t i=0; i<count; i++) {
            fputs("\n", mFileDescriptor);
        }
        fflush(mFileDescriptor);
    }

};

#endif //GEO_NETWORK_CLIENT_FILELOGGER_H
