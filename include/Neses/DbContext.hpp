#pragma once
#include <string>

namespace NESES
{
	enum class DbType
	{
		none = 0,
		mssql,
		postgre,
		oracle,
		mysql
	};

	struct DbContext
	{
		std::string ConName;
		DbType DbType;
		std::string DbServer;
		int DbPort{ 0 };
		std::string DbName;
		std::string DbUser;
		std::string DbPass;
		bool DbIsTrusted{ false };

		void Reset()
		{
			ConName.clear();
			DbType = DbType::none;
			DbServer.clear();
			DbPort = 0;
			DbName.clear();
			DbUser.clear();
			DbPass.clear();
			DbIsTrusted = false;
		}
	};
}