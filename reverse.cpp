#include <windows.h>
#include <cstdio>
#include <string>
#include <vector>
#include <string.h>
#include "resource.h"
constexpr DWORD BUFFER_SIZE = 4096;

static bool ReportError(const char* step)
{
    DWORD error = GetLastError();
    std::printf("%s failed. Windows error: %lu\n", step, error);
    return false;
}

bool Extractpdf()
{
    HMODULE module = GetModuleHandleW(nullptr);
    if (!module)
        return ReportError("GetModuleHandleW");

    HRSRC resource = FindResourceW(
        module,
        MAKEINTRESOURCEW(IDR_PDF),
        MAKEINTRESOURCEW(10)
    );

    if (!resource)
        return ReportError("FindResourceW");

    DWORD size = SizeofResource(module, resource);
    if (size == 0)
        return ReportError("SizeofResource");

    HGLOBAL loaded = LoadResource(module, resource);
    if (!loaded)
        return ReportError("LoadResource");

    const void* data = LockResource(loaded);
    if (!data)
    {
        std::printf("LockResource returned null.\n");
        return false;
    }

    std::vector<wchar_t> buffer(512);
    DWORD length = 0;

    for (;;)
    {
        length = GetModuleFileNameW(
            nullptr,
            buffer.data(),
            static_cast<DWORD>(buffer.size())
        );

        if (length == 0)
            return ReportError("GetModuleFileNameW");

        if (length < buffer.size())
            break;

        buffer.resize(buffer.size() * 2);
    }

    std::wstring exePath(buffer.data(), length);
    auto separator = exePath.find_last_of(L"\\/");

    if (separator == std::wstring::npos)
    {
        std::printf("Cannot determine executable directory.\n");
        return false;
    }

    std::wstring outputPath =
        exePath.substr(0, separator + 1) + L"windows hardening.pdf";

    HANDLE file = CreateFileW(
        outputPath.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (file == INVALID_HANDLE_VALUE)
        return ReportError("CreateFileW");

    DWORD written = 0;
    if (!WriteFile(file, data, size, &written, nullptr))
    {        ReportError("WriteFile");
        CloseHandle(file);
        return false;
    }

    if (!CloseHandle(file))
        return ReportError("CloseHandle");

    if (written != size)
    {
        std::printf("Incomplete write: %lu of %lu bytes.\n", written, size);
        return false;
    }

    std::printf("PDF extracted next to the executable.\n");
    return true;
}

int main(void)
{
    if (!Extractpdf())
    {
        std::printf("PDF extraction failed.\n");
        return 1;
    }

    std::printf("PDF extracted. Enter a command, or exit to quit.\n");


    while (1)
    {
        char input[1024];
        char command[1200];

        printf("\nShellCommand$: ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("\nInput failed.\n");
            break;
        }

        input[strcspn(input, "\r\n")] = '\0';

        if (input[0] == '\0')
        {
            continue;
        }

        if (_stricmp(input, "exit") == 0)
        {
            break;
        }
        int length = snprintf(
            command,
            sizeof(command),
            "%s",
            input
        );

        if (length < 0 || (size_t)length >= sizeof(command))
        {
            printf("Command is too long.\n");
            continue;
        }

        HANDLE hread = NULL;
        HANDLE hwrite = NULL;

        SECURITY_ATTRIBUTES sa = { 0 };
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = TRUE;

        if (!CreatePipe(&hread, &hwrite, &sa, 0))
        {
            printf("CreatePipe failed: %lu\n", GetLastError());
            continue;
        }

        if (!SetHandleInformation(
            hread,
            HANDLE_FLAG_INHERIT,
            0))
        {
            printf("SetHandleInformation failed: %lu\n", GetLastError());

            CloseHandle(hread);
            CloseHandle(hwrite);
            continue;
        }

        STARTUPINFOA si = { 0 };
        PROCESS_INFORMATION pi = { 0 };

        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;

        si.hStdOutput = hwrite;
        si.hStdError = hwrite;
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

        if (!CreateProcessA(
            NULL,
            command,
            NULL,
            NULL,
            TRUE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &si,
            &pi))
        {
            printf("CreateProcess failed: %lu\n", GetLastError());

            CloseHandle(hread);
            CloseHandle(hwrite);
            continue;
        }

        //printf("\nCommand executed successfully.\n");
        //printf("Output:\n");
        CloseHandle(hwrite);
        hwrite = NULL;
        CloseHandle(pi.hThread);
        pi.hThread = NULL;
        
        char buffer[BUFFER_SIZE];
        DWORD bytesRead;

        while (1)
        {
            BOOL result = ReadFile(
                hread,
                buffer,
                sizeof(buffer),
                &bytesRead,
                NULL
            );

            if (!result)
            {
                DWORD error = GetLastError();
                if (error != ERROR_BROKEN_PIPE)
                {
                    printf("\nReadFile failed: %lu\n", error);
                }

                break;
            }

            if (bytesRead == 0)
            {
                break;
            }
            fwrite(buffer, 1, bytesRead, stdout);
        }

        CloseHandle(hread);
        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
    }

    printf("\nProgram finished.\n");
    return 0;
}
