#pragma once
#include <string>
#include <locale>
#include "NesesString.hpp"
namespace NESES
{
	class Globals
	{
	public:

		static inline std::string ConfigFile{"config.xml"};
		static inline std::string LogPath{"."};
		static inline std::locale local = StringUtil::GetLocale(StringUtil::cEncoding::e_tr_utf8);
		
		static const int MaxWorkerCount{ 4 };
		static const int MaxLogQueueSize{ 255 };

	};
}