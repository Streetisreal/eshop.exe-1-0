#include <windows.h>
#include <string>
#include <vector>

#pragma comment(lib, "winmm.lib")
#include <windows.h>
#include <string>
#include <vector>
 
#pragma comment(lib, "winmm.lib")

#define IDR_BG_IMAGE 101
#define IDR_AUDIO    102

std::wstring ExtractResourceToTemp(int resourceId, const std::wstring& outputFileName) {
    HMODULE hModule = GetModuleHandle(NULL);
    HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(resourceId), RT_RCDATA);
    if (!hRes) return L"";

    HGLOBAL hData = LoadResource(hModule, hRes);
    if (!hData) return L"";

    DWORD dataSize = SizeofResource(hModule, hRes);
    LPVOID pData = LockResource(hData);

    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring fullPath = std::wstring(tempPath) + outputFileName;

    HANDLE hFile = CreateFileW(fullPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten;
        WriteFile(hFile, pData, dataSize, &bytesWritten, NULL);
        CloseHandle(hFile);
    }

    return fullPath;
}

void SetWallpaper(const std::wstring& path) {
    SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (PVOID)path.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    int response = MessageBox(
        NULL,
        L"This will NOT destroy your computer.\nPlease make sure to save everything before continuing.\n\nPress ALT + S at any time while running to close the program.\n\nDo you want to continue?",
        L"Disclaimer",
        MB_YESNO | MB_ICONWARNING
    );

    if (response != IDYES) {
        return 0;
    }

    std::wstring tempImagePath = ExtractResourceToTemp(IDR_BG_IMAGE, L"temp_3ds_bg.jpg");
    if (!tempImagePath.empty()) {
        SetWallpaper(tempImagePath);
    }

    HMODULE hModule = GetModuleHandle(NULL);
    HRSRC hAudioRes = FindResource(hModule, MAKEINTRESOURCE(IDR_AUDIO), RT_RCDATA);
    if (hAudioRes) {
        HGLOBAL hAudioData = LoadResource(hModule, hAudioRes);
        LPVOID pAudioBytes = LockResource(hAudioData);
        PlaySound((LPCTSTR)pAudioBytes, NULL, SND_MEMORY | SND_ASYNC | SND_LOOP);
    }

    std::vector<PROCESS_INFORMATION> mshtaProcesses;
    std::wstring mshtaCmd = L"mshta \"javascript:x=y=a=0,b=c=1,w=10,h=100;resizeTo(w,h);setInterval(function(){a+=0.1;moveTo(x+=(a*b),y+=(a*c));if(x<0|x>1280-w)b*=-1;if(y<0|y>720-h)c*=-1},20),'eshop :D'\"";

    auto SpawnMshta = [&]() {
        STARTUPINFOW siMshta = { sizeof(siMshta) };
        PROCESS_INFORMATION piMshta;
        if (CreateProcessW(NULL, (LPWSTR)mshtaCmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &siMshta, &piMshta)) {
            mshtaProcesses.push_back(piMshta);
        }
        };

    SpawnMshta();

    std::wstring pyCmd1 = L"cmd.exe /c py -c \"from ctypes import windll as w;u,g,s=w.user32,w.gdi32,0x00660046;d=u.GetDC(0);exec('while 1:\\n g.BitBlt(d,0,1,1920,1080,d,0,0,s)\\n g.BitBlt(d,1,0,1920,1080,d,0,0,s)')\"";
    std::wstring pyCmd2 = L"cmd.exe /c py -c \"import ctypes as c; g=c.windll.gdi32; d=c.windll.user32.GetDC(0); [g.StretchBlt(d, 5, 5, 1910, 1070, d, 0, 0, 1920, 1080, 0x999999) for _ in range(10**9)]\"";

    auto LaunchPythonEffect = [&](const std::wstring& cmd, PROCESS_INFORMATION& piOut) {
        STARTUPINFOW siPy = { sizeof(siPy) };
        CreateProcessW(NULL, (LPWSTR)cmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &siPy, &piOut);
        };

    PROCESS_INFORMATION piPy = { 0 };
    int currentGdiState = 1;

    LaunchPythonEffect(pyCmd1, piPy);

    ULONGLONG lastMshtaSpawnTime = GetTickCount64();
    ULONGLONG gdiEffectStartTime = GetTickCount64();

    while (true) {
        if ((GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState('S') & 0x8000)) {
            PlaySound(NULL, 0, 0);

            for (const auto& pi : mshtaProcesses) {
                TerminateProcess(pi.hProcess, 0);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }

            system("taskkill /f /im python.exe >nul 2>&1");

            CloseHandle(piPy.hProcess);
            CloseHandle(piPy.hThread);
            break;
        }

        ULONGLONG currentTime = GetTickCount64();

        if (currentTime - lastMshtaSpawnTime >= 5000) {
            SpawnMshta();
            lastMshtaSpawnTime = currentTime;
        }

        if (currentGdiState == 1 && (currentTime - gdiEffectStartTime >= 20000)) {
            system("taskkill /f /im python.exe >nul 2>&1");
            CloseHandle(piPy.hProcess);
            CloseHandle(piPy.hThread);

            LaunchPythonEffect(pyCmd2, piPy);
            currentGdiState = 2;
            gdiEffectStartTime = currentTime;
        }
        else if (currentGdiState == 2 && (currentTime - gdiEffectStartTime >= 20000)) {
            system("taskkill /f /im python.exe >nul 2>&1");
            CloseHandle(piPy.hProcess);
            CloseHandle(piPy.hThread);

            LaunchPythonEffect(pyCmd1, piPy);
            currentGdiState = 1;
            gdiEffectStartTime = currentTime;
        }

        Sleep(100);
    }

    return 0;
}
// Resource IDs matching the .rc file
#define IDR_BG_IMAGE 101
#define IDR_AUDIO    102

