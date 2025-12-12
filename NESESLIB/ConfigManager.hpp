#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <mutex>
#include "Exporter.h"

namespace NESES
{

	class ConfigManager
	{
	private:
		struct ConfigManagerImp;
		static std::unique_ptr<ConfigManagerImp> pimpl_;

		ConfigManager() = delete;

		static void ensureImpl();

	public:
		NESESAPI static bool ReadConfigFile();
		NESESAPI static bool WriteConfigFile();

		NESESAPI static std::string GetValue(const std::string& path, const std::string& defval="");
		NESESAPI static void UpdateValue(const std::string& path, const std::string& val);

	};
}
