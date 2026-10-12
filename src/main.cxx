#include "app/application.hxx"
#include "core/utf.hxx"
#include "storage/app_paths.hxx"
#include "storage/log_file.hxx"

#include <Windows.h>

#include <exception>

namespace
{

void reportFatalError(const char* message)
{
    try
    {
        loader::storage::Logger(loader::storage::AppPaths()).error(message);
    }
    catch (const std::exception&)
    {
    }
    MessageBoxW(nullptr, loader::core::utf::widen(message).c_str(), L"loader.gg", MB_OK | MB_ICONERROR);
}

}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    try
    {
        loader::app::Application application;
        return application.run();
    }
    catch (const std::exception& error)
    {
        reportFatalError(error.what());
        return 1;
    }
}
