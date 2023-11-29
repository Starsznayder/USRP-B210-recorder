#include "RecorderMagister.h"

// UHD stuff
#include <unistd.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <kittyLogs/log.h>
#include "config/Config.h"
#include "FileOut.h"

namespace Magister {
//conf.usrp.FsTx/conf.signal.PRF

static const int numRxBuffers = 20; // increase in case of file overflow


bool preCheckAlltimers(std::vector<uhd::usrp::multi_usrp::sptr>& usrp, uhd::time_spec_t gpsTime)
{

    for (int u = 0; u < usrp.size(); ++u)
    {
        if (usrp[u]->get_time_last_pps() < gpsTime)
        {
            return true;
        }
    }
    return false;
}

bool finalCheckAlltimers(std::vector<uhd::usrp::multi_usrp::sptr>& usrp, uhd::time_spec_t gpsTime)
{

    for (int u = 0; u < usrp.size(); ++u)
    {
        if (usrp[u]->get_time_last_pps() != gpsTime)
        {
            return true;
        }
    }
    return false;
}

typedef std::function<uhd::sensor_value_t(const std::string&)> get_sensor_fn_t;

bool check_locked_sensor(std::vector<std::string> sensor_names,
                         const char* sensor_name,
                         get_sensor_fn_t get_sensor_fn,
                         int brdID)
{
    if (std::find(sensor_names.begin(), sensor_names.end(), sensor_name) == sensor_names.end())
    {
        return false;
    }

    bool lock_Ketected = false;

    int cnt = 0;
    while (true)
    {
        if (lock_Ketected)
        {
            _KI("USRP" + std::to_string(brdID), "[INFO][" << sensor_name << "]: locked");
            break;
        }

        ++cnt;
        if (cnt >= 10)
        {
            cnt = 0;
            _KW("USRP" + std::to_string(brdID), boost::format("Waiting for \"%s\": ") % sensor_name);
        }

        if (get_sensor_fn(sensor_name).to_bool())
        {
            lock_Ketected = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return lock_Ketected;
}

void RecorderMagister::start(const config::Recorder& conf)
{
    uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
    if (conf.rec.octoclockAddr != "none" && conf.rec.syncSource == config::SyncSource::GPS)
    {
        octoclock = uhd::usrp_clock::multi_usrp_clock::make(std::string("addr=") + conf.rec.octoclockAddr);

        while(!(octoclock->get_sensor("gps_locked").to_bool()))
        {
            _KI("Recorder", "OCTOCLOCK: " << octoclock->get_sensor("gps_locked").to_pp_string());
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        time_t gps_time;
        do {
            gps_time = octoclock->get_sensor("gps_time").to_int();
            for (int u = 0; u < usrp_.size(); ++u)
            {
                usrp_[u]->set_time_next_pps(uhd::time_spec_t(gps_time + 1));
            }

            _KW("Recorder", "OCTOCLOCK: " << gps_time);

            while (preCheckAlltimers(usrp_, uhd::time_spec_t(gps_time + 1)))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

        } while(finalCheckAlltimers(usrp_, uhd::time_spec_t(gps_time + 1)));

        stream_cmd.num_samps = size_t(0);
        stream_cmd.stream_now = false;
        if (uhd::time_spec_t(static_cast<double>(conf.rec.targetStartTime)) > gps_time)
        {
            stream_cmd.time_spec = uhd::time_spec_t(static_cast<double>(conf.rec.targetStartTime));
        }
        else
        {
            stream_cmd.time_spec = gps_time + static_cast<double>(conf.rec.startOffset);
        }
    }
    else if (conf.rec.octoclockAddr != "none" && conf.rec.syncSource == config::SyncSource::EXTERNAL)
    {
        do {
            for (int u = 0; u < usrp_.size(); ++u)
            {
                usrp_[u]->set_time_next_pps(uhd::time_spec_t(1.0));
            }
        } while(finalCheckAlltimers(usrp_, uhd::time_spec_t(1.0)));

        stream_cmd.num_samps = size_t(0);
        stream_cmd.stream_now = false;
        stream_cmd.time_spec = static_cast<double>(conf.rec.startOffset);
    }
    else if (conf.rec.octoclockAddr == "none" && conf.rec.syncSource == config::SyncSource::GPS)
    {
        for (int u = 0; u < usrp_.size(); ++u)
        {
            uhd::usrp::multi_usrp::sptr currUsrp = usrp_[u];
            check_locked_sensor(currUsrp->get_mboard_sensor_names(0),
                                "gps_locked",
                                [currUsrp](const std::string& sensor_name) {return currUsrp->get_mboard_sensor(sensor_name);},
                                u);
        }

        uhd::usrp::multi_usrp::sptr masterUsrp = usrp_[0];
        uhd::time_spec_t gps_time;
        do {
            gps_time = uhd::time_spec_t(int64_t(masterUsrp->get_mboard_sensor("gps_time").to_int()));
            for (int u = 0; u < usrp_.size(); ++u)
            {
                usrp_[u]->set_time_next_pps(uhd::time_spec_t(gps_time + 1));
            }

            while (preCheckAlltimers(usrp_, uhd::time_spec_t(gps_time + 1)))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

        } while(finalCheckAlltimers(usrp_, uhd::time_spec_t(gps_time + 1)));
        for (int u = 0; u < usrp_.size(); ++u)
        {
            _KW("Recorder", "USRP" << u <<" time: " << usrp_[u]->get_time_last_pps().get_full_secs());
        }

        stream_cmd.num_samps = size_t(0);
        stream_cmd.stream_now = false;
        if (uhd::time_spec_t(static_cast<double>(conf.rec.targetStartTime)) > gps_time)
        {
            stream_cmd.time_spec = uhd::time_spec_t(static_cast<double>(conf.rec.targetStartTime));
        }
        else
        {
            stream_cmd.time_spec = gps_time + static_cast<double>(conf.rec.startOffset);
        }
    }
    else if (conf.rec.syncSource == config::SyncSource::INTERNAL)
    {
        uhd::time_spec_t systemtime_spec_t;
        do {
            auto sT = std::chrono::high_resolution_clock::now();
            double systemTime = static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(sT.time_since_epoch()).count());
            systemtime_spec_t = uhd::time_spec_t(systemTime);
            for (int u = 0; u < usrp_.size(); ++u)
            {
                usrp_[u]->set_time_next_pps(systemtime_spec_t + uhd::time_spec_t(1.0));
            }
        } while(finalCheckAlltimers(usrp_, systemtime_spec_t + uhd::time_spec_t(1.0)));

        stream_cmd.num_samps = size_t(0);
        stream_cmd.stream_now = false;
        stream_cmd.time_spec = systemtime_spec_t;
        stream_cmd.time_spec += static_cast<double>(conf.rec.startOffset);
    }

    while((stream_cmd.time_spec - usrp_[0]->get_time_last_pps()).get_full_secs() > static_cast<double>(conf.rec.startOffset))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        _KI("Recorder", "Final Countdown: " << (stream_cmd.time_spec - usrp_[0]->get_time_last_pps()).get_full_secs());
    }

    for (unsigned r = 0; r < rxStream_.size(); ++r)
    {
        _KI("Recorder", "Issuing stream cmd: " << r << " " << stream_cmd.time_spec.get_full_secs());
        rxStream_[r]->issue_stream_cmd(stream_cmd);
    }
}

void RecorderMagister::setupRxDev(const config::Usrp& conf,
                                  const config::SyncSource& syncSource,
                                  std::vector<std::vector<std::shared_ptr<RxData<std::complex<int16_t>>>>>& buffer,
                                  const unsigned numBuffers)
{
    usrp_.push_back(uhd::usrp::multi_usrp::make(conf.addr));
    usrp_.back()->set_clock_source(config::syncSource2string(syncSource));
    usrp_.back()->set_time_source(config::syncSource2string(syncSource));
    uhd::usrp::multi_usrp::sptr usrp = usrp_.back();

    _KI("USRP" + std::to_string(usrp_.size() - 1), "[INFO][CLK]: " << config::syncSource2string(syncSource));

    _KI("USRP" + std::to_string(usrp_.size() - 1), boost::format("Using Device: %s") % usrp->get_pp_string());

    usrp->set_rx_rate(conf.FsRx);
    _KI("USRP" + std::to_string(usrp_.size() - 1), "[INFO][Fs]: " << usrp->get_rx_rate(0) / 1e6);

    usrp->set_rx_freq(conf.FcRx);
    _KI("USRP" + std::to_string(usrp_.size() - 1), "[INFO][Fc]: " << usrp->get_rx_freq(0) / 1e6);

    usrp->set_rx_gain(conf.GainRX);
    _KI("USRP" + std::to_string(usrp_.size() - 1), "[INFO][gain]: " << usrp->get_rx_gain(0));

    usrp->set_rx_bandwidth(conf.B);
    _KI("USRP" + std::to_string(usrp_.size() - 1), "[INFO][B]: " << usrp->get_rx_bandwidth(0) / 1e6);

    if (syncSource == config::SyncSource::EXTERNAL)
    {
        check_locked_sensor(usrp->get_mboard_sensor_names(0),
                            "ref_locked",
                            [usrp](const std::string& sensor_name) {return usrp->get_mboard_sensor(sensor_name);},
                            0);
    }

    uhd::stream_args_t stream_args("sc16", "sc16");
    std::vector<size_t> channel_nums;
    for(unsigned i = 0; i < conf.writePath.size() && i < 2; ++i)
    {
        channel_nums.push_back(i);
    }
    stream_args.channels = channel_nums;
    rxStream_.push_back(usrp->get_rx_stream(stream_args));

    buffer.push_back(std::vector<std::shared_ptr<RxData<std::complex<int16_t>>>>(0));
    for (unsigned n = 0; n < numBuffers; ++n)
    {
        buffer.back().push_back(std::make_shared<RxData<std::complex<int16_t>>>(conf.dataPartSize, channel_nums.size()));
    }
    fSyncQueue_.push_back(std::make_shared<SafeQueue<RxData<std::complex<int16_t>>>>(numRxBuffers));
}

RecorderMagister::RecorderMagister(std::atomic<bool> *finishFlag,
                                   FileOut<std::complex<int16_t>>* fileWritter) : finishFlag_(finishFlag)
{
    config::Config& config = config::Config::getInstance();
    config::Recorder conf = config.recorder.get();
    std::cout << conf;
    for (unsigned i = 0; i < conf.usrp.size(); ++i)
    {
        setupRxDev(conf.usrp[i], conf.rec.syncSource, dataBuffer_, numRxBuffers);
    }

    demonicFutures_.push_back(std::async(std::launch::async, &RecorderMagister::fileDataSynchronizer, this, finishFlag_, fileWritter));

    start(conf);

    unsigned g = 0;
    for (unsigned u = 0; u < conf.usrp.size(); ++u)
    {
        futures_.push_back(std::async(std::launch::async, &RecorderMagister::recorderThread, this,
                                                                                             finishFlag_,
                                                                                             u,
                                                                                             conf.usrp[u].dataPartSize,
                                                                                             conf.usrp[u].FsRx,
                                                                                             conf.usrp[u].writePath.size()));
    }
}

void RecorderMagister::reconfigureRx(const config::Recorder& conf)
{
    //TO DO
}

void RecorderMagister::reconfigure(const config::Recorder& conf)
{
    //TO DO
}

void RecorderMagister::configThread(std::atomic<bool> *finishFlag)
{
    /*config::Config& config = config::Config::getInstance();
    config::Recorder defConf = config.recorder.get();

    while (!finishFlag->load())
    {
        config::Recorder conf = config.recorder.get();
        if (conf.usrp != defConf.usrp || conf.signal != defConf.signal)
        {
            _KI("Recorder", "Reconfiguring");
            reconfigure(conf);
        }

        defConf = conf;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }*/
}

static bool checkPtrs(const std::vector<std::shared_ptr<RxData<std::complex<int16_t>>>>& buffs)
{
    for (unsigned b = 0; b < buffs.size(); ++b)
    {
        if (!buffs[b])
        {
            return false;
        }
    }
    return true;
}

void RecorderMagister::fileDataSynchronizer(std::atomic<bool> *finishFlag, FileOut<std::complex<int16_t>>* fileWritter)
{
    config::Config& config = config::Config::getInstance();
    bool newNote = false;
    char time_Ketails[64];
    while (!finishFlag->load())
    {
        std::vector<std::shared_ptr<RxData<std::complex<int16_t>>>> buffs;
        for (unsigned i = 0; i < fSyncQueue_.size(); ++i)
        {
            buffs.push_back(fSyncQueue_[i]->getWhenPossible(finishFlag));
        }

        config::Recorder conf = config.recorder.get();
        if(checkPtrs(buffs))
        {
            std::shared_ptr<RawSamplesToSave<std::complex<int16_t>>> ptr = std::make_shared<RawSamplesToSave<std::complex<int16_t>>>();
            ptr->conf = conf;
            ptr->conf.rec.timestamp64 = buffs[0]->timestamp;
            std::chrono::system_clock::time_point uptime_timepoint{std::chrono::duration_cast<std::chrono::system_clock::time_point::duration>(std::chrono::nanoseconds(buffs[0]->timestamp))};
            std::time_t timeTromTimestmp = std::chrono::system_clock::to_time_t(uptime_timepoint);
            std::strftime(time_Ketails, 64, "%Y_%m_%d_%H_%M_%S", std::localtime(&timeTromTimestmp));
            ptr->conf.rec.timestamp = std::string(time_Ketails);
            for (unsigned b = 0; b < buffs.size(); ++b)
            {
                ptr->rawData.push_back(buffs[b]);
            }
            fileWritter->push_back(ptr, finishFlag);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void RecorderMagister::recorderThread(std::atomic<bool> *finishFlag, unsigned strmId, unsigned dataPartSize, double FsRx, unsigned chNum)
{
    _KE("Recorder", "Running Recorder Thread: " << strmId);
    config::Config& config = config::Config::getInstance();
    config::Recorder defConf = config.recorder.get();

    size_t samps_per_buff = dataPartSize / 5;

    uhd::rx_metadata_t md;
    size_t num_rx_samps;
    std::vector<std::vector<std::complex<int16_t>>> buff;
    for (unsigned k = 0; k < chNum; ++k)
    {
        buff.push_back(std::vector<std::complex<int16_t>>( 10 * dataPartSize));
    }

    std::vector<std::complex<int16_t>*> bufPtrs(2);
    bufPtrs[0] = buff[0].data();
    bufPtrs[1] = buff[1].data();

    uint64_t num_total_samps = 0;

    uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);

    stream_cmd.num_samps = size_t(0);
    stream_cmd.stream_now = true;
    stream_cmd.time_spec = uhd::time_spec_t(3.0);
    uint64_t actT = 0;
    double timeOfSampleNs = 1e9 / static_cast<double>(FsRx);

    auto strMT = std::chrono::high_resolution_clock::now();

    double dT = 0;
    uint64_t timestamp;
    uhd::rx_streamer::sptr rxStream = rxStream_[strmId];

    unsigned rxBufferCounter = 0;
    std::vector<std::shared_ptr<RxData<std::complex<int16_t>>>> buffer = dataBuffer_[strmId];

    while (!finishFlag->load())
    {
        bufPtrs[0] = &buff[0][num_total_samps];
        bufPtrs[1] = &buff[1][num_total_samps];
        num_rx_samps = rxStream->recv(bufPtrs, samps_per_buff, md, 100.0, true);

        if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_TIMEOUT)
        {
            _KE("System", "Timeout while streaming on STRM: " << strmId);
            finishFlag->store(true);
        }
        else if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_OVERFLOW)
        {
            _KW("Recorder", "uhd::rx_metadata_t::ERROR_CODE_OVERFLOW on STRM: " << strmId);
            finishFlag->store(true);
        }
        else if (md.error_code != uhd::rx_metadata_t::ERROR_CODE_NONE)
        {
            _KE("Recorder", "Receiver error: " << md.strerror() << " on STRM: " << strmId);
            finishFlag->store(true);
        }

        num_total_samps += num_rx_samps;
        actT = static_cast<uint64_t>(md.time_spec.get_full_secs() * 1e9) + static_cast<uint64_t>(md.time_spec.get_frac_secs() * 1e9);

        if (num_total_samps >= dataPartSize)
        {
            auto strMT2 = std::chrono::high_resolution_clock::now();
            double t = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(strMT2 - strMT).count()) / 1e9;
            _KI("Recorder", "[INFO][Stream" << strmId << " band]: " << static_cast<double>(dataPartSize * chNum) * 32 / t / 1e6 << " Mb/s");
            _KI("Recorder", "[INFO][Stream" << strmId << " rate]: " << static_cast<double>(dataPartSize * chNum) / t / 1e6 << " MS/s");

            strMT = strMT2;
            timestamp = actT - (timeOfSampleNs * (num_total_samps - 2040));

            buffer[rxBufferCounter]->proctor.lock();
            if(buffer[rxBufferCounter]->hasFreshData)
            {
                _KE("Recorder", "File Sync Queue overflow! : " << strmId);
                finishFlag->store(true);
            }

            for (unsigned k = 0; k < buffer[rxBufferCounter]->data.size(); ++k)
            {
                std::copy(buff[k].data(), buff[k].data() + dataPartSize, buffer[rxBufferCounter]->data[k].data());
                std::copy(buff[k].data() + dataPartSize, buff[k].data() + num_total_samps, buff[k].data());// jezeli bedzie wiecej to umrze
            }

            buffer[rxBufferCounter]->hasFreshData = true;
            buffer[rxBufferCounter]->timestamp = timestamp;
            buffer[rxBufferCounter]->proctor.unlock();
             _KI("Recorder", "[INFO][sync queue CH" << strmId <<" size]: " << fSyncQueue_[strmId]->push_back(buffer[rxBufferCounter]));
            ++rxBufferCounter;
            if (rxBufferCounter >= buffer.size())
            {
                rxBufferCounter = 0;
            }

            num_total_samps -= dataPartSize;
        }
    }

    stream_cmd.stream_mode = uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS;
    _KW("Recorder", "Issuing stop stream cmd");
    rxStream->issue_stream_cmd(stream_cmd);
    int num_post_samps = 0;
    bufPtrs[0] = &buff[0][0];
    bufPtrs[1] = &buff[1][0];
    do {
        num_post_samps = rxStream->recv(bufPtrs, buff[0].size(), md, 3.0);
    } while (num_post_samps && md.error_code == uhd::rx_metadata_t::ERROR_CODE_NONE);

    fSyncQueue_[strmId]->clear();
}

RecorderMagister::~RecorderMagister()
{
    finishFlag_->store(true);
    for (auto& f : futures_) f.wait();
    for (auto& f : demonicFutures_) f.wait();
}

}
