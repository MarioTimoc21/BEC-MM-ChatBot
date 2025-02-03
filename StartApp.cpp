#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <windows.h>
#include <direct.h>

bool runExecutable(const std::string& path) {
    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    PROCESS_INFORMATION pi;

    std::wstring widePath(path.begin(), path.end());

    if (CreateProcessW(widePath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return true;
    } else {
        std::cerr << "Error running: " << path << std::endl;
        return false;
    }
}

int main() {
    if (_chdir("ai\\dist") != 0) {
        std::cerr << "Failed to set working directory to ai\\dist.\n";
        return 1;
    }

    std::cout << "Starting server...\n";
    if (!runExecutable("main.exe")) {
        std::cerr << "Failed to run main.exe.\n";
        return 1;
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
    if (_chdir("..\\..\\APP") != 0) {
        std::cerr << "Failed to set working directory to APP.\n";
        return 1;
    }

    if (!runExecutable("BECMMChatBot.exe")) {
        std::cerr << "Failed to run BECMMChatBot.exe.\n";
        return 1;
    }

    return 0;
}
