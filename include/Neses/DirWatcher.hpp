#pragma once
#include <filesystem>
#include <chrono>
#include <vector>
#include <memory>
#include "App.hpp"
#include "CallBack.hpp"
#include "NesesString.hpp"
#include "FileInfo.hpp"
#include "FileList.hpp"
#include "DirContext.hpp"
#include "NesesIO.hpp"
#include "NesesThread.hpp"

namespace NESES
{
    class DirWatcher
    {
    private:
        std::string name;
        DirContext dirContext;
        std::vector<std::string> allowedExts;
        std::chrono::duration<int, std::milli> delay{ 2000 };
        FileList files;
        CallBack<FileInfo> FileCreatedCB;
        CallBack<FileInfo> FileErasedCB;
        CallBack<FileInfo> FileModifiedCB;
        CallBack<std::string> MessageCB;
        std::shared_ptr<NesesThread> thHandle;

        bool IsComplete{ false };
        bool IsStarted{ false };

        bool IsExtOk(std::string anExt)
        {
            bool back = false;
            if (anExt.empty())
                return back;
            std::string fext = StringUtil::ToLower(anExt);
            for (const auto& str : allowedExts)
            {
                if (fext == str)
                {
                    back = true;
                    break;
                }
            }
            return back;
        }
        void GetFiles(const std::string& dirpath, BackObject& back)
        {
            FileInfo fi;
            for (auto& file : std::filesystem::directory_iterator(dirpath))
            {
                if (std::filesystem::is_regular_file(file))
                {
                    if (IsExtOk(file.path().extension().string()))
                    {
                        fi.fpath = file.path();
                        fi.setFileTime(std::filesystem::last_write_time(file));
                        fi.hash = std::filesystem::hash_value(file);
                        back = files.AddItem(fi);
                        if (back.Success == false)
                        {
                            std::cout << back.ErrDesc << std::endl;
                        }
                    }
                }
            }
        }
        void FireCallback(FileInfo& arg)
        {
            switch (arg.fs)
            {
            case FileStatus::created:
            {
                FileCreatedCB.invoke(arg);
                break;
            }
            case FileStatus::erased:
            {
                FileErasedCB.invoke(arg);
                break;
            }
            case FileStatus::modified:
            {
                FileModifiedCB.invoke(arg);
                break;
            }
            }
        }
        void FireMessageCB(const std::string& aStr)
        {
            MessageCB.invoke(aStr);
        }
        void WatchRoutine(std::shared_ptr<NesesThread>& nesesth)
        {
            BackObject back;
            FireMessageCB("Directory watcher " + name + " for " + dirContext.dirPath.string() + " started");

            while (nesesth->GetStopFlag() == false)
            {
                /////////////////////////////////// watchpath control
                if (!dirContext.IsValid())
                {
                    FireMessageCB("Watchpath is not avaliable : " + dirContext.dirPath.string());
                    std::this_thread::sleep_for(delay);
                    continue;
                }
                /// ////////////////////////////////////////

                // önce listeyi dön ve değişiklikleri uygula
                for (int i = 0; i < files.GetSize(); i++)
                {
                    FileInfo& fi = files.Front();

                    // gündönümünde path değişmiş olabilir
                    if (IOUtil::PathCompare(fi.fpath.parent_path(), dirContext.dirPath) != 0) // old path items to be removed -- gundönümü
                    {
                        fi.IsDeleted = true;
                        fi.fs = FileStatus::erased;
                        files.PopFront();
                        FireMessageCB("file will be removed from list due to path change: " + fi.fpath.string());
                        continue;
                    }

                    if (fi.IsDeleted)
                    {
                        files.PopFront();
                        continue;
                    }

                    if (!std::filesystem::exists(fi.fpath))
                    {
                        fi.IsDeleted = true;
                        fi.fs = FileStatus::erased;
                        files.PopFront();
                        FireCallback(fi);
                    }
                }

                if (nesesth->GetStopFlag())
                    break;

                // klasörü tara
                FileInfo fi;
                FileInfo* pfi;

                for (auto& file : std::filesystem::directory_iterator(dirContext.dirPath))
                {
                    fi.clear();
                    pfi = nullptr;
                    if (std::filesystem::is_regular_file(file))
                    {
                        if (IsExtOk(file.path().extension().string()))
                        {
                            fi.fpath = file.path();
                            fi.setFileTime(std::filesystem::last_write_time(file));
                            fi.hash = std::filesystem::hash_value(file);

                            pfi = files.GetIfContains(fi);

                            // yoksa ekle
                            if (pfi == nullptr)
                            {
                                back = files.AddItem(fi);
                                if (back.Success == true)
                                {
                                    fi.fs = FileStatus::created;
                                    FireCallback(fi);
                                }
                                else
                                {
                                    std::cout << back.ErrDesc << std::endl;
                                }
                            }
                            else // varsa ve değişmişse guncelle
                            {
                                if (pfi->GetFileTime() != fi.GetFileTime())
                                {
                                    pfi->fpath = fi.fpath;
                                    pfi->hash = fi.hash;
                                    pfi->setFileTime(fi.GetFileTime());
                                    fi.fs = FileStatus::modified;
                                    FireCallback(fi);
                                }
                            }
                        }
                    }
                }

                if (nesesth->GetStopFlag())
                    break;

                std::this_thread::sleep_for(delay);
            }

            nesesth->SetIsDone(true);

#ifdef _DEBUG
            FireMessageCB("Directory watcher " + name + " for " + dirContext.dirPath.string() + " loop end"); // debug
#endif
        }

