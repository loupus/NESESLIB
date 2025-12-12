#pragma once
#include <map>
#include <string>
#include <filesystem>
#include "Callback.hpp"
#include "Exporter.h"


namespace NESES
{
	enum class eContentType
	{
		txt_plain
		, txt_html
		, app_formurlencoded
		, app_json
		, app_xml
		, app_octetstream
		, multipart_formdata
		, img_jpeg
		, video_mp4
	};

	struct NesesUrl
	{
		std::string scheme;
		std::string host;
		std::string port;
		std::string path;
		std::string query;
		std::string fragment;

		std::string target;
		std::string root;

		void Clear()
		{
			scheme.clear();
			host.clear();
			port.clear();
			path.clear();
			query.clear();
			fragment.clear();
			target.clear();
			root.clear();
		}
	};

	class NESESAPI WebContext
	{
	private:
		std::string id;
		std::string strurl;
		std::string contenttype;
		std::map<std::string, std::string> mheaders;
		CallBack<const std::string&> cbCompleted_;
		CallBack<int, const std::string&> cbProgress_;

		void SetId();
		bool SetHostTarget();

	public:
	
		NesesUrl nurl;
		std::filesystem::path filepath;
		std::string postfields;

		WebContext();
		WebContext(const std::string& surl);
		WebContext(const std::string& surl, const std::string& fpath);
		~WebContext() = default;

		void AddHeader(const std::string& key, const std::string& value, bool clearPrevious = false);
		void ClearHeaders();
		bool SetUrl(const std::string& url);
		void SetFinishedEvent(CallBack<const std::string&>& cbFinished);
		void SetProgressEvent(CallBack<int, const std::string&>& cbProgress);
		void SetContentType(eContentType ct);
		void FireCompletedCb(const std::string& msg);
		void FireProgress(int percent, const std::string& msg);
		const std::map<std::string, std::string>& GetHeaders();
		const std::string GetId();

	
	};
}


