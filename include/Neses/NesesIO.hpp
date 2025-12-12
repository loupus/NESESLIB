#pragma once
#include <filesystem>
#include "Exporter.h"
#include "BackObject.hpp"

namespace NESES
{
	namespace IOUtil
	{
		NESESAPI bool IsDirectory(std::filesystem::path aPath);
		NESESAPI bool DirectoryExists(std::filesystem::path aPath);
		NESESAPI bool FileExists(std::filesystem::path aPath);
		NESESAPI bool CreateDir(std::filesystem::path aPath);
		NESESAPI bool GetDir(std::filesystem::path aPath);
		NESESAPI BackObject CopyAFile(std::filesystem::path srcPath, std::filesystem::path destPath);
		NESESAPI BackObject MoveAFile(std::filesystem::path srcPath, std::filesystem::path destPath);
		NESESAPI uintmax_t GetFileSize(std::filesystem::path aPath);
		NESESAPI bool DeleteAFile(std::filesystem::path aPath);
		NESESAPI uintmax_t RemoveAll(std::filesystem::path aFolder);
		NESESAPI bool FileHasSize(std::filesystem::path aPath);
		NESESAPI int PathCompare(const std::filesystem::path& FirstPath, const std::filesystem::path& SecondPath);
	}
}


