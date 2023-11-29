#ifndef RECORDERCONFIG_H
#define RECORDERCONFIG_H

#include <string>
#include "ConfigException.h"
#include <kittyLogs/log.h>

namespace config{

template<typename T>
bool compareFields(std::vector<T>& a, const std::vector<T>& b)
{
    if (a.size() != b.size())
    {
        return false;
    }

    for (unsigned i = 0; i < a.size(); ++i)
    {
        if (a[i] != b[i])
        {
            return false;
        }
    }
    return true;
}

enum class SyncSource : unsigned
{
    INTERNAL = 0,
    EXTERNAL = 1,
    GPS = 2
};

static std::string syncSource2string(const SyncSource& src)
{
    if (src == SyncSource::GPS)
    {
        return std::string("gpsdo");
    }
    else if (src == SyncSource::EXTERNAL)
    {
        return std::string("external");
    }
    else
    {
        return std::string("internal");
    }
}

static SyncSource string2syncSource(const std::string& src)
{
    if (src == std::string("gpsdo"))
    {
        return SyncSource::GPS;
    }
    else if (src == std::string("external"))
    {
        return SyncSource::EXTERNAL;
    }
    else
    {
        return SyncSource::INTERNAL;
    }
}

struct Usrp
{
    std::string addr;
    double GainRX;
    double FsRx;
    double FcRx;
    double B;
    unsigned dataPartSize;
    std::vector<std::string> writePath;

    bool operator==(const Usrp &uc)
    {
        return uc.addr == addr &&
               FcRx == uc.FcRx &&
               GainRX == uc.GainRX &&
               FsRx == uc.FsRx &&
               B == uc.B &&
               compareFields(writePath, uc.writePath) &&
               dataPartSize == uc.dataPartSize;
    }

    bool operator!=(const Usrp &uc)
    {
        return !(*this == uc);
    }

    Usrp& operator=(const Usrp& o)
    {
        addr = o.addr;
        FcRx = o.FcRx;
        GainRX = o.GainRX;
        FsRx = o.FsRx;
        B = o.B;
        dataPartSize = o.dataPartSize;
        writePath = o.writePath;
        return *this;
    }
};

struct Rec
{
    std::string timestamp;
    std::string octoclockAddr;
    uint64_t targetStartTime;
    uint64_t startOffset;
    uint64_t timestamp64;
    SyncSource syncSource;
    std::string notes;

    bool operator==(const Rec &rc)
    {
        return rc.timestamp == timestamp &&
               rc.octoclockAddr == octoclockAddr &&
               rc.targetStartTime == targetStartTime &&
               rc.startOffset == startOffset &&
               rc.timestamp64 == timestamp64 &&
               rc.syncSource == syncSource &&
               rc.notes == notes;
    }

    bool operator!=(const Rec &rc)
    {
        return !(*this == rc);
    }

    Rec& operator=(const Rec& o)
    {
        timestamp = o.timestamp;
        octoclockAddr = o.octoclockAddr;
        targetStartTime = o.targetStartTime;
        startOffset = o.startOffset;
        timestamp64 = o.timestamp64;
        syncSource = o.syncSource;
        notes = o.notes;
        return *this;
    }
};

class Recorder
{
public:
    Recorder(const std::string& fileName);
    Recorder() {}

    std::vector<Usrp> usrp;
    Rec rec;

    bool operator==(const Recorder &rc)
    {
        return compareFields(usrp, rc.usrp) &&
               rec == rc.rec;
    }

    bool operator!=(const Recorder &dc)
    {
        return !(*this == dc);
    }

    Recorder& operator=(const Recorder& o)
    {
        usrp = o.usrp;
        rec = o.rec;
        return *this;
    }
};

}

std::ostream& operator<<(std::ostream& stream, const config::Recorder& n);

#endif //RECORDERCONFIG_H
