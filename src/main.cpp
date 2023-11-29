#include "config/Config.h"
#include "RecorderMagister.h"
#include <csignal>

static std::atomic<bool> finishFlag(false);

void signalHandler( int signum ) {
    _DC("main", "Interrupt signal (" << signum << ") received");
    finishFlag.store(true);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    exit(signum);
}

int main()
{
    finishFlag.store(false);

    try{
        utu::logger::init(utu::logger::consoleSink);

        config::Config& conf = config::Config::getInstance();
        conf.init("ini", finishFlag);


        /*utu::logger::init(utu::logger::networkSink,
                          conf.network.get().logs.addr,
                          conf.network.get().logs.port);*/
        utu::logger::setProcess("main");

        signal(SIGKILL, signalHandler);
        signal(SIGTERM, signalHandler);
        signal(SIGINT, signalHandler);

        FileOut<std::complex<int16_t>> fileWritter(&finishFlag);

        std::shared_ptr<Magister::RecorderMagister> rm = std::make_shared<Magister::RecorderMagister>(&finishFlag, &fileWritter);

        while(!finishFlag.load())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    catch(const config::ConfigFileException &e)
    {
        _DC("main", e.what());
        return -1;
    }
}
