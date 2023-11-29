#include "FileOut.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <complex>

template <typename T>
void FileOut<T>::init(const config::Recorder &config)
{
    recConfig = config;
    unsigned uwp = 0;
    for (int u = 0; u < recConfig.usrp.size(); ++u)
    {
        for (int wp = 0; wp < recConfig.usrp[u].writePath.size(); ++wp)
        {
            std::ostringstream ss;
            ss.str("");
            ss  << recConfig.usrp[u].writePath[wp] << "/" << recConfig.rec.timestamp;
            std::string dir = ss.str();
            dirs.push_back(std::make_shared<std::string>(dir));

            std::error_code ec;
            bool success = true;
            try {
                success = fs::create_directories(dir, ec);
                if (!success)
                {
                    throw SaveToFileException(ec.message());
                }

                std::ofstream configStream;
                configStream.open(dir + std::string("/recorder.ini"));

                if (configStream.good())
                {
                    configStream << recConfig;
                    configStream.close();
                }
                else
                {
                    throw SaveToFileException();
                }

                ss.str("");
                ss << "/rawData" << uwp++ << ".bin";
                fileStream.push_back(std::make_shared<std::ofstream>());
                fileStream.back()->open(dir + ss.str(), std::ios::binary);
                buffer.push_back(std::make_shared<std::vector<char>>(recConfig.usrp[u].dataPartSize * sizeof(T)));
                fileStream.back()->rdbuf()->pubsetbuf(buffer.back()->data(), buffer.back()->size());
            }
            catch(const std::exception& e)
            {
                 _KE("FileWritter", "Unable to create dir: " << dir);
            }
        }
    }
}

template <typename T>
void FileOut<T>::addNote(const config::Recorder *config, int ch)
{
    try {
        std::ofstream configStream;
        configStream.open(*dirs[ch] + "/" + std::to_string(config->rec.timestamp64) + ".note");

        if (configStream.good())
        {
            configStream << *config;
            configStream.close();
        }
        else
        {
            throw SaveToFileException();
        }
    }
    catch(const std::exception& e)
    {
         _KW("FileWritter", "Unable to create note: " << *dirs[ch] + "/"  + std::to_string(config->rec.timestamp64) + ".note");
    }
}

template <typename T>
void FileOut<T>::saveTofile(RxData<T>* data, int subChannel, int ch)
{
    if (fileStream[ch]->good())
    {
        data->proctor.lock();
        fileStream[ch]->write(reinterpret_cast<char *>(data->data[subChannel].data()), data->data[subChannel].size() * sizeof(std::complex<int16_t>));
        _KI("FileWritter", "[INFO][CH : " << ch << " status]: OK");
        data->hasFreshData = false;
        data->proctor.unlock();
    }
    else
    {
        _KE("FileWritter", "[INFO][CH : " << ch << " status]: ERROR");
    }
}

//template class FileOut<float>;
//template class FileOut<int16_t>;
template class FileOut<std::complex<float>>;
template class FileOut<std::complex<int16_t>>;
