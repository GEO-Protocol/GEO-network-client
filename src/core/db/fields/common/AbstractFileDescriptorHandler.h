#ifndef GEO_NETWORK_CLIENT_COMMON_H
#define GEO_NETWORK_CLIENT_COMMON_H


#include "../../../common/exceptions/IOError.h"

#include <boost/filesystem.hpp>

#include <string>
#include <cstdio>
#include <sys/stat.h>


namespace db {


using namespace std;
namespace fs = boost::filesystem;


class AbstractFileDescriptorHandler {
public:
    static const constexpr char *kReadAccessMode  = "r+";
    static const constexpr char *kWriteAccessMode = "w+";

public:
    AbstractFileDescriptorHandler(
        const fs::path &path);
    ~AbstractFileDescriptorHandler();

    const fs::path& filename() const;
    const fs::path& path() const;
    virtual void open(
        const char *accessMode);

    void close();
    const bool exists() const;
    __off_t fileSize() const;

    void syncLowLevelOSBuffers() const;

protected:
    fs::path mDirPath;
    fs::path mFilename;
    FILE* mFileDescriptor;
};


} // namespace db


#endif //GEO_NETWORK_CLIENT_COMMON_H
