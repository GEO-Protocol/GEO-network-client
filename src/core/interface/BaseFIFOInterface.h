//#ifndef GEO_NETWORK_CLIENT_BASEINTERFACE_H
//#define GEO_NETWORK_CLIENT_BASEINTERFACE_H
//
//#include <boost/filesystem.hpp>
//
//#include <string>
//
//
//using namespace std;
//namespace fs = boost::filesystem;
//
//class BaseFIFOInterface {
//protected:
//    virtual const char* dir() const {
//        return "fifo/";
//    }
//
//    virtual const char* name() const {
//        return "commands.fifo";
//    }
//
//    const string FIFOPath() const {
//        return (string(dir()) + string(name())).c_str();
//    }
//
//    const bool FIFOExists() const {
//        return fs::exists(fs::path(FIFOPath()));
//    }
//
//protected:
//    int mFIFODescriptor;
//};
//
//#endif //GEO_NETWORK_CLIENT_BASEINTERFACE_H
