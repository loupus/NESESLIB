#pragma once
#include <string>
#include <ctime>
#include <memory>
#include "Exporter.h"

// todo consider using boost::locale::date_time instead of boost::date_time
namespace NESES
{

	class NESESAPI NesesDateTime
	{
	private :
		struct NesesDateTimeImp;
		std::unique_ptr<NesesDateTimeImp> pimpl; // convert it to raw pointer?! oldies but goodies

	public:
		NesesDateTime();													// ctor
		NesesDateTime(const std::string& str);								// paramed ctor
		~NesesDateTime();													// dtor
		NesesDateTime(const NesesDateTime& other);							// copy ctor
		NesesDateTime(NesesDateTime&& other) noexcept = default;			// move ctor
		NesesDateTime& operator=(const NesesDateTime& other);				// copy assign
		NesesDateTime& operator=(NesesDateTime&& other) noexcept = default; // move assign
		bool operator >(const NesesDateTime& other);
		bool operator < (const NesesDateTime& other);
		std::string ToString(const std::string& format = "%Y-%m-%d %H:%M:%S", bool localtime = true) const;
		void FromString(std::string pStr, const std::string& format = "%Y-%m-%d %H:%M:%S", bool localtime = true);
		std::string Dump();
		void SetNow();
		struct tm GetTm(bool local = true);
		time_t GetTimeT(bool local = true);
		void AddTime(const char* datepart, int datevalue);
		int Second();
		int Minute();
		int Hour();
		int Day();
		int Month();
		int Year();
		uint64_t TotalSeconds();
		uint64_t TotalMinutes();
		uint64_t TotalHours();
		uint64_t TotalDays();
		uint64_t TotalSecondsOfDay();
		uint64_t TotalMilliSecondsOfDay();
		static std::string GetNowStr(const std::string& format = "%Y-%m-%d %H:%M:%S", bool isLocal = true);
		void fromTm(const std::tm atm);
		
	};
}


