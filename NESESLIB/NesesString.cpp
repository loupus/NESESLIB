#include "NesesString.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "boost/lexical_cast.hpp"		//boost::lexical_cast
#include "boost/algorithm/string.hpp"	//boost::algorithm::split
#include "boost/locale.hpp"

NESESAPI std::string NESES::StringUtil::NewGuid()
{
	boost::uuids::uuid auid = boost::uuids::random_generator()();
	return boost::uuids::to_string(auid);
}

NESESAPI std::string NESES::StringUtil::GetLocaleName(cEncoding enc)
{
	switch (enc)
	{
	case cEncoding::e_win_1251:   return "ru_RU.CP1251";          // Windows-1251 (Cyrillic)
	case cEncoding::e_win_1252:   return "en_US.CP1252";         // Windows-1252 (Western Latin)
	case cEncoding::e_win_1254:   return "tr_TR.CP1254";         // Windows-1254 (Turkish)
	case cEncoding::e_iso_8859_1: return "en_US.ISO-8859-1";    // Latin-1
	case cEncoding::e_iso_8859_2: return "pl_PL.ISO-8859-2";    // Central European (example country)
	case cEncoding::e_iso_8859_9: return "tr_TR.ISO-8859-9";    // Latin-5 (Turkish)
	case cEncoding::e_en_utf8:    return "en_US.UTF-8";
	case cEncoding::e_tr_utf8:    return "tr_TR.UTF-8";
	default:                      return "en_US.UTF-8";
	}
}

NESESAPI std::locale NESES::StringUtil::GetLocale(cEncoding enc)
{
	return  boost::locale::generator().generate(GetLocaleName(enc));
}

NESESAPI std::string NESES::StringUtil::ToUtf8(const std::string& str, cEncoding srcEncoding, BackObject& backobj)
{
	std::string back;	

	if (str.empty())
	{
		backobj.Success = false;
		backobj.ErrDesc = "empty input!";
		return back;
	}

	std::string localname = StringUtil::GetLocaleName(srcEncoding);
	try
	{
		back = boost::locale::conv::to_utf<char>(str, localname);
	}
	catch (const std::exception& ex)
	{
		backobj.Success = false;
		backobj.ErrDesc = ex.what();
	}

	return back;
}

NESESAPI std::string NESES::StringUtil::FromUtf8(const std::string& str, cEncoding destEncoding, BackObject& backobj)
{
	std::string back;

	if (str.empty())
	{
		backobj.Success = false;
		backobj.ErrDesc = "empty input!";
		return back;
	}

	std::string localname = StringUtil::GetLocaleName(destEncoding);
	try
	{
		back = boost::locale::conv::from_utf<char>(str, localname);
	}
	catch (const std::exception& ex)
	{
		backobj.Success = false;
		backobj.ErrDesc = ex.what();
	}

	return back;
}

NESESAPI std::string NESES::StringUtil::ToUpper(const std::string& str, std::locale local)
{
	std::string back;
	if (!str.empty())
	{
		try
		{
			back = boost::locale::to_upper(str, local);
		}
		catch (...)
		{

		}
		
	}
	return back;
}

NESESAPI std::string NESES::StringUtil::ToLower(const std::string& str, std::locale local)
{
	std::string back;
	if (!str.empty())
	{
		try
		{
			back = boost::locale::to_lower(str, local);
		}
		catch (...)
		{

		}

	}
	return back;
}

NESESAPI void NESES::StringUtil::Split(const std::string& str, const std::string& delim, std::vector<std::string>& dest)
{
	boost::algorithm::split(dest, str, boost::is_any_of(delim));
}

NESESAPI int NESES::StringUtil::ParseInteger(const std::string& str, int replaceVal)
{
	int back = replaceVal;
	if (str.empty()) return back;
	try
	{
		back = boost::lexical_cast<int>(str);
	}
	catch (...)
	{

	}
	return back;
}

NESESAPI float NESES::StringUtil::ParseFloat(const std::string& str, float replaceVal)
{
	float back = replaceVal;
	if (str.empty()) return back;
	try
	{
		back = boost::lexical_cast<float>(str);
	}
	catch (...)
	{

	}
	return back;
}

NESESAPI std::string NESES::StringUtil::Trim(const std::string& str)
{
	std::string back = str;
	boost::algorithm::trim(back);
	return back;
}

NESESAPI void NESES::StringUtil::AddTrailingSlash(std::string& str)
{
	if (!(str.back() == '\\' || str.back() == '/'))
		str += '\\';
}

NESESAPI bool NESES::StringUtil::IsValidUtf8(const std::string& str)
{
	const unsigned char* bytes = reinterpret_cast<const unsigned char*>(str.data());
	std::size_t len = str.size();
	std::size_t i = 0;

	while (i < len)
	{
		unsigned char c = bytes[i];
		if (c <= 0x7F) // ASCII
		{
			++i;
			continue;
		}

		std::size_t remaining = 0;
		if ((c & 0xE0) == 0xC0) remaining = 1;       // 110xxxxx
		else if ((c & 0xF0) == 0xE0) remaining = 2;  // 1110xxxx
		else if ((c & 0xF8) == 0xF0) remaining = 3;  // 11110xxx
		else return false; // invalid leading byte

		if (i + remaining >= len) return false; // truncated sequence

		// continuation bytes must be 10xxxxxx
		for (std::size_t j = 1; j <= remaining; ++j)
			if ((bytes[i + j] & 0xC0) != 0x80) return false;

		// Note: this basic validator does not reject all overlong encodings or codepoints > U+10FFFF.
		// If you need stricter validation, add overlong / surrogate / range checks here.

		i += remaining + 1;
	}

	return true;
}

NESESAPI std::size_t NESES::StringUtil::GraphClusterLen(const std::string& str, std::locale local)
{
	if (str.empty()) return 0;

	// validate UTF-8 first (optional but helps return -1 on invalid input)
	if (!IsValidUtf8(str)) return static_cast<std::size_t>(-1);

	try
	{
		using boost::locale::boundary::ssegment_index;
		ssegment_index segments(boost::locale::boundary::character, str.begin(), str.end(), local);
		return static_cast<std::size_t>(std::distance(segments.begin(), segments.end()));
	}
	catch (...)
	{
		// follow existing convention: return -1 cast to size_t on error
		return static_cast<std::size_t>(-1);
	}
}

NESESAPI std::size_t NESES::StringUtil::CodePointLen(const std::string& str)
{
	if (str.empty()) return 0;

	try
	{
		// Convert UTF-8 to UTF-32 (each char32_t is one Unicode code point)
		auto u32 = boost::locale::conv::utf_to_utf<char32_t>(str);
		return u32.size();
	}
	catch (const std::exception&)
	{
		// follow the project's convention: return -1 cast to size_t on error
		return static_cast<std::size_t>(-1);
	}
}

