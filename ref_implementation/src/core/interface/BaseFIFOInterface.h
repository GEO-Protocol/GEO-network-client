/**
 * This file is part of GEO Protocol.
 * It is subject to the license terms in the LICENSE.md file found in the top-level directory
 * of this distribution and at https://github.com/GEO-Protocol/GEO-network-client/blob/master/LICENSE.md
 *
 * No part of GEO Protocol, including this file, may be copied, modified, propagated, or distributed
 * except according to the terms contained in the LICENSE.md file.
 */

#ifndef GEO_NETWORK_CLIENT_BASEINTERFACE_H
#define GEO_NETWORK_CLIENT_BASEINTERFACE_H


#include "../common/exceptions/IOError.h"

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include <string>
#include <iostream>

// for mkfifo()
#include <sys/types.h>
#include <sys/stat.h>


using namespace std;
namespace as = boost::asio;
namespace fs = boost::filesystem;


class BaseFIFOInterface {
public:
    static const constexpr char *kFIFODir = "fifo/";

protected:
    BaseFIFOInterface():
        mFIFODescriptor(0){};

    virtual const char* FIFOName() const = 0;

    const string FIFOFilePath() const {
        return string(kFIFODir) + string(FIFOName());
    }

    const bool isFIFOExists() const {
        return fs::exists(fs::path(FIFOFilePath()));
    }

    void createFIFO(unsigned int permissionsMask) {
        if (! isFIFOExists()) {
            fs::create_directories(kFIFODir);

            if (mkfifo(FIFOFilePath().c_str(), permissionsMask) != 0) {
                throw IOError(
                    "BaseFIFOInterface::createFIFO: "
                        "Can't create FIFO file.");
            }
        }
    }

protected:
    int mFIFODescriptor;
};

#endif //GEO_NETWORK_CLIENT_BASEINTERFACE_H
