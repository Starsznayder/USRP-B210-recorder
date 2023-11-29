#include "Config.h"


namespace config
{
    void Config::init(const std::string& iniPath, std::atomic<bool>& finish)
    {
        finishFlag_ = &finish;
        recorder = Recorder(iniPath + std::string("/simpleRecorder.ini"));

        futures_.push_back(std::async(std::launch::async, updateThread,
                                                          &finish,
                                                          iniPath,
                                                          &recorder));
    }

    void Config::updateThread(std::atomic<bool>* finish,
                              std::string iniPath,
                              ConfigValue<Recorder>* recorder)
    {
        while (!finish->load())
        {
            try{
                Recorder t = recorder->get();
                recorder->set(Recorder(iniPath + std::string("/simpleRecorder.ini")));
            }
            catch(const ConfigFileException &e)
            {
                _KE("Config", e.what());
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    Config::~Config()
    {
        finishFlag_->store(true);
        for (auto& f : futures_) f.wait();
    }
}


