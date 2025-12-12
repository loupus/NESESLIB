#include "NesesTime.hpp"
#include "boost/date_time.hpp"

struct NESES::NesesDateTime::NesesDateTimeImp
{
	// default ctor
	NesesDateTimeImp()
		:pt(boost::date_time::min_date_time)
		,tzUtc(new boost::local_time::posix_time_zone("UTC+3:00:00"))
		,ldt(pt, tzUtc)
	{

	}

	// copy ctor
	NesesDateTimeImp(const NesesDateTimeImp& other)
		:pt(other.pt),tzUtc(other.tzUtc),ldt(other.ldt)
	{

	}

	// copy assigment operator
	NesesDateTimeImp& operator=(const NesesDateTimeImp& other)
	{
		if (this != &other)
		{
			pt = other.pt;
			tzUtc = other.tzUtc;
			ldt = other.ldt;
		}
		return *this;
	}


	boost::posix_time::ptime pt;
	boost::local_time::time_zone_ptr tzUtc;
	boost::local_time::local_date_time ldt;
	const boost::posix_time::ptime epoch = boost::posix_time::time_from_string("1970-01-01 00:00:00.000");
};


//***************************************************************************************************************
// default ctor
NESES::NesesDateTime::NesesDateTime()
	:pimpl(std::make_unique<NesesDateTimeImp>())
{
	
}

NESES::NesesDateTime::~NesesDateTime() = default;


// copy stor // deep copy 
NESES::NesesDateTime::NesesDateTime(const NesesDateTime& other)
	:pimpl(std::make_unique<NesesDateTimeImp>(*other.pimpl))
{

}

// copy assigment
NESES::NesesDateTime& NESES::NesesDateTime::NesesDateTime::operator=(const NesesDateTime& other)
{
	if (this != &other) 
	{
		*pimpl = *other.pimpl;  
	}
	return *this;
}



NESES::NesesDateTime::NesesDateTime(const std::string& str)
	: NesesDateTime()
{
	if (str.empty()) // now
	{
		SetNow();
	}
	else
	{
		try
		{
			pimpl->pt = boost::posix_time::time_from_string(str);
			pimpl->ldt = boost::local_time::local_date_time(pimpl->pt, pimpl->tzUtc);
		}
		catch (...)
		{

		}

	}
}


bool NESES::NesesDateTime::operator>(const NesesDateTime& other)
{
	return pimpl->ldt > other.pimpl->ldt;
}

bool NESES::NesesDateTime::operator<(const NesesDateTime& other)
{
	return pimpl->ldt < other.pimpl->ldt;
}

std::string NESES::NesesDateTime::ToString(const std::string& format, bool localtime) const
{
	std::string back = "";
	std::stringstream stream;

	if (localtime)
	{
		stream.imbue(std::locale(std::locale::classic(), new boost::local_time::local_time_facet(format.c_str())));

		try
		{
			stream << pimpl->ldt;
			//std::cout << ldt.local_time() << std::endl;
		}
		catch (...)
		{

		}
	}
	else
	{
		stream.imbue(std::locale(std::locale::classic(), new boost::posix_time::time_facet(format.c_str())));

		try
		{
			stream << pimpl->ldt.utc_time();
			//ldt = boost::local_time::local_date_time(pt, tzUtc);
			//std::cout << ldt.utc_time() << std::endl;
		}
		catch (...)
		{

		}
	}

	back = stream.str();
	stream.clear();
	return back;
}

void NESES::NesesDateTime::FromString(std::string pStr, const std::string& format, bool localtime)
{
	std::istringstream is(pStr);
	std::locale loc;
	if (localtime)
	{
		//ndt1.FromString("1978-03-16 05:00:00 UTC+03", "%Y-%m-%d %H:%M:%S %ZP");
		loc = std::locale(std::locale::classic(), new boost::local_time::local_time_input_facet(format));
		is.imbue(loc);
		try
		{
			is >> pimpl->ldt;
		}
		catch (...)
		{

		}
	}
	else
	{
		//ndt1.FromString("1978-03-16 05:00:00", "%Y-%m-%d %H:%M:%S", false);
		loc = std::locale(std::locale::classic(), new boost::posix_time::time_input_facet(format));
		is.imbue(loc);
		try
		{
			is >> pimpl->pt;
		}
		catch (...)
		{

		}
		pimpl->ldt = boost::local_time::local_date_time(pimpl->pt, pimpl->tzUtc);
	}
}

