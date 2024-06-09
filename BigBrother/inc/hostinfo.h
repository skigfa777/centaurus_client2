#pragma once
#ifndef hosinfo_H
#define hosinfo_H
#include <iostream>
using namespace std;


string GetUser();

string GetMachine();

string GetDomain();

string GetExternalIP();

string SendRequest(string url, string fields = "", string method = "GET");

#endif

