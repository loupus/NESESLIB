#pragma once
#include <filesystem>
#include "NesesTime.hpp"


/*
Rule Of Three : destructor - copy constructor - copy assigment

Rule Of Five : destructor - copy constructor - copy assigment  -- move constructor -- move assigment
*/


namespace NESES
{
	enum class FileStatus
	{
		none,
		created,
		modified,
		erased
	};

	class FileInfo
	{

	private:
		std::filesystem::file_time_type ftime;

	public:
		std::filesystem::path fpath;
		size_t hash{ 0 };
		NesesDateTime ntime;
		FileStatus fs{ FileStatus::none };
		bool IsDeleted{ false };

		//default ctor
		FileInfo()
		{

		}

		// parameter ctor
		FileInfo(std::filesystem::path apath)
		{
			hash = std::filesystem::hash_value(apath);
		}

		/*
		//copy ctor
		FileInfo(const FileInfo& other)
		{
			fpath = other.fpath;
			hash = other.hash;
			ftime = other.ftime;
			fs = other.fs;
			IsDeleted = other.IsDeleted;
		}

		//copy assignment operator
		FileInfo& operator=(const FileInfo& other)
		{
			if (this != &other)
			{
				fpath = other.fpath;
				hash = other.hash;
				ftime = other.ftime;
				fs = other.fs;
				IsDeleted = other.IsDeleted;
			}
			return *this;
		}

		// move constructor
		FileInfo(FileInfo&& other) noexcept = default;

		// move assigment
		FileInfo& operator=(FileInfo&& other) noexcept = default;



		////move ctor
		//FileInfo(FileInfo&& other) noexcept
		//{
		//	fpath = std::move(other.fpath);
		//	ftime = std::move(other.ftime);
		//	hash = other.hash;
		//}
		*/


		// equality operator
		bool operator==(const FileInfo& other) const
		{
			if (hash == 0 || other.hash == 0) return false;
			return (hash == other.hash);
		}
		// non-equality operator
		bool operator!=(const FileInfo& other) const
		{
			return !operator==(other);
		}

		void clear()
		{
			fpath.clear();
			hash = 0;
			ftime = std::filesystem::file_time_type::clock::now(); // todo min yap bunu
			fs = FileStatus::none;
			IsDeleted = false;
		}

		void setFileTime(std::filesystem::file_time_type aFileTime)
		{
			ftime = aFileTime;
			filetimeToTimet();
		}

		std::filesystem::file_time_type GetFileTime()
		{
			return ftime;
		}

		void filetimeToTimet()
		{
			std::time_t systimet(0);
			struct tm* systm;
			systimet = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now() - (std::filesystem::file_time_type::clock::now() - ftime));
			systm = std::localtime(&systimet);
			ntime.fromTm(*systm);

		}
	};
}