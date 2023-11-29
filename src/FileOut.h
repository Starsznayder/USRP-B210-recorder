#ifndef FILEOUT_H_INCLUDED
#define FILEOUT_H_INCLUDED

#include <fstream>
#include <experimental/filesystem>
#include <sstream>
#include <vector>
#include <future>
#include <config/Config.h>
#include <utu/log.h>
#include "SafeQueue.h"

namespace fs = std::experimental::filesystem;
struct SaveToFileException : public std::runtime_error
{
    SaveToFileException()
        : std::runtime_error("File output error") {}
    SaveToFileException(std::string msg)
        : std::runtime_error(std::string("File output error") + msg) {}
};

template <typename T>
struct RxData
{
    std::mutex proctor;
    bool hasFreshData;
    std::vector<std::vector<T>> data;
    uint64_t timestamp;

    RxData(unsigned N, unsigned chNum) : hasFreshData(false), data(chNum)
    {
        for (unsigned k = 0; k < chNum; ++k)
        {
            data[k] = std::vector<T>(N);
        }
    }
};

template <typename T>
struct RawSamplesToSave
{
    config::Recorder conf;
    std::vector<std::shared_ptr<RxData<T>>> rawData;
};

template <typename T>
class FileOut
{
private:
    std::vector<std::shared_ptr<std::ofstream>> fileStream;
    std::vector<std::shared_ptr<std::vector<char>>> buffer;
    std::vector<std::shared_ptr<std::string>> dirs;
    config::Recorder recConfig;
    std::vector<std::future<void>> futures;
    std::atomic<bool> initialized;

    SafeQueue<RawSamplesToSave<T>> queue;
    std::mutex queueProctor;

    void init(const config::Recorder &config);
    void addNote(const config::Recorder *config, int ch);
    void saveTofile(RxData<T>* data, int subChannel, int ch);

public:

    size_t push_back(std::shared_ptr<RawSamplesToSave<T>> rsts, std::atomic<bool>* flag)
    {
        return queue.push_backWhenPossible(rsts, flag);
    }

    FileOut(std::atomic<bool> *finishFlag)
    {
        initialized.store(false);
        futures.push_back(std::async(std::launch::async, &FileOut::writter, this, finishFlag));
    }

    ~FileOut()
    {
        for (auto& f : futures) f.wait();
        close();
    }

    void writter(std::atomic<bool>* finishFlag)
    {
        std::vector<std::future<void>> fut;
        while(!finishFlag->load())
        {
            std::shared_ptr<RawSamplesToSave<T>> data = queue.getWhenPossible(finishFlag);
            if (data)
            {
                if (!initialized.load())
                {
                    init(data->conf);
                    initialized.store(true);
                }

                if(data->conf.rec.notes != recConfig.rec.notes)
                {
                    int ch = 0;
                    for (int u = 0; u < recConfig.usrp.size(); ++u)
                    {
                        for (int wp = 0; wp < recConfig.usrp[u].writePath.size(); ++wp)
                        {
                            fut.push_back(std::async(std::launch::async, &FileOut::addNote, this, &data->conf, ch++));
                        }
                    }
                    for (auto& f : fut)
                    {
                        f.wait();
                    }
                    recConfig.rec.notes = data->conf.rec.notes;
                    _DI("FileWritter", "[INFO][" << data->conf.rec.timestamp64 << "]: " << data->conf.rec.notes);
                }

                int ch = 0;
                for (int u = 0; u < recConfig.usrp.size(); ++u)
                {
                    for (int wp = 0; wp < recConfig.usrp[u].writePath.size(); ++wp)
                    {
                        fut.push_back(std::async(std::launch::async, &FileOut::saveTofile, this, &*data->rawData[u], wp, ch));
                        ch++;
                    }
                }
                for (auto& f : fut) f.wait();
                fut.clear();
            }
        }
    }

    void close()
    {
        initialized.store(false);
        for (unsigned i = 0; i < fileStream.size(); ++i)
        {
            if(fileStream[i]->is_open() || fileStream[i]->good())
            {
                fileStream[i]->flush();
                fileStream[i]->close();
                _DI("FileWritter", "[INFO][CH : " << i << " status]: DONE!");
            }
        }
    }
};

#endif // FILEOUT_H_INCLUDED