std::string NESES::NesesDateTime::Dump()
{
	std::string back = "";
	std::stringstream stream;
	stream << pimpl->ldt.date().year();
	stream << "-";
	stream << pimpl->ldt.date().month();
	stream << "-";
	stream << pimpl->ldt.date().day();
	stream << " ";
	stream << pimpl->ldt.time_of_day().hours();
	stream << ":";
	stream << pimpl->ldt.time_of_day().minutes();
	stream << ":";
	stream << pimpl->ldt.time_of_day().seconds();
	stream << ":";
	stream << pimpl->ldt.time_of_day().fractional_seconds() << std::endl;
	stream << "is dst: " << pimpl->ldt.is_dst() << std::endl;
	stream << "zone: " << pimpl->ldt.zone() << std::endl;
	stream << "zone abb: " << pimpl->ldt.zone_abbrev() << std::endl;
	stream << "zone name: " << pimpl->ldt.zone_name() << std::endl;
	stream << "zone_as_posix_string: " << pimpl->ldt.zone_as_posix_string() << std::endl;
	back = stream.str();
	return back;
}

void NESES::NesesDateTime::SetNow()
{
	//ldt = boost::local_time::local_microsec_clock::local_time();
	pimpl->ldt = boost::local_time::local_microsec_clock::local_time(pimpl->tzUtc);
}

tm NESES::NesesDateTime::GetTm(bool local)
{
	//boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(pt);
	if (local)
		return boost::local_time::to_tm(pimpl->ldt);
	else
		return boost::posix_time::to_tm(pimpl->ldt.utc_time());
}

time_t NESES::NesesDateTime::GetTimeT(bool local)
{
	if (local)
		return boost::posix_time::to_time_t(pimpl->ldt.local_time());
	else
		return boost::posix_time::to_time_t(pimpl->ldt.utc_time());
}

void NESES::NesesDateTime::AddTime(const char* datepart, int datevalue)
{
	if (datepart == "sec")
		pimpl->ldt += boost::posix_time::seconds(datevalue);
	else if (datepart == "min")
		pimpl->ldt += boost::posix_time::minutes(datevalue);
	else if (datepart == "hour")
		pimpl->ldt += boost::posix_time::hours(datevalue);
	else if (datepart == "day")
		pimpl->ldt += boost::gregorian::days(datevalue);
	else if (datepart == "mon")
		pimpl->ldt += boost::gregorian::months(datevalue);
	else if (datepart == "year")
		pimpl->ldt += boost::gregorian::years(datevalue);
}

int NESES::NesesDateTime::Second()
{
	return static_cast<int>(pimpl->ldt.time_of_day().seconds());
}

int NESES::NesesDateTime::Minute()
{
	return static_cast<int>(pimpl->ldt.time_of_day().minutes());
}

int NESES::NesesDateTime::Hour()
{
	return static_cast<int>(pimpl->ldt.local_time().time_of_day().hours());
}

int NESES::NesesDateTime::Day()
{
	return pimpl->ldt.date().day();
}

int NESES::NesesDateTime::Month()
{
	return pimpl->ldt.date().month();
}

int NESES::NesesDateTime::Year()
{
	return pimpl->ldt.date().year();
}

uint64_t NESES::NesesDateTime::TotalSeconds()
{
	return pimpl->ldt.time_of_day().total_seconds();
}

uint64_t NESES::NesesDateTime::TotalMinutes()
{
	return pimpl->ldt.time_of_day().total_milliseconds();
}

uint64_t NESES::NesesDateTime::TotalHours()
{
	boost::posix_time::time_duration td = pimpl->ldt.local_time() - pimpl->epoch;
	if (td.is_special()) return -1;
	return td.total_seconds();
}

uint64_t NESES::NesesDateTime::TotalDays()
{
	boost::posix_time::time_duration td = pimpl->ldt.local_time() - pimpl->epoch;
	if (td.is_special()) return -1;
	return td.minutes();
}

uint64_t NESES::NesesDateTime::TotalSecondsOfDay()
{
	boost::posix_time::time_duration td = pimpl->ldt.local_time() - pimpl->epoch;
	if (td.is_special()) return -1;
	return td.hours();
}

uint64_t NESES::NesesDateTime::TotalMilliSecondsOfDay()
{
	auto dd = pimpl->ldt.date() - pimpl->epoch.date();
	return dd.days();
}

std::string NESES::NesesDateTime::GetNowStr(const std::string& format, bool isLocal)
{
	NesesDateTime tmp("");
	return tmp.ToString(format, isLocal);
}

void NESES::NesesDateTime::fromTm(const std::tm atm)
{
	pimpl->ldt = boost::local_time::local_date_time(
		boost::gregorian::date(
			atm.tm_year + 1900,
			atm.tm_mon + 1,
			atm.tm_mday),
		boost::posix_time::time_duration(
			atm.tm_hour,
			atm.tm_min,
			atm.tm_sec),
		pimpl->tzUtc, false);
}
//
//// mathematical operators
//boost::posix_time::time_duration operator -(NesesDateTime& other)
//{
//	return ldt - other.ldt;
//
//}
