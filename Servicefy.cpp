#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <windows.h>
#include <shlobj_core.h>
#include <direct.h>

using namespace std;

std::wstring stringToWstring(const std::string& str) {
    std::wstring wstr(str.begin(), str.end());
    return wstr;
}

string getAppDataFolderPath() {
    PWSTR appdata = NULL;
    if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, NULL, &appdata) == S_OK) {
        char dest[MAX_PATH];
        wcstombs_s(NULL, dest, appdata, MAX_PATH);
        printf("AppData path is %s\n", dest);
        return string(dest);
    }
    else {
        fprintf(stderr, "Error getting AppData path\n");
        return "";
    }
}

bool InstallService(const string& serviceName, const string& serviceExePath) {
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (NULL == schSCManager) {
        cout << "Failed to open Service Control Manager: " << GetLastError() << endl;
        return false;
    }

    wstring wServiceName = stringToWstring(serviceName);
    wstring wServiceExePath = stringToWstring(serviceExePath);

    SC_HANDLE schService = CreateService(
        schSCManager,
        wServiceName.c_str(),            // Convert std::string to wide string
        wServiceName.c_str(),            // Display name
        SERVICE_ALL_ACCESS,              // Access rights
        SERVICE_WIN32_OWN_PROCESS,       // Service type
        SERVICE_AUTO_START,              // Start type
        SERVICE_ERROR_NORMAL,            // Error control type
        wServiceExePath.c_str(),         // Path to the service executable
        NULL, NULL, NULL, NULL, NULL     // Dependencies, account info, etc.
    );

    if (schService == NULL) {
        cout << "Failed to create service: " << GetLastError() << endl;
        CloseServiceHandle(schSCManager);
        return false;
    }

    cout << "Service installed successfully." << endl;
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return true;
}

bool UninstallService(const string& serviceName) {
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (NULL == schSCManager) {
        cout << "Failed to open Service Control Manager: " << GetLastError() << endl;
        return false;
    }

    wstring wServiceName = stringToWstring(serviceName);

    SC_HANDLE schService = OpenService(schSCManager, wServiceName.c_str(), DELETE);
    if (schService == NULL) {
        cout << "Failed to open service: " << GetLastError() << endl;
        CloseServiceHandle(schSCManager);
        return false;
    }

    if (!DeleteService(schService)) {
        cout << "Failed to delete service: " << GetLastError() << endl;
    }
    else {
        cout << "Service uninstalled successfully." << endl;
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Error, No parameters provided, exiting now" << endl;
        return 1;
    }

    if (strcmp(argv[1], "--install") == 0) {
        cout << "Installing service" << endl;
        if (argc < 4) {
            cout << "Error, No service name or path provided, exiting" << endl;
            return 1;
        }

        string serviceName = argv[2];
        string exePath = argv[3];

        ifstream file(exePath);
        if (!file) {
            cout << "Error, File does not exist, exiting" << endl;
            return 1;
        }

        if (!InstallService(serviceName, exePath)) {
            cout << "Error, Failed to install service, exiting" << endl;
            return 1;
        }

        string appDataPath = getAppDataFolderPath();
        if (appDataPath.empty()) {
            cout << "Warning, Failed to get AppData path, cannot save program as saved service, exiting" << endl;
            return 1;
        }
        else {
            ofstream file(appDataPath + "\\Servicefy\\services.txt", ios::app);
            if (file.is_open()) {
                file << exePath << endl;
                file.close();
            }
            else {
                cout << "Warning, Failed to open file to save service, exiting" << endl;
                return 1;
            }
        }

    }
    else if (strcmp(argv[1], "--uninstall") == 0) {
        cout << "Uninstalling service" << endl;
        if (argc < 3) {
            cout << "No service name provided" << endl;
            return 1;
        }

        string serviceName = argv[2];

        if (!UninstallService(serviceName)) {
            cout << "Failed to uninstall service" << endl;
            return 1;
        }

        string appDataPath = getAppDataFolderPath();
        if (appDataPath.empty()) {
            cout << "Failed to get AppData path, cannot remove program as saved service" << endl;
            return 1;
        }
        else {
            ifstream file(appDataPath + "\\Servicefy\\services.txt");
            if (file.is_open()) {
                string line;
                ofstream temp(appDataPath + "\\Servicefy\\temp.txt");
                if (temp.is_open()) {
                    while (getline(file, line)) {
                        if (line != serviceName) {
                            temp << line << endl;
                        }
                    }
                    file.close();
                    temp.close();
                    remove((appDataPath + "\\Servicefy\\services.txt").c_str());
                    rename((appDataPath + "\\Servicefy\\temp.txt").c_str(), (appDataPath + "\\Servicefy\\services.txt").c_str());
                }
                else {
                    cout << "Failed to open temp file to save service" << endl;
                    return 1;
                }
            }
            else {
                cout << "Failed to open file to save service" << endl;
                return 1;
            }
        }

    }
    else if (strcmp(argv[1], "--list") == 0) {
        string appDataPath = getAppDataFolderPath();
        if (appDataPath.empty()) {
            cout << "Failed to get AppData path, cannot list saved services" << endl;
            return 1;
        }
        else {
            cout << "Listing services:" << endl;
            ifstream file(appDataPath + "\\Servicefy\\services.txt");
            if (file.is_open()) {
                string line;
                while (getline(file, line)) {
                    cout << line << endl;
                }
                file.close();
            }
            else {
                cout << "Failed to open file to list services" << endl;
                return 1;
            }
        }

    }
    else {
        cout << "Unknown parameter" << endl;
    }

    return 0;
}
