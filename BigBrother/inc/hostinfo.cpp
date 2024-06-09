#include "hostinfo.h"
#include <stdio.h>
#include <string>
#include <winsock.h>
#include <lmcons.h>
#include <iostream>
#include <algorithm>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

using namespace std;



string GetUser() {
    wchar_t username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;

    GetUserName((wchar_t*)username, &username_len);

    wstring ws(username);
    string result(ws.begin(), ws.end());

    return result;
}

string GetMachine() {
    setlocale(0, "");
    char buffer[256];
    DWORD size = 256;
    GetComputerNameA(buffer, &size);

    return buffer;
}

string GetDomain() {
    DWORD bufSize = MAX_PATH;
    wchar_t domainNameBuf[MAX_PATH];

    GetComputerNameEx(ComputerNameDnsHostname, domainNameBuf, &bufSize);//или ComputerNameDnsDomain

    wstring ws(domainNameBuf);
    string result(ws.begin(), ws.end());

    return result;
}

string GetExternalIP() {
    //string response = SendGETRequest("https://api64.ipify.org?format=json");
    //if (response != "error") {
    //    nlohmann::json j = nlohmann::json::parse(response);
    //    return j.at("ip");
    //}
    string response = SendRequest("http://icanhazip.com");
    response.erase(remove(response.begin(), response.end(), '\n'), response.cend());
    return response;
}




size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

string SendRequest(string url, string fields, string method) {
    CURLcode res;
    struct curl_slist* headers = NULL;
    string readBuffer = "";

    CURL* curl = curl_easy_init();

    if (curl) {
        if (method != "GET") {
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            if (method == "PUT") {
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            }
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields.c_str()); 
        }
        else {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
  
        res = curl_easy_perform(curl);

        //cout << readBuffer << "\n";

        if (res != CURLE_OK) {
            //cout << "Error: " << curl_easy_strerror(res) << endl;
            return "error";
        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

