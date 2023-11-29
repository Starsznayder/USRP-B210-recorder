#ifndef CONFIG_H
#define CONFIG_H

#include <future>
#include <memory>

#include "RecorderConfig.h"
#include "ConfigException.h"

namespace config{

template<typename T>
class ConfigValue
{

public:
    ConfigValue(const T &value)
    {
        protector_.lock();
        value_ = value;
        protector_.unlock();
    }

    inline T get()
    {

        protector_.lock();
        T value = value_;
        protector_.unlock();
        return value;
    }

    void set(const T &value)
    {
        protector_.lock();
        value_ = value;
        protector_.unlock();
    }

    ConfigValue& operator=(const T& value)
    {
        protector_.lock();
        value_ = value;
        protector_.unlock();
        return *this;
    }

    ConfigValue& operator()(const T &value)
    {
        protector_.lock();
        value_ = value;
        protector_.unlock();
        return *this;
    }

    ConfigValue() {}
    ConfigValue& operator=(const ConfigValue& val)
    {
        protector_.lock();
        value_ = val.value_;
        protector_.unlock();
    }
private:
    T value_;
    std::mutex protector_;
};

class Config
{
public:
    void init(const std::string& iniPath, std::atomic<bool>& finish);

    static Config& getInstance()
    {
            static Config c;
            return c;
    }

    ~Config();

    ConfigValue<Recorder> recorder;

    static void updateThread(std::atomic<bool>* finish,
                             std::string iniPath,
                             ConfigValue<Recorder>* recorder);

private:
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(){}

    std::vector<std::future<void>> futures_;
    std::atomic<bool> *finishFlag_;
};

}

#endif //CONFIG_H
