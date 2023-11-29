#ifndef RECORDERMAGISTER_H
#define RECORDERMAGISTER_H

#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <future>
#include <utu/log.h>
#include <fstream>
#include "config/RecorderConfig.h"

#include <uhd.h>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/usrp_clock/usrp_clock.h>
#include <uhd/usrp_clock/multi_usrp_clock.hpp>
#include <uhd/types/device_addr.hpp>
#include <uhd/types/tune_request.hpp>
#include <uhd/usrp/subdev_spec.hpp>
#include <uhd/exception.hpp>
#include <uhd/utils/thread.hpp>

#include "FileOut.h"

namespace Magister{

class RecorderMagister
{
public:

    RecorderMagister(std::atomic<bool> *finishFlag, FileOut<std::complex<int16_t>>* fileWritter);
    ~RecorderMagister();

private:
    std::vector<uhd::usrp::multi_usrp::sptr> usrp_;
    std::vector<uhd::rx_streamer::sptr> rxStream_;
    std::vector<std::vector<std::shared_ptr<RxData<std::complex<int16_t>>>>> dataBuffer_;
    std::vector<std::shared_ptr<SafeQueue<RxData<std::complex<int16_t>>>>> fSyncQueue_;

    std::atomic<bool> *finishFlag_;

    std::vector<std::future<void>> futures_;
    std::vector<std::future<void>> demonicFutures_;
    uhd::usrp_clock::multi_usrp_clock::sptr octoclock;

    void setupRxDev(const config::Usrp& conf,
                    const config::SyncSource& syncSource,
                    std::vector<std::vector<std::shared_ptr<RxData<std::complex<int16_t>>>>>& buffer,
                    const unsigned numBuffers);
    void start(const config::Recorder& conf);
    void reconfigure(const config::Recorder& conf);
    void reconfigureRx(const config::Recorder& conf);

    void recorderThread(std::atomic<bool> *finishFlag, unsigned strmId, unsigned dataPartSize, double FsRxd, unsigned chNum);
    void configThread(std::atomic<bool> *finishFlag);
    void fileDataSynchronizer(std::atomic<bool> *finishFlag, FileOut<std::complex<int16_t>>* fileWritter);

};

}

#endif // RECORDERMAGISTER_H_INCLUDED
