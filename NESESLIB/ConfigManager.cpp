#include <fstream>
#include <mutex>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "Globals.hpp"
#include "ConfigManager.hpp"

namespace NESES
{
    // Define static members
    std::unique_ptr<ConfigManager::ConfigManagerImp> ConfigManager::pimpl_ = nullptr;
  

    // Implementation struct hidden from header
    struct ConfigManager::ConfigManagerImp
    {
        static inline boost::property_tree::ptree tree;
        std::mutex implMutex;                   // protect impl state if needed

        bool Read()
        {
            bool back{ false };
            try
            {
                boost::property_tree::read_xml(Globals::ConfigFile, tree, boost::property_tree::xml_parser::trim_whitespace, Globals::local);
                back = true;
            }
            catch (const boost::property_tree::xml_parser_error  ex)
            {
                std::cout << "config xml parse failed: " << ex.message() << std::endl;
            }
            catch (...)
            {
                std::cout << "config xml parse failed: " << std::endl;
            }
            return back;
        }

        bool Write()
        {
            bool back{ false };
            try
            {
                boost::property_tree::write_xml(Globals::ConfigFile, tree, Globals::local, boost::property_tree::xml_writer_make_settings<std::string>(' ', 4));
                back = true;
            }
            catch (const std::exception& ex)
            {
                std::cout << "config xml write failed: " << ex.what() << '\n';
            }
            catch (...)
            {

            }
            return back;
        }

        std::string Get(const std::string& path, const std::string& defval)
        {
            std::string back;
            try
            {
                back = tree.get<std::string>(path, defval);
            }
            catch (const std::exception&)
            {
            }
            return back;
        }

        void Put(const std::string& path, const std::string& val)
        {
            try
            {
                tree.put<std::string>(path, val);
            }
            catch (...)
            {

            }
        }
    };

    // Create the impl if missing (thread-safe)
    void ConfigManager::ensureImpl()
    {
        if (!pimpl_)
        {
                pimpl_ = std::make_unique<ConfigManagerImp>();
        }
    }

    NESESAPI bool ConfigManager::ReadConfigFile()
    {
        ensureImpl();
        return pimpl_->Read();
    }

    NESESAPI bool ConfigManager::WriteConfigFile()
    {
        ensureImpl();
        return pimpl_->Write();
    }

    NESESAPI std::string ConfigManager::GetValue(const std::string& path, const std::string& defval)
    {
        ensureImpl();
        return pimpl_->Get(path, defval);
    }

    NESESAPI void ConfigManager::UpdateValue(const std::string& path, const std::string& val)
    {
        ensureImpl();
        pimpl_->Put(path, val);
    }
}