#include "Backup.h"

BackupDelayedTasks::BackupDelayedTasks(
        as::io_service &IOService):
    mIOService(IOService)
{
    srand(time(NULL));
    int TimeStarted = rand() % 20;
    mBackupTimer = make_unique<as::steady_timer>(
            mIOService);
    mBackupTimer->expires_from_now(std::chrono::seconds(TimeStarted));
    mBackupTimer->async_wait(boost::bind(
            &BackupDelayedTasks::MakeDBAndOperationFileBackup,
            this,
            as::placeholders::error
    ));
}

void BackupDelayedTasks::MakeDBAndOperationFileBackup(const boost::system::error_code &error)
{
    if (error) {
        cout << error.message() << endl;
    }
    mBackupTimer->cancel();
    mBackupTimer->expires_from_now(std::chrono::seconds(mBackupSignalRepeatTimeSeconds));
    mBackupTimer->async_wait(boost::bind(
            &BackupDelayedTasks::MakeDBAndOperationFileBackup,
            this,
            as::placeholders::error
    ));
    mBackupSignal();
}