// Function to extract an embedded resource to a temporary file path
std::wstring ExtractResourceToTemp(int resourceId, const std::wstring& outputFileName) {
    HMODULE hModule = GetModuleHandle(NULL);
    HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(resourceId), RT_RCDATA);
    if (!hRes) return L"";

    HGLOBAL hData = LoadResource(hModule, hRes);
    if (!hData) return L"";

    DWORD dataSize = SizeofResource(hModule, hRes);
    LPVOID pData = LockResource(hData);

    // Get Windows Temp folder path
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring fullPath = std::wstring(tempPath) + outputFileName;

    // Write data to the temp file
    HANDLE hFile = CreateFileW(fullPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten;
        WriteFile(hFile, pData, dataSize, &bytesWritten, NULL);
        CloseHandle(hFile);
    }

    return fullPath;
}

// Function to change the desktop wallpaper
void SetWallpaper(const std::wstring& path) {
    SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (PVOID)path.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 1. Warning Prompt
    int response = MessageBox(
        NULL,
        L"This will NOT destroy your computer.\nPlease make sure to save everything before continuing.\n\nPress ALT + S at any time while running to close the program.\n\nDo you want to continue?",
        L"Disclaimer",
        MB_YESNO | MB_ICONWARNING
    );

    if (response != IDYES) {
        return 0;
    }

    // 2. Extract embedded image to temp folder and apply wallpaper
    std::wstring tempImagePath = ExtractResourceToTemp(IDR_BG_IMAGE, L"temp_3ds_bg.jpg");
    if (!tempImagePath.empty()) {
        SetWallpaper(tempImagePath);
    }

    // 3. Play audio directly from memory (Make sure your resource is a .WAV file!)
    HMODULE hModule = GetModuleHandle(NULL);
    HRSRC hAudioRes = FindResource(hModule, MAKEINTRESOURCE(IDR_AUDIO), RT_RCDATA);
    if (hAudioRes) {
        HGLOBAL hAudioData = LoadResource(hModule, hAudioRes);
        LPVOID pAudioBytes = LockResource(hAudioData);
        PlaySound((LPCTSTR)pAudioBytes, NULL, SND_MEMORY | SND_ASYNC | SND_LOOP);
    }

    // Keep track of all spawned mshta processes
    std::vector<PROCESS_INFORMATION> mshtaProcesses;
    std::wstring mshtaCmd = L"mshta \"javascript:x=y=a=0,b=c=1,w=10,h=100;resizeTo(w,h);setInterval(function(){a+=0.1;moveTo(x+=(a*b),y+=(a*c));if(x<0|x>1280-w)b*=-1;if(y<0|y>720-h)c*=-1},20),'eshop party :D'\"";

    auto SpawnMshta = [&]() {
        STARTUPINFOW siMshta = { sizeof(siMshta) };
        PROCESS_INFORMATION piMshta;
        if (CreateProcessW(NULL, (LPWSTR)mshtaCmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &siMshta, &piMshta)) {
            mshtaProcesses.push_back(piMshta);
        }
        };

    // Spawn the first mshta window immediately
    SpawnMshta();

    // Python GDI Effect Commands
    std::wstring pyCmd1 = L"cmd.exe /c py -c \"from ctypes import windll as w;u,g,s=w.user32,w.gdi32,0x00660046;d=u.GetDC(0);exec('while 1:\\n g.BitBlt(d,0,1,1920,1080,d,0,0,s)\\n g.BitBlt(d,1,0,1920,1080,d,0,0,s)')\"";
    std::wstring pyCmd2 = L"cmd.exe /c py -c \"import ctypes as c; g=c.windll.gdi32; d=c.windll.user32.GetDC(0); [g.StretchBlt(d, 5, 5, 1910, 1070, d, 0, 0, 1920, 1080, 0x999999) for _ in range(10**9)]\"";

    // Function to launch a Python GDI effect
    auto LaunchPythonEffect = [&](const std::wstring& cmd, PROCESS_INFORMATION& piOut) {
        STARTUPINFOW siPy = { sizeof(siPy) };
        CreateProcessW(NULL, (LPWSTR)cmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &siPy, &piOut);
        };

    PROCESS_INFORMATION piPy = { 0 };
    int currentGdiState = 1; // 1 = First effect, 2 = Second effect

    // Start with GDI Effect 1 immediately
    LaunchPythonEffect(pyCmd1, piPy);

    // 4. Hotkey Listener & Timers Loop
    ULONGLONG lastMshtaSpawnTime = GetTickCount64();
    ULONGLONG gdiEffectStartTime = GetTickCount64();

    while (true) {
        // Check if ALT + S is pressed to exit
        if ((GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState('S') & 0x8000)) {
            // Stop looping audio
            PlaySound(NULL, 0, 0);

            // Terminate all spawned mshta processes
            for (const auto& pi : mshtaProcesses) {
                TerminateProcess(pi.hProcess, 0);
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }

            // Forcefully kill any running python instances
            system("taskkill /f /im python.exe >nul 2>&1");

            CloseHandle(piPy.hProcess);
            CloseHandle(piPy.hThread);
            break;
        }

        ULONGLONG currentTime = GetTickCount64();

        // Check if 5 seconds have passed to spawn another mshta window
        if (currentTime - lastMshtaSpawnTime >= 5000) {
            SpawnMshta();
            lastMshtaSpawnTime = currentTime;
        }

        // Check GDI effect state machine switching
        if (currentGdiState == 1 && (currentTime - gdiEffectStartTime >= 20000)) {
            // After 20 seconds, kill Python GDI 1 and switch to GDI 2
            system("taskkill /f /im python.exe >nul 2>&1");
            CloseHandle(piPy.hProcess);
            CloseHandle(piPy.hThread);

            LaunchPythonEffect(pyCmd2, piPy);
            currentGdiState = 2;
            gdiEffectStartTime = currentTime;
        }
        else if (currentGdiState == 2 && (currentTime - gdiEffectStartTime >= 20000)) {
            // Optionally loop back to Effect 1 after another 20 seconds (or adjust timing as needed)
            system("taskkill /f /im python.exe >nul 2>&1");
            CloseHandle(piPy.hProcess);
            CloseHandle(piPy.hThread);

            LaunchPythonEffect(pyCmd1, piPy);
            currentGdiState = 1;
            gdiEffectStartTime = currentTime;
        }

        Sleep(100);
    }

    return 0;
}
