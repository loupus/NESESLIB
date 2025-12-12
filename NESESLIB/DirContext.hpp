#pragma once
#include <string>
#include <filesystem>

namespace NESES
{
  
    struct DirContext
    {
        std::string dirName;
        std::string dirPathStr;
        std::filesystem::path dirPath;
        DirContext() = default;
        DirContext(const std::string& dirname) : dirName(dirname) {}
        DirContext(const std::string& dirname, const std::string& dirpath) 
        : dirName(dirname),dirPathStr(dirpath) {}
        bool IsValid() const
        {
          	if (dirPath.empty()) return false;
			try
			{
				return std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath);
			}
			catch (...)
			{

			}
			return false;
        }

    };
}
