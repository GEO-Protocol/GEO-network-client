#ifndef GEO_NETWORK_CLIENT_BACKUPS_H
#define GEO_NETWORK_CLIENT_BACKUPS_H


#include "../io/storage/StorageHandler.h"

#include <boost/signals2.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <boost/asio/steady_timer.hpp>


using namespace std;

namespace as = boost::asio;
namespace signals = boost::signals2;

class BackupDelayedTasks {
public:
    BackupDelayedTasks(as::io_service &IOService);

    signals::signal<void()> mBackupSignal;

public:
    void MakeDBAndOperationFileBackup(const boost::system::error_code &error);

protected:
    as::io_service &mIOService;
    const int mBackupSignalRepeatTimeSeconds = 5 * 60 * 60;

    unique_ptr<as::steady_timer> mBackupTimer;
};


#endif //GEO_NETWORK_CLIENT_BACKUPS_H
