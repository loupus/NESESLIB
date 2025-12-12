#include "NesesIO.hpp"
#include "NesesString.hpp"

NESESAPI bool NESES::IOUtil::IsDirectory(std::filesystem::path aPath)
{
    bool back = false;
    std::error_code ErrCode;
    if (aPath.empty()) return back;
    back = std::filesystem::is_directory(aPath, ErrCode);
    return back;
}

NESESAPI bool NESES::IOUtil::DirectoryExists(std::filesystem::path aPath)
{
    bool back = false;
    std::error_code ErrCode;
    if (aPath.empty()) return back;
    back = std::filesystem::exists(aPath, ErrCode); // noexcept  
    return back;
}

NESESAPI bool NESES::IOUtil::FileExists(std::filesystem::path aPath)
{
    bool back = false;
    std::error_code ErrCode;
    if (aPath.empty()) return back;
    back = std::filesystem::exists(aPath, ErrCode); // noexcept  
    return back;
}

NESESAPI bool NESES::IOUtil::CreateDir(std::filesystem::path aPath)
{
    std::error_code ErrCode;
    if (aPath.empty()) return false;
    /*
    sonundaki slashı kaldırmazsan false dönüyor
    */
    std::string strpath = aPath.string();
    if (strpath.back() == '\\')
        strpath.pop_back();

    if (DirectoryExists(strpath) == false)
        return std::filesystem::create_directories(strpath, ErrCode);
    return true;
}

NESESAPI bool NESES::IOUtil::GetDir(std::filesystem::path aPath)
{
    bool back = false;
    if (aPath.empty()) return back;
    if (!DirectoryExists(aPath))
    {
        back = CreateDir(aPath);
        if (back == false) return back;
    }
    back = true;
    return back;
}

NESESAPI NESES::BackObject NESES::IOUtil::CopyAFile(std::filesystem::path srcPath, std::filesystem::path destPath)
{
    BackObject back;
    if (srcPath.empty() || destPath.empty())
    {
        back.Success = false;
        back.ErrDesc = "Invalid source or destination";
        return back;
    }
    try
    {
        std::filesystem::copy(srcPath, destPath, std::filesystem::copy_options::skip_existing);
        back.Success = true;
    }
    catch (std::filesystem::filesystem_error const& ex)
    {
        back.Success = false;
        back.ErrDesc = "CopyFile:Cannot copy file " + srcPath.string() + " Ex:" + ex.what();
    }
    return back;
}

NESESAPI NESES::BackObject NESES::IOUtil::MoveAFile(std::filesystem::path srcPath, std::filesystem::path destPath)
{
    BackObject back;
    if (srcPath.empty() || destPath.empty())
    {
        back.Success = false;
        back.ErrDesc = "Invalid source or destination";
        return back;
    }

    if (std::filesystem::exists(destPath))
    {
        if (std::filesystem::exists(srcPath))
        {
            try
            {
                std::filesystem::remove(srcPath);
            }
            catch (std::filesystem::filesystem_error const& ex)
            {
                back.ErrDesc = "MoveFile:Cannot delete file: " + srcPath.string() + " Ex:" + ex.what();
                back.Success = false;
                return back;
            }
        }
    }

    try
    {
        std::filesystem::rename(srcPath, destPath);
        back.Success = true;
    }
    catch (std::filesystem::filesystem_error const& ex)
    {
        back.Success = false;
        back.ErrDesc = "MoveAFile:Cannot move file " + srcPath.string() + " Ex:" + ex.what();
    }
    return back;
}

NESESAPI uintmax_t NESES::IOUtil::GetFileSize(std::filesystem::path aPath)
{
    uintmax_t back = 0;
    if (std::filesystem::exists(aPath))
        back = std::filesystem::file_size(aPath);
    return back;
}

NESESAPI bool NESES::IOUtil::DeleteAFile(std::filesystem::path aPath)
{
    bool back = false;
    std::error_code ErrCode;
    if (aPath.empty()) return back;
    back = std::filesystem::remove(aPath, ErrCode);
    return back;
}

NESESAPI std::uintmax_t NESES::IOUtil::RemoveAll(std::filesystem::path aFolder)
{
    std::uintmax_t back = 0;
    std::error_code ErrCode;
    if (aFolder.empty()) return back;
    back = std::filesystem::remove_all(aFolder, ErrCode);
    return back;
}

NESESAPI bool NESES::IOUtil::FileHasSize(std::filesystem::path aPath)
{
    bool back = false;
    if (aPath.empty()) return back;
    if (!FileExists(aPath)) return back;
    long long filesize = GetFileSize(aPath);
    if (filesize > 0)
        back = true;
    return back;
}

NESESAPI int NESES::IOUtil::PathCompare(const std::filesystem::path& FirstPath, const std::filesystem::path& SecondPath)
{
    int back = -999;
    std::string strFirst = FirstPath.string();
    std::string strSecond = SecondPath.string();

    StringUtil::AddTrailingSlash(strFirst);
    StringUtil::AddTrailingSlash(strSecond);

    strFirst = StringUtil::ToLower(strFirst);
    strSecond = StringUtil::ToLower(strSecond);

    std::filesystem::path fpath(strFirst);
    std::filesystem::path spath(strSecond);

    back = fpath.compare(spath);

    return back;
}
