#pragma once
#include <string>
#include <iostream>

namespace NESES
{
//todo generic data, result pattern
struct BackObject
{
	bool Success = true;
	int ErrCode = 0;
	std::string ErrDesc;
	std::string Warning;

	friend std::ostream& operator << (std::ostream& out, const BackObject& c)
	{
		out << "Success: " << c.Success << '\n'
			<< "ErrCode: " << c.ErrCode << '\n'
			<< "ErrDesc: " << c.ErrDesc << '\n'
			<< "Warning: " << c.Warning << '\n';
		return out;
	}

	void Reset(bool success = true)
	{
		Success = success;
		ErrCode = 0;
		ErrDesc.clear();
		Warning.clear();
	}

	operator bool() const
	{
		return Success;
	}
};
}