    public:
        DirWatcher()
            : DirWatcher{ "NoName" }
        {
        }
        DirWatcher(std::string aName)
            : name(aName)
        {
            auto res = App::Instance().NewWorker("DirWatcher-" + name);
            if (res)
            {
                thHandle = res;
                thHandle->Set(&DirWatcher::WatchRoutine, this, std::ref(thHandle));
            }
            else
            {
                std::cerr << "Cannot get new thread for DirWatcher" << std::endl;
            }
        }
        ~DirWatcher()
        {
            Stop();
        }
        void Start()
        {
            if (IsStarted)
                return;
            if (dirContext.IsValid() == false)
            {
                FireMessageCB("DirWatcher: Invalid directory context for " + name);
                return;
            }
            if (DoGetFiles() >= 0)
            {
                thHandle->Start();
            }
            IsStarted = true;
            FireMessageCB("Directory watcher " + name + " started");
        }
        void Stop()
        {
            if (IsStarted == false)
                return;
            if (thHandle)
            {
                thHandle->Stop();
                while (thHandle->GetIsDone() == false)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                thHandle.reset();
            }
            IsStarted = false;
            FireMessageCB("Directory watcher " + name + " stopped");
        }
        void SetFileCB(CallBack<FileInfo>& cb, FileStatus afs)
        {
            switch (afs)
            {
            case FileStatus::created:
            {
                FileCreatedCB = cb;
                break;
            }
            case FileStatus::erased:
            {
                FileErasedCB = cb;
                break;
            }
            case FileStatus::modified:
            {
                FileModifiedCB = cb;
                break;
            }
            }
        }
        void SetMessageCallback(CallBack<std::string>& aCB)
        {
            MessageCB = aCB;
        }
        bool SetDirUtil(DirContext& dirctx)
        {
            if (dirctx.IsValid() == false)
                return false;
            else
                dirContext = dirctx;
            return true;
        }
        int DoGetFiles()
        {
            BackObject back;
            if (dirContext.IsValid())
            {
                GetFiles(dirContext.dirPath.string(), back);
                if (!back.Success)
                {
                    FireMessageCB("Error getting initial file list: " + back.ErrDesc);
                    return -1;
                }
                int fcount = files.GetSize();
                FireMessageCB(std::to_string(fcount) + " files found on " + dirContext.dirPath.string());
                return fcount;
            }
            return -1;
        }
        const int GetFilesCount()
        {
            return files.GetSize();
        }
        const bool GetIsStarted()
        {
            return IsStarted;
        }
        const std::string GetWatchPath()
        {
            return dirContext.dirPath.string();
        }
        bool AddExtension(std::string anExt)
        {
            bool back = false;
            std::string lanExt = StringUtil::ToLower(anExt);
            if (!IsExtOk(lanExt))
            {
                allowedExts.push_back(lanExt);
                back = true;
            }
            return back;
        }
        void CloneExtensions(const std::vector<std::string>& other)
        {
            allowedExts.clear();
            for (std::string str : other)
            {
                allowedExts.push_back(str);
            }
        }
        bool Reset(std::string apath = "")
        {
            bool back = false;
            FireMessageCB("Directory watcher is reseting ... " + name);
            Stop();
            Start();
            return back;
        }
    };
}