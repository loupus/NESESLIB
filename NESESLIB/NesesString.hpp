#pragma once
#include <string>
#include <locale>
#include <vector>
#include "Exporter.h"
#include "BackObject.hpp"

namespace NESES
{
	namespace StringUtil
	{
		enum class cEncoding
		{
			e_win_1251 = 0,
			e_win_1252,
			e_win_1254,
			e_iso_8859_1,
			e_iso_8859_2,
			e_iso_8859_9,
			e_en_utf8,
			e_tr_utf8
		};

		NESESAPI std::string NewGuid();
		NESESAPI std::string GetLocaleName(cEncoding enc);
		NESESAPI std::locale GetLocale(cEncoding enc);
		NESESAPI std::string ToUtf8(const std::string& str, cEncoding srcEncoding, BackObject& backobj);
		NESESAPI std::string FromUtf8(const std::string& str, cEncoding destEncoding, BackObject& backobj);
		NESESAPI std::string ToUpper(const std::string& str, std::locale local = std::locale());
		NESESAPI std::string ToLower(const std::string& str, std::locale local = std::locale());
		NESESAPI void Split(const std::string& str, const std::string& delim, std::vector<std::string>& dest);
		NESESAPI int ParseInteger(const std::string& str, int replaceVal = 0);
		NESESAPI float ParseFloat(const std::string& str, float replaceVal = 0.0f);
		NESESAPI std::string Trim(const std::string& str);
		NESESAPI void AddTrailingSlash(std::string& str);
		NESESAPI bool IsValidUtf8(const std::string& str);



		// get the grapheme cluster count in given local
		NESESAPI std::size_t GraphClusterLen(const std::string& str, std::locale local);

		// get utf8 codepoint count
		NESESAPI std::size_t CodePointLen(const std::string& str);

	}
}




