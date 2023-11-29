#include "RecorderConfig.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using ptree = boost::property_tree::ptree;
namespace property_tree = boost::property_tree;
namespace ini_parser = boost::property_tree::ini_parser;

namespace config{

template <typename T>
void readFields(ptree& pt, std::vector<T>& target, std::string prefix)
{
    int i = 0;
    while (true)
    {
        try {
          std::string name = prefix + std::to_string(i);
          target.push_back(pt.get<T>(name));
          ++i;
        }
        catch (const property_tree::ptree_bad_path&)
        {
            break;
        }
        catch (const property_tree::ptree_bad_data &)
        {
            break;
        }
        catch (const ini_parser::ini_parser_error &)
        {
            break;
        }

    }
}

Recorder::Recorder(const std::string &fileName)
{
    ptree pt;
    try {
        ini_parser::read_ini(fileName, pt);

        int i = 0;
        std::string section="USRP";
        while (true)
        {
            try {
                std::string name = section + std::to_string(i);
                Usrp usrp_t;
                usrp_t.addr = pt.get<std::string>(name + std::string(".addr"));
                usrp_t.GainRX = pt.get<double>(name + std::string(".GainRX"));
                usrp_t.FsRx = pt.get<double>(name + std::string(".FsRx"));
                usrp_t.FcRx = pt.get<double>(name + std::string(".FcRx"));
                usrp_t.B = pt.get<double>(name + std::string(".B"));
                usrp_t.dataPartSize = pt.get<unsigned>(name + std::string(".dataPartSize"));
                readFields(pt, usrp_t.writePath, name + std::string(".writePath"));
                usrp.push_back(usrp_t);
                ++i;
            }
            catch (const property_tree::ptree_bad_path&)
            {
                break;
            }
            catch (const property_tree::ptree_bad_data &)
            {
                break;
            }
            catch (const ini_parser::ini_parser_error &)
            {
                break;
            }
        }

        rec.timestamp = pt.get<std::string>("rec.timestamp");
        rec.timestamp64 = pt.get<uint64_t>("rec.timestamp64");
        rec.notes = pt.get<std::string>("rec.notes");
        rec.octoclockAddr = pt.get<std::string>("rec.octoclockAddr");
        rec.targetStartTime = pt.get<uint64_t>("rec.targetStartTime");
        rec.startOffset = pt.get<uint64_t>("rec.startOffset");
        rec.syncSource = string2syncSource(pt.get<std::string>("rec.syncSource"));
    }
    catch (const ini_parser::ini_parser_error& e)
    {
        throw ConfigFileException(e.what());
    }
    catch (const property_tree::ptree_bad_path& e)
    {
        throw ConfigFileException(e.what());
    }
    catch (const property_tree::ptree_bad_data& e)
    {
        throw ConfigFileException(e.what());
    }

    _DI("Configuration", "Number of configurations loaded: " << usrp.size());

    if (usrp.size() < 1)
    {
        throw ConfigFileException("Broken USRP configuration");
    }

    for (int t = 0; t < usrp.size(); ++t)
    {
        if (usrp[t].writePath.size() == 0 || usrp[t].writePath.size() > 2)
        {
            throw ConfigFileException("Wrong number of channels for USRP: " + std::to_string(t));
        }
    }
}

}

std::ostream& operator<<(std::ostream& stream, const config::Recorder& n)
{
    for (unsigned s = 0; s < n.usrp.size(); ++s)
    {
        stream <<"[USRP" << std::to_string(s) << "]" << std::endl
               << "addr=" << n.usrp[s].addr << std::endl
               <<"GainRX" << std::to_string(s) << "=" << n.usrp[s].GainRX << std::endl
               <<"FsRx" << std::to_string(s) << "=" << n.usrp[s].FsRx << std::endl
               <<"FcRx" << std::to_string(s) << "=" << n.usrp[s].FcRx << std::endl
               <<"B" << std::to_string(s) << "=" << n.usrp[s].B << std::endl
               <<"dataPartSize" << std::to_string(s) << "=" << n.usrp[s].dataPartSize << std::endl;
        for (unsigned k = 0; k < n.usrp[s].writePath.size(); ++k)
        {
            stream <<"writePath" << std::to_string(s) << "=" << n.usrp[s].writePath[k] << std::endl;
        }
    }

    stream << "[rec]" << std::endl
           << "timestamp=" << n.rec.timestamp << std::endl
           << "octoclockAddr=" << n.rec.octoclockAddr << std::endl
           << "syncSource=" << config::syncSource2string(n.rec.syncSource) << std::endl
           << "targetStartTime=" << n.rec.targetStartTime << std::endl
           << "startOffset=" << n.rec.startOffset << std::endl
           << "timestamp64=" << n.rec.timestamp64 << std::endl
           << "notes=" << n.rec.notes << std::endl;

    return stream;
}
