#ifndef GEO_NETWORK_CLIENT_RESULTSINTERFACE_H
#define GEO_NETWORK_CLIENT_RESULTSINTERFACE_H

namespace fs = boost::filesystem;

class ResultsInterface: public BaseFIFOInterface {
private:
    void createFifo();
};

#endif //GEO_NETWORK_CLIENT_RESULTSINTERFACE_H
