#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <shlobj_core.h>
#include <direct.h>

using namespace std;

static bool runCommand(const string& command) {
    cout << "Running command: " << command << endl;
    int result = system(command.c_str());
    if (result != 0) {
        cout << "Command failed with error code: " << result << endl;
        return false;
    }
    return true;
}

string getAppDataFolderPath() {
    PWSTR appdata = NULL;
	if (SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, NULL, &appdata) == S_OK) {
        char dest[MAX_PATH];
		wcstombs_s(NULL, dest, appdata, MAX_PATH);
        printf("appdata path is %s\n", dest);
        wcstombs_s(NULL, dest, appdata, MAX_PATH - 1);
        dest[MAX_PATH - 1] = '\0';
        return string(dest);
    }
    else {
        fprintf(stderr, "error getting appdata path\n");
		return "";
    }
}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        cout << "Error, No parameters provided, exiting now" << endl;
        return 1;
    }

    if (strcmp(argv[1], "--install") == 0)
    {
        cout << "Installing service" << endl;
        if (argc < 3)
        {
            cout << "Error, No path provided, exiting" << endl;
            return 1;
        }

        string exePath = argv[2];

        if (exePath.find(".exe") == string::npos)
        {
            cout << "Error, Not a valid exe file, exiting" << endl;
            return 1;
        }

        ifstream file(exePath);
        if (!file) {
            cout << "Error, File does not exist, exiting" << endl;
            return 1;
        }

        string command = "InstallUtil.exe " + exePath;

        if (!runCommand(command)) {
            cout << "Error, Failed to install service, exiting" << endl;
            return 1;
        }

        cout << "Service installed" << endl;

		string appDataPath = getAppDataFolderPath();
        if (appDataPath.empty()) {
            cout << "Warning, Failed to get appdata path, can not save program as saved service, exiting" << endl;
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
    else if (strcmp(argv[1], "--uninstall") == 0)
    {
        cout << "Uninstalling service" << endl;
        if (argc < 3)
        {
            cout << "No path provided" << endl;
            return 1;
        }

        string exePath = argv[2];

        if (runCommand("InstallUtil.exe /uninstall " + exePath)) {
            cout << "Service uninstalled" << endl;
        }
        else {
            cout << "Failed to uninstall service" << endl;
            return 1;
        }

		string appDataPath = getAppDataFolderPath();

		if (appDataPath.empty()) {
			cout << "Failed to get appdata path, can not remove program as saved service" << endl;
			return 1;
		}
		else {
			ifstream file(appDataPath + "\\Servicefy\\services.txt");
			if (file.is_open()) {
				string line;
				ofstream temp(appDataPath + "\\Servicefy\\temp.txt");
				if (temp.is_open()) {
					while (getline(file, line)) {
						if (line != exePath) {
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
    else if (strcmp(argv[1], "--list") == 0)
    {
		string appDataPath = getAppDataFolderPath();
        if (appDataPath.empty()) {
            cout << "Failed to get appdata path, can not list saved services" << endl;
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
    else
    {
        cout << "Unknown parameter" << endl;
    }

    return 0;
}
