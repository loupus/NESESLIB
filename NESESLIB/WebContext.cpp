#include "WebContext.hpp"
#include "NesesString.hpp"
#include <boost/url.hpp>

void NESES::WebContext::SetId()
{
    id = NESES::StringUtil::NewGuid();
}

bool NESES::WebContext::SetHostTarget()
{
	// todo encoded mı değil mi, hata mesajı dönecen mi?
	if (strurl.empty()) return false;
	if (nurl.host.empty() || nurl.target.empty())
	{
		boost::core::string_view s = strurl;
		boost::system::result<boost::urls::url_view> r = boost::urls::parse_uri(s);
		if (r.has_error()) return false;
		if (r.has_value())
		{
			boost::urls::url_view burl = r.value();
			nurl.scheme = burl.scheme();
			nurl.host = burl.host();
			nurl.path = burl.path();
			nurl.port = burl.port();
			nurl.query = burl.query();
			nurl.fragment = burl.fragment();
			nurl.target = nurl.path + "?" + nurl.query;
			nurl.root = nurl.scheme.empty() ? "" : nurl.scheme + "://";
			nurl.root.append(nurl.host);
			nurl.root.append(nurl.port.empty() ? "" : nurl.port);

		}	
	}
	if (nurl.host.empty() || nurl.target.empty()) return false;
	return true;
}

NESES::WebContext::WebContext()
{
    SetId();
}

NESES::WebContext::WebContext(const std::string& surl)
    :strurl(surl)
{
    SetId();
}

NESES::WebContext::WebContext(const std::string& surl, const std::string& fpath)
    :strurl(surl),filepath(fpath)
{
    SetId();
}

void NESES::WebContext::AddHeader(const std::string& key, const std::string& value, bool clearPrevious)
{
    if (key.empty() || value.empty()) return;
    if (clearPrevious)
        ClearHeaders();
    mheaders[key] = value;
}

void NESES::WebContext::ClearHeaders()
{
    mheaders.clear();
}

bool NESES::WebContext::SetUrl(const std::string& url)
{
    strurl.clear();
	nurl.Clear();
    strurl = url;
    return SetHostTarget();
}

void NESES::WebContext::SetFinishedEvent(CallBack<const std::string&>& cbFinished)
{
    cbCompleted_ = cbFinished;
}

void NESES::WebContext::SetProgressEvent(CallBack<int, const std::string&>& cbProgress)
{
    cbProgress_ = cbProgress;
}

void NESES::WebContext::SetContentType(eContentType ct)
{
	switch (ct)
	{
	case eContentType::txt_plain:
	{
		contenttype = "text/plain";
		break;
	}
	case eContentType::txt_html:
	{
		contenttype = "text/html";
		break;
	}
	case eContentType::app_formurlencoded:
	{
		contenttype = "application/x-www-form-urlencoded";
		break;
	}
	case eContentType::app_json:
	{
		contenttype = "application/json";
		break;
	}
	case eContentType::app_xml:
	{
		contenttype = "application/xml";
		break;
	}
	case eContentType::app_octetstream:
	{
		contenttype = "application/octet-stream";
		break;
	}
	case eContentType::multipart_formdata:
	{
		contenttype = "multipart/form-data";
		break;
	}
	case eContentType::img_jpeg:
	{
		contenttype = "image/jpeg";
		break;
	}
	case eContentType::video_mp4:
	{
		contenttype = "video/mp4";
		break;
	}
	default:
	{
		contenttype = "text/plain";
		break;
	}
	}
}

void NESES::WebContext::FireCompletedCb(const std::string& msg)
{
	cbCompleted_.invoke(msg);
}

void NESES::WebContext::FireProgress(int percent, const std::string& msg)
{
	cbProgress_.invoke(percent, msg);
}

const std::map<std::string, std::string>& NESES::WebContext::GetHeaders()
{
    return mheaders;
}

const std::string NESES::WebContext::GetId()
{
    return id;
}
