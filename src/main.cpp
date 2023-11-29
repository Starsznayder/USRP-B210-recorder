#include "config/Config.h"
#include "RecorderMagister.h"
#include <csignal>

static std::atomic<bool> finishFlag(false);

void signalHandler( int signum ) {
    _KW("main", "Interrupt signal (" << signum << ") received");
    finishFlag.store(true);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    exit(signum);
}

int main()
{
    finishFlag.store(false);

    try{
        kittyLogs::logger::init(kittyLogs::logger::consoleSink);

        config::Config& conf = config::Config::getInstance();
        conf.init("ini", finishFlag);

        /*kittyLogs::logger::init(kittyLogs::logger::networkSink,
								conf.network.get().logs.addr,
								conf.network.get().logs.port);*/

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
        _KE("main", e.what());
        return -1;
    }
}
