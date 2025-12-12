#pragma once
#if defined(NESESLIB_EXPORTS)
#   define NESESAPI   __declspec(dllexport)
#else 
#   define NESESAPI   __declspec(dllimport)
#endif  

#pragma warning(disable : 4251)
//:\PROJECTS_2\NESESLIB\NESESLIB\WebContext.hpp(40,25): warning C4251: 'NESES::WebContext::filepath': 'std::filesystem::path' needs to have dll-interface to be used by clients of 'NESES::WebContext'