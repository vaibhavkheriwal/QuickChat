#include <iostream>
#include <string>
#include <curl/curl.h>
#include <ctime>
#include "json.hpp"

#define APP_VERSION "V1.0.0"

using namespace std;
using json = nlohmann::json;

string response_data;

// CURL CALLBACK
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t totalSize = size * nmemb;
    response_data.append((char *)contents, totalSize);
    return totalSize;
}

// FORMAT DATE FUNCTION
string formatDate(long timestamp)
{
    time_t t = timestamp;
    tm *timeInfo = localtime(&t);

    char buffer[100];
    strftime(buffer, sizeof(buffer), "%d %B %Y %I:%M %p", timeInfo);

    return string(buffer);
}

// FORMAT USER FUNCTION
string formatUser(string userCode)
{
    if (userCode == "a")
    {
        return "User A";
    }
    else if (userCode == "b")
    {
        return "User B";
    }
    return "Unknown";
}

// FETCH AND PRINT FUNCTION
void fetchAndPrintChat()
{
    response_data.clear();

    CURL *curl = curl_easy_init();
    if (!curl)
    {
        cout << "Curl init failed.\n";
        return;
    }

    string url = "YOUR_DATABASE_ENDPOINT";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    // For testing
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        cout << "Curl Error: " << "No internet connection or " << curl_easy_strerror(res) << endl
             << endl;
        curl_easy_cleanup(curl);
        return;
    }

    curl_easy_cleanup(curl);

    if (response_data == "null")
    {
        cout << "No messages yet.\n";
        return;
    }

    json data = json::parse(response_data);

    for (auto &[key, value] : data.items())
    {

        string user = value["user"];
        string message = value["message"];
        long timestamp = value["time"];

        string name = formatUser(user);
        string dateTime = formatDate(timestamp);

        cout << name << "\t" << "  " << dateTime
             << "\t  >  " << message << endl;
    }
}

void sendMessage(string userCode, string messageText)
{
    CURL *curl = curl_easy_init();
    if (!curl)
        return;

    string url = "YOUR_DATABASE_ENDPOINT";

    // Create JSON payload
    json payload;
    payload["user"] = userCode;
    payload["message"] = messageText;
    payload["time"] = time(nullptr);

    string jsonData = payload.dump();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
}

void enterCurrentUser(char &currentUser)
{
    do
    {
        cout << "Enter your username: ";
        cin >> currentUser;
        cin.ignore();
        if (currentUser != 'a' && currentUser != 'b')
        {
            cout << "Invalid User." << endl
                 << endl;
        }

    } while (currentUser != 'a' && currentUser != 'b');
}

bool handleUserInput(string input, char currentUser)
{
    if (input == "QCEXIT")
    {
        return false; // stop app
    }
    else if (input == "QCHELP")
    {
        cout << "  QCHELP  - View available commands\n";
        cout << "  QCEXIT  - Exit the application\n";
    }
    else if (!input.empty())
    {
        sendMessage(string(1, currentUser), input);
    }

    return true; // continue running
}

void enableUTF8()
{
#ifdef _WIN32
    system("chcp 65001 > nul");
#endif
}

void startScreen()
{
    cout << R"(
    ████████ ██    ██ ██ ███████  ██   ██    ███████  ██   ██ ███████ ████████
    ██    ██ ██    ██ ██ ██       ██  ██     ██       ██   ██ ██   ██    ██
    ██    ██ ██    ██ ██ ██       █████      ██       ███████ ███████    ██
    ██ ▄▄ ██ ██    ██ ██ ██       ██  ██     ██       ██   ██ ██   ██    ██
    ████████ ████████ ██ ███████  ██   ██    ███████  ██   ██ ██   ██    ██
         ██
    )" << endl;

    cout << "Welcome to Quick Chat " << APP_VERSION << "." << endl
         << "  QCHELP  - View available commands" << endl
         << "  QCEXIT  - Exit the application" << endl
         << endl;
}

int main()
{
    enableUTF8();
    curl_global_init(CURL_GLOBAL_DEFAULT);

    char currentUser;
    string input;

    startScreen();
    enterCurrentUser(currentUser);
    system("cls");
    startScreen();

    bool appRunning = true;

    while (appRunning)
    {
        fetchAndPrintChat();

        cout << "\nType your message: ";
        getline(cin, input);

        appRunning = handleUserInput(input, currentUser);
        system("cls");
        startScreen();
    }

    curl_global_cleanup();
    return 0;
}
