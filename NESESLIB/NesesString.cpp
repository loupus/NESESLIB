#include "NesesString.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "boost/lexical_cast.hpp"		//boost::lexical_cast
#include "boost/algorithm/string.hpp"	//boost::algorithm::split
#include "boost/locale.hpp"
//#include "ztd/text.hpp"				// patlak
#include "utf8.h"

NESESAPI std::string NESES::StringUtil::NewGuid()
{
	boost::uuids::uuid auid = boost::uuids::random_generator()();
	return boost::uuids::to_string(auid);
}

NESESAPI std::string NESES::StringUtil::GetLocaleName(cEncoding enc)
{
	std::string back("");
	switch (enc)
	{
	case cEncoding::e_win_1251:
	case cEncoding::e_iso_8859_1:
	{
		back = "en_US.ISO-8859-1";
		break;
	}
	case cEncoding::e_win_1252:
	case cEncoding::e_iso_8859_2:
	{
		back = "en_US.ISO-8859-2";
		break;
	}
	case cEncoding::e_win_1254:
	{
		back = "tr_TR.ISO-8859-9";
		break;
	}
	case cEncoding::e_en_utf8:
	{
		back = "en_US.UTF-8";
		break;
	}
	case cEncoding::e_tr_utf8:
	default:
	{
		back = "tr_TR.UTF-8";
		break;
	}
	}

	return back;
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

	switch (srcEncoding)
	{
	case cEncoding::e_win_1251:
	{
		try
		{
			back = boost::locale::conv::to_utf<char>(str, "Windows-1251");
		}
		catch (const std::exception& ex)
		{
			backobj.Success = false;
			backobj.ErrDesc = ex.what();
		}
		


		break;
	}
	case cEncoding::e_win_1252:
	{

		try
		{
			back = boost::locale::conv::to_utf<char>(str, "Windows-1252");
		}
		catch (const std::exception& ex)
		{
			backobj.Success = false;
			backobj.ErrDesc = ex.what();
		}

		//auto res = ztd::text::transcode_to(str, ztd::text::windows_1252, ztd::text::compat_utf8);
		//if (res.error_code == ztd::text::encoding_error::ok)
		//	back = res.output;
		//else
		//{
		//	backobj.Success = false;
		//	backobj.ErrDesc = ztd::text::to_name(res.error_code);
		//}
		break;
	}
	case cEncoding::e_win_1254:
	{

		try
		{
			back = boost::locale::conv::to_utf<char>(str, "Windows-1254");
		}
		catch (const std::exception& ex)
		{
			backobj.Success = false;
			backobj.ErrDesc = ex.what();
		}
		//auto res = ztd::text::transcode_to(str, ztd::text::windows_1254, ztd::text::compat_utf8);
		//if (res.error_code == ztd::text::encoding_error::ok)
		//	back = res.output;
		//else
		//{
		//	backobj.Success = false;
		//	backobj.ErrDesc = ztd::text::to_name(res.error_code);
		//}
		break;
	}
	case cEncoding::e_iso_8859_1:
	{
		try
		{
			back = boost::locale::conv::to_utf<char>(str, "ISO-8859-1");
		}
		catch (const std::exception& ex)
		{
			backobj.Success = false;
			backobj.ErrDesc = ex.what();
		}

		//auto res = ztd::text::transcode_to(str, ztd::text::iso_8859_1, ztd::text::compat_utf8);
		//if (res.error_code == ztd::text::encoding_error::ok)
		//	back = res.output;
		//else
		//{
		//	backobj.Success = false;
		//	backobj.ErrDesc = ztd::text::to_name(res.error_code);
		//}
		break;
	}
	case cEncoding::e_iso_8859_2:
	{

		try
		{
			back = boost::locale::conv::to_utf<char>(str, "ISO-8859-2");
		}
		catch (const std::exception& ex)
		{
			backobj.Success = false;
			backobj.ErrDesc = ex.what();
		}
		//auto res = ztd::text::transcode_to(str, ztd::text::iso_8859_2, ztd::text::compat_utf8);
		//if (res.error_code == ztd::text::encoding_error::ok)
		//	back = res.output;
		//else
		//{
		//	backobj.Success = false;
		//	backobj.ErrDesc = ztd::text::to_name(res.error_code);
		//}
		break;
	}

	case cEncoding::e_iso_8859_9:
	{

		try
		{
			back = boost::locale::conv::to_utf<char>(str, "ISO-8859-9");
		}
		catch (const std::exception& ex)
		{
			backobj.Success = false;
			backobj.ErrDesc = ex.what();
		}
		//auto res = ztd::text::transcode_to(str, ztd::text::iso_8859_2, ztd::text::compat_utf8);
		//if (res.error_code == ztd::text::encoding_error::ok)
		//	back = res.output;
		//else
		//{
		//	backobj.Success = false;
		//	backobj.ErrDesc = ztd::text::to_name(res.error_code);
		//}
		break;
	}
	default:
	{
		backobj.Success = false;
		backobj.ErrDesc = "invalid source encoding";
	}
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

	switch (destEncoding)
	{
	case cEncoding::e_win_1251:
	{
		try
		{
			back = boost::locale::conv::from_utf<char>(str, "Windows-1251");
		}
		catch (const std::exception& ex)
		{
			backobj.Success = false;
			backobj.ErrDesc = ex.what();
		}
		break;
	}
	case cEncoding::e_win_1252:
	{
		try
		{
			back = boost::locale::conv::from_utf<char>(str, "Windows-1252");
		}
		catch (const std::exception& ex)
		{
			backobj.Success = false;
			backobj.ErrDesc = ex.what();
		}
		break;
	}
	case cEncoding::e_win_1254:
	{
		try
		{
			back = boost::locale::conv::from_utf<char>(str, "Windows-1254");
		}
		catch (const std::exception& ex)
		{
			backobj.Success = false;
			backobj.ErrDesc = ex.what();
		}
		break;
	}
	case cEncoding::e_iso_8859_1:
	{
		try
		{
			back = boost::locale::conv::from_utf<char>(str, "ISO-8859-1");
		}
		catch (const std::exception& ex)
		{
			backobj.Success = false;
			backobj.ErrDesc = ex.what();
		}
		break;
	}
	case cEncoding::e_iso_8859_2:
	{
		try
		{
			back = boost::locale::conv::from_utf<char>(str, "ISO-8859-2");
		}
		catch (const std::exception& ex)
		{
			backobj.Success = false;
			backobj.ErrDesc = ex.what();
		}
		break;
	}
	case cEncoding::e_iso_8859_9:
	{
		try
		{
			back = boost::locale::conv::from_utf<char>(str, "ISO-8859-9");
		}
		catch (const std::exception& ex)
		{
			backobj.Success = false;
			backobj.ErrDesc = ex.what();
		}
		break;
	}
	default:
	{
		backobj.Success = false;
		backobj.ErrDesc = "invalid source encoding";
	}
	}



	//switch (destEncoding)
	//{
	//case cEncoding::e_win_1251:
	//{
	//	auto res = ztd::text::transcode_to(str, ztd::text::compat_utf8, ztd::text::windows_1251, ztd::text::replacement_handler);
	//	if (res.error_code == ztd::text::encoding_error::ok)
	//		back = res.output;
	//	else
	//	{
	//		backobj.Success = false;
	//		backobj.ErrDesc = ztd::text::to_name(res.error_code);
	//	}
	//	break;
	//}
	//case cEncoding::e_win_1252:
	//{
	//	auto res = ztd::text::transcode_to(str, ztd::text::compat_utf8, ztd::text::windows_1252, ztd::text::replacement_handler);
	//	if (res.error_code == ztd::text::encoding_error::ok)
	//		back = res.output;
	//	else
	//	{
	//		backobj.Success = false;
	//		backobj.ErrDesc = ztd::text::to_name(res.error_code);
	//	}
	//	break;
	//}
	//case cEncoding::e_win_1254:
	//{
	//	auto res = ztd::text::transcode_to(str, ztd::text::compat_utf8, ztd::text::windows_1254, ztd::text::replacement_handler);
	//	if (res.error_code == ztd::text::encoding_error::ok)
	//		back = res.output;
	//	else
	//	{
	//		backobj.Success = false;
	//		backobj.ErrDesc = ztd::text::to_name(res.error_code);
	//	}
	//	break;
	//}
	//case cEncoding::e_iso_8859_1:
	//{
	//	auto res = ztd::text::transcode_to(str, ztd::text::compat_utf8, ztd::text::iso_8859_1, ztd::text::replacement_handler);
	//	if (res.error_code == ztd::text::encoding_error::ok)
	//		back = res.output;
	//	else
	//	{
	//		backobj.Success = false;
	//		backobj.ErrDesc = ztd::text::to_name(res.error_code);
	//	}
	//	break;
	//}
	//case cEncoding::e_iso_8859_2:
	//{
	//	auto res = ztd::text::transcode_to(str, ztd::text::compat_utf8, ztd::text::iso_8859_2, ztd::text::replacement_handler);
	//	if (res.error_code == ztd::text::encoding_error::ok)
	//		back = res.output;
	//	else
	//	{
	//		backobj.Success = false;
	//		backobj.ErrDesc = ztd::text::to_name(res.error_code);
	//	}
	//	break;
	//}
	//default:
	//{
	//	backobj.Success = false;
	//	backobj.ErrDesc = "invalid source encoding";
	//}
	//}

	return back;
}

NESESAPI std::string NESES::StringUtil::ToUpper(const std::string& str, std::locale local)
{
	std::string back;
	if (!str.empty())
	{
		back = boost::algorithm::to_upper_copy(str, local);
	}
	return back;
}

NESESAPI std::string NESES::StringUtil::ToLower(const std::string& str, std::locale local)
{
	std::string back;
	if (!str.empty())
	{
		back = boost::algorithm::to_lower_copy(str, local);
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

NESESAPI size_t NESES::StringUtil::GetCharLen(const std::string& str)
{
	// bool is_valid = utf8::is_valid(utf8str.begin(), utf8str.end());
	// utf8::distance(x.begin(), x.end());  // returns 1
	std::size_t back = 0;
	if (str.empty()) return back;
	auto it = str.begin();
	auto end = str.end();

	while (it != end)
	{
		try {
			utf8::next(it, end);
			++back;
		}
		catch (const utf8::invalid_utf8&) 
		{
			// handle error
		//	++it; // skip invalid byte
			back = -1;
			break;
		}
	}
	return back;
}

NESESAPI bool NESES::StringUtil::IsValidUtf8(const std::string& str)
{
	return utf8::is_valid(str.begin(), str.end());
}
