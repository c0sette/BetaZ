#include "Header.h"
using namespace std;

//************************
//*     VARIABLES       *
enum ScanCode
{
    SCANCODE_X = 0x2D,
    SCANCODE_Y = 0x15,
    SCANCODE_ESC = 0x01,
    SCANCODE_LMB_DOWN = 0x1,
    SCANCODE_LMB_UP = 0x2,
    SCANCODE_RMB_DOWN = 0x4,
    SCANCODE_RMB_UP = 0x8,
    SCANCODE_MMB_ROLL = 1024
};
struct Settings
{
    string Key;
    string Wifi_1;
    string Wifi_2;
    string token;
    string interface_wifi;
};

CaptureD3D11 c_D3D11;
int press = 0;
int capture_times = 0;
int nTick = 0;
POINT p1;
POINT p2;
int sen = 0;
int c = 0;
string sx[50];
int json_pos = 0;
int command = 0;
Settings settings;
vector <string> uploaded_file;
 //************************
void CaptureSRC();
void ShowAnswer();
long check_key_api(string input_key, int type);
void StartupApp();
int send_open_app();
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
bool SuspendProcessByName(const char* processName, bool suspend) {
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Take a snapshot of all running processes
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create snapshot of processes." << std::endl;
        return false;
    }

    // Iterate through the processes
    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (_stricmp(pe32.szExeFile, processName) == 0) {
                // Found the process, try to suspend or resume it
                HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pe32.th32ProcessID);
                if (hProcess != NULL) {
                    if (suspend) 
                    {
                        // Suspend all threads in the process
                        HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
                        if (hThreadSnapshot != INVALID_HANDLE_VALUE) {
                            THREADENTRY32 te32;
                            te32.dwSize = sizeof(THREADENTRY32);

                            if (Thread32First(hThreadSnapshot, &te32)) {
                                do {
                                    if (te32.th32OwnerProcessID == pe32.th32ProcessID) {
                                        HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
                                        if (hThread != NULL) {
                                            // Suspend the thread
                                            SuspendThread(hThread);
                                            CloseHandle(hThread);
                                        }
                                    }
                                } while (Thread32Next(hThreadSnapshot, &te32));
                            }

                            CloseHandle(hThreadSnapshot);
                        }
                    }
                    else {
                        // Resume all threads in the process
                        HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
                        if (hThreadSnapshot != INVALID_HANDLE_VALUE) {
                            THREADENTRY32 te32;
                            te32.dwSize = sizeof(THREADENTRY32);

                            if (Thread32First(hThreadSnapshot, &te32)) {
                                do {
                                    if (te32.th32OwnerProcessID == pe32.th32ProcessID) {
                                        HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
                                        if (hThread != NULL) {
                                            // Resume the thread
                                            ResumeThread(hThread);
                                            ResumeThread(hThread);
                                            CloseHandle(hThread);
                                        }
                                    }
                                } while (Thread32Next(hThreadSnapshot, &te32));
                            }

                            CloseHandle(hThreadSnapshot);
                        }
                    }

                    CloseHandle(hProcess);
                    CloseHandle(hSnapshot);
                    return true;
                }
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return false; // Process not found or failed to suspend/resume
}
void DownloadAnswer()
{
    UINT oldcp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_ALL);
    string result;
    curl = curl_easy_init();
    if (curl)
    {
        struct curl_slist* headerlist = NULL;
        static const char buf[] = "Expect :";

        string content = "Content-Type: text/html; charset=UTF-8";
        string header_key = "x-access-key: " + settings.Key;
        string header_token = "x-access-token: " + settings.token;
        headerlist = curl_slist_append(headerlist, header_key.c_str());
        headerlist = curl_slist_append(headerlist, header_token.c_str());
        headerlist = curl_slist_append(headerlist, content.c_str());
        headerlist = curl_slist_append(headerlist, buf);
        char key[150];
        sprintf(key, "http://103.153.73.103/api/support/messages");

        curl_easy_setopt(curl, CURLOPT_URL, key);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 7L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cout << "error " << res << endl;
        }
        if (res == CURLE_OK)
        {
            if (result.find("301 Moved") == string::npos && result.find("<!doctype html><html><head><title>") == string::npos && result.find("<!DOCTYPE HTML PUBLIC") == string::npos && result.find("404 Not Found") == string::npos)
            {
                tiny::TinyJson json;
                json.ReadJson(result);
                cout << "Result CURL from down_file():" << result << endl;
                tiny::xarray data = json.Get<tiny::xarray>("data");
                string message;
                cout << "File read from server : " << message << endl;
                size_t total_count = data.Count();

                if (total_count > 0)
                {
                    try {

                        printf("Json pos:%d\n", json_pos);
                        data.Enter(json_pos);
                        message = data.Get<string>("value");
                        json_pos++;
                        if (json_pos >= total_count) json_pos = 0;
                    }
                    catch (exception e)
                    {
                        cout << e.what() << endl;
                    }
                }
                char cmd[100];
                std::ofstream outfile("read.txt", std::ios::out | std::ios::binary);
                size_t pos = 0;
                string newLineChar = "\\n";
                string replacement = "\n";
                while ((pos = message.find(newLineChar, pos)) != string::npos) {
                    message.replace(pos, newLineChar.length(), replacement);
                    pos += replacement.length();
                }
                string result = "";
                size_t z = 0;
                while (z < message.length()) {
                    if (message.substr(z, 2) == "\\u") {
                        unsigned int unicodeValue;
                        sscanf(message.substr(z + 2, 4).c_str(), "%x", &unicodeValue);
                        char utf8[5] = { 0 };
                        if (unicodeValue <= 0x7f) {
                            utf8[0] = static_cast<char>(unicodeValue);
                        }
                        else if (unicodeValue <= 0x7ff) {
                            utf8[0] = static_cast<char>((unicodeValue >> 6) | 0xc0);
                            utf8[1] = static_cast<char>((unicodeValue & 0x3f) | 0x80);
                        }
                        else if (unicodeValue <= 0xffff) {
                            utf8[0] = static_cast<char>((unicodeValue >> 12) | 0xe0);
                            utf8[1] = static_cast<char>(((unicodeValue >> 6) & 0x3f) | 0x80);
                            utf8[2] = static_cast<char>((unicodeValue & 0x3f) | 0x80);
                        }
                        result += utf8;
                        z += 6;
                    }
                    else {
                        result += message[z];
                        z++;
                    }
                }
                outfile << "\xEF\xBB\xBF" << result;
                outfile.close();

                sprintf(cmd, R"("move read.txt "C:/Windows"")");
                system(cmd);
            }
        }
        curl_slist_free_all(headerlist);
        curl_easy_cleanup(curl);
    }
    SetConsoleOutputCP(oldcp);
}
bool PostFile(string link, string filename, string postfilename) {
    CURL* curl;
    CURLcode res;
    string readBuffer;
    string data;
    bool result = false;
    struct curl_httppost* formpost = NULL;
    struct curl_httppost* lastptr = NULL;
    struct curl_slist* headerlist = NULL;
    static const char buf[] = "Expect :";

    curl_global_init(CURL_GLOBAL_ALL);
    string header_key = "x-access-key: " + settings.Key;
    string header_token = "x-access-token: " + settings.token;
    headerlist = curl_slist_append(headerlist, header_key.c_str());
    headerlist = curl_slist_append(headerlist, header_token.c_str());
    curl_formadd(&formpost,
        &lastptr,
        CURLFORM_COPYNAME, postfilename.c_str(),
        CURLFORM_FILE, filename.c_str(),
        CURLFORM_END);

    curl_formadd(&formpost,
        &lastptr,
        CURLFORM_COPYNAME, "filename",
        CURLFORM_COPYCONTENTS, filename.c_str(),
        CURLFORM_END);
    curl_formadd(&formpost,
        &lastptr,
        CURLFORM_COPYNAME, "key",
        CURLFORM_COPYCONTENTS, settings.Key.c_str(),
        CURLFORM_END);
    curl_formadd(&formpost,
        &lastptr,
        CURLFORM_COPYNAME, "from",
        CURLFORM_COPYCONTENTS, "software",
        CURLFORM_END);
    curl = curl_easy_init();
    headerlist = curl_slist_append(headerlist, buf);

    if (!curl) return "Error";

    curl_easy_setopt(curl, CURLOPT_URL, link.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
    tiny::TinyJson json;
    json.ReadJson(readBuffer);
    data = json.Get<string>("message");
    if (data == "success") result = true;
    curl_easy_cleanup(curl);
    curl_formfree(formpost);
    curl_slist_free_all(headerlist);

    return result;
}
std::vector<std::string> vec_file_extension(const std::string& file_extension) {
    std::vector<std::string> names;
    std::string search_path = "*." + file_extension;
    WIN32_FIND_DATA fd;
    HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                names.push_back(fd.cFileName);
            }
        } while (::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }
    return names;
}
void up_file_png()
{
    try
    {
        vector<string>query_file = vec_file_extension("jpeg");
        if (query_file.size() > 0)
        {
            std::vector<std::string> new_files;
            for (int j = 0; j < query_file.size(); j++)
            {
                if (std::count(uploaded_file.begin(), uploaded_file.end(), query_file[j]) < 1)
                {
                    string url = "http://103.153.73.103/api/support/upfile";
                    string filePath = query_file[j];
                    bool result = PostFile(url, filePath, "support_file");
                    if (result == true)
                    {
                        uploaded_file.push_back(query_file[j]);
                        new_files.push_back(query_file[j]);
                        //filesystem::remove(filePath.c_str());
                    }
                }
            }
        }
    }
    catch (exception e)
    {
        cout << e.what() << endl;
    }
}
DWORD WINAPI sync_upload(LPVOID ds)
{
    while (1)
    {

        if (settings.Key.length() <= 1) continue;
        up_file_png();
        Sleep(3000);
        if (command == 1)
        {
            DownloadAnswer();
            command = 0;
        }
    }
}
int main()
{
    StartupApp();
    
    InterceptionContext context;
    InterceptionDevice device;
    InterceptionKeyStroke stroke;
    raise_process_priority();
   
    context = interception_create_context();
    //cout << context << endl;
    interception_set_filter(context, interception_is_keyboard , INTERCEPTION_FILTER_KEY_UP | INTERCEPTION_FILTER_KEY_DOWN);
    interception_set_filter(context, interception_is_mouse, INTERCEPTION_MOUSE_WHEEL | INTERCEPTION_FILTER_MOUSE_LEFT_BUTTON_DOWN | INTERCEPTION_FILTER_MOUSE_LEFT_BUTTON_UP | INTERCEPTION_FILTER_MOUSE_RIGHT_BUTTON_UP | INTERCEPTION_FILTER_MOUSE_RIGHT_BUTTON_DOWN);
    while (interception_receive(context, device = interception_wait(context), (InterceptionStroke*)&stroke, 1) > 0)
    {
        cout << context << endl;
        cout << device << endl;
        if (interception_is_keyboard(device))
        {
            cout << "Keyboard Input "
                << "ScanCode=" << stroke.code
                << " State=" << stroke.state << endl;

            static bool ctrl_pressed = false;  // Static variable to remember CTRL state

            if (stroke.state == 0)
            {
                if (stroke.code == 29)  // CTRL key is pressed
                {
                    ctrl_pressed = true;
                }
                else if (stroke.code == 45 && ctrl_pressed)  // X key is released after CTRL
                {
                    cout << "CTRL+X detected!" << endl;
                    //CaptureSRC();
                    system("taskkill /F /T /IM tip.exe");
                    exit(1);
                }
            }
            else if (stroke.state == 1)
            {
                if (stroke.code == 51)  // F2 supspend
                {
                    ctrl_pressed = false;
                    //("chrome.exe", true);
                }
                if (stroke.code == 52) //F3 unsupspend
                {
                    //SuspendProcessByName("chrome.exe", false);
                }
                if (stroke.code == 26)
                {
                    char c[100];
                    //system("netsh wlan disconnect");
                    //sprintf_s(c, R"(netsh wlan connect name="%s" interface="%s")", settings.Wifi_1.c_str(), settings.interface_wifi.c_str());
                   //system(c);
                    CaptureSRC();
                    system("taskkill /F /T /IM tip.exe");
                }
                if (stroke.code == 27)
                {
                    //char c[100];
                    //system("netsh wlan disconnect");
                    //sprintf_s(c, R"(netsh wlan connect name="%s" interface="%s")", settings.Wifi_2.c_str(), settings.interface_wifi.c_str());
                   //system(c);
                    command = 1;
                    ShowAnswer();
                }
            }
            wchar_t hardware_id[500];
            size_t length = interception_get_hardware_id(context, device, hardware_id, sizeof(hardware_id));

            if (length > 0 && length < sizeof(hardware_id))
                wcout << hardware_id << endl;
            interception_send(context, device, (InterceptionStroke*)&stroke, 1);
        }
        if (interception_is_mouse(device))
        {
            InterceptionMouseStroke& mstroke = *(InterceptionMouseStroke*)&stroke;
            if (stroke.code == SCANCODE_LMB_DOWN)
            {
                nTick = 0;
                nTick = GetTickCount();
                GetCursorPos(&p1);
                int zz = mstroke.x;
            }
            if (stroke.code == SCANCODE_LMB_UP)
            {
                GetCursorPos(&p2);
                int zz = mstroke.x;
                if (GetTickCount() - nTick > 250 && nTick != 0)
                {
                    if (p1.x - p2.x >= 150 && abs(p1.y - p2.y) <= 150)
                    {
                        //printf("Huong sang trai\n");

                    }
                    else if (p1.y - p2.y >= 15 && abs(p1.x - p2.x) <= 150)
                    {
                        printf("Huong len tren\n");
                        CaptureSRC();
                        
                    }
                    else if (p2.y - p1.y >= 25 && abs(p1.x - p2.x) <= 150)
                    {
                        printf("Huong xuong duoi\n");
                        command = 1;
                        ShowAnswer();
                    }
                }
                nTick = 0;
            }
            if (stroke.code == SCANCODE_RMB_DOWN)
            {
                printf("Right down\n");
            }
            if (stroke.code == SCANCODE_RMB_UP)
            {
                printf("Right up\n");
            }
            if (stroke.code == SCANCODE_MMB_ROLL && nTick != 0)
            {
                if (mstroke.rolling = -120)
                {
                    system("taskkill /F /T /IM tip.exe");
                }
            }
            interception_send(context, device, (InterceptionStroke*)&stroke, 1);
        }
    }
    interception_destroy_context(context);

    return 0;
}

void CaptureSRC()
{
    std::cout << "--------------------------------------" << std::endl;
    std::cout << "Captured HWND test" << std::endl;
    HWND hwnd = GetForegroundWindow();
    HRESULT result;
    {
        try {
           
            result = c_D3D11.CaptureScreenD3D11_Test(hwnd,settings.Key);
            capture_times++;
        }
        catch (exception e)
        {
            std::cout << e.what() << std::endl;
        }
    }
    std::cout << "-------------------------------" << std::endl;
    std::cout << "Result capture screen: " << result << std::endl;
}
void ShowAnswer()
{

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char cmd[500];
   
    try 
    {
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        sprintf(cmd, R"(C:\\Windows\\System32\\cmd.exe chcp 65001 /C tip.exe)");
        string command(cmd);
        if (!CreateProcess(NULL,   // No module name (use command line)
            &command[0],        // Command line
            NULL,           // Process handle not inheritable
            NULL,           // Thread handle not inheritable
            FALSE,          // Set handle inheritance to FALSE
            0,              // No creation flags
            NULL,           // Use parent's environment block
            NULL,           // Use parent's starting directory 
            &si,            // Pointer to STARTUPINFO structure
            &pi)           // Pointer to PROCESS_INFORMATION structure
            )
            printf("tip\n");
    }
    catch (exception e)
    {
        cout << e.what() << endl;
    }
}
void StartupApp()
{
    std::time_t open_time = std::time(0);   // get time now
    std::tm* now = std::localtime(&open_time);
    int year = now->tm_year + 1900;
    int month = now->tm_mon + 1;
    int day = now->tm_mday;
    if (year != 2023)
    {
        exit(1);
    }
    if (month <9 && month >12)
    {
        exit(1);
    }

    SetConsoleTitleA("");
    ShowWindow(GetConsoleWindow(), SW_HIDE);


    mINI::INIFile files("config.ini");

    // next, create a structure that will hold data
    mINI::INIStructure ini;

    // now we can read the file
    files.read(ini);

    // read a value
    settings.Key = ini["APP"]["key"];
    settings.Wifi_1 = ini["APP"]["wf1"];
    settings.Wifi_2 = ini["APP"]["wf2"];
    settings.interface_wifi = ini["APP"]["interface"];
    std::ifstream file("C:/Users/profile1.bin");
    if (file) 
    {
        std::stringstream buffer;
        buffer << file.rdbuf();
        settings.token = buffer.str();
    }
   // c_D3D11.Init();  //Cài đặt adapter
    if (check_key_api(settings.Key, 1) == 504)
    {
        //MessageBoxA(GetConsoleWindow(), "Successs", "", MB_OK);
        cout << "Active success";
        c_D3D11.Init();  //Cài đặt adapter
    }
    else
    {
        MessageBoxA(GetConsoleWindow(), "Key khong hop le", "", MB_OK);
        exit(1);
    }
}
long check_key_api(string input_key, int type)
{

    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_ALL);
    string result = "";
    long response_code = 999;
    struct curl_httppost* post = NULL;
    struct curl_httppost* last = NULL;
    if (type == 1) //Check while open folder
    {
        if (settings.Key.length() >= 5 && settings.token.length() > 10 )
        {
            /*if (send_open_app())
            {
                DWORD id;
                CreateThread(NULL, 128, sync_upload, NULL, 0, &id);
                response_code = 504;
            }*/
            DWORD id;
            CreateThread(NULL, 128, sync_upload, NULL, 0, &id);
            response_code = 504;
            return response_code;
        }
        else if (settings.token.length() == 0 && settings.Key.length() <=1)
        {
            string error_code = "Key khong hop le 2";
            MessageBoxA(GetConsoleWindow(), error_code.c_str(), "", MB_OK);
            exit(1);
        }
    }
    curl = curl_easy_init();


    if (curl && settings.token.length() == 0)
    {
        char key[250];
        sprintf(key, "http://103.153.73.103/api/support/auth");

        printf("API send:%s\n", key);

        curl_easy_setopt(curl, CURLOPT_URL, key);
        sprintf(key, "key=%s&device=tool&from=software&student_code=YWJj\0", input_key.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, key);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 7L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cout << "error " << res << endl;
        }
        if (res == CURLE_OK)
        {
            cout << result << endl;
            string token = " ";
            tiny::TinyJson json;
            json.ReadJson(result);
            tiny::xarray data = json.Get<tiny::xarray>("data");

            if (data.Count() > 0)
            {
                data.Enter(0);
                token = data.Get<string>("access_token");
            }

            cout << "Token: " << token << endl;
            if (token.length() > 3)
            {
                MessageBoxA(GetConsoleWindow(), "Kich hoat thanh cong, vui long khoi dong lai ung dung", "", MB_OK);
               
                std::ofstream outfile("C:\\Users\\profile1.bin", std::ios::out | std::ios::binary);
                outfile.write(token.c_str(), token.length());
                exit(1);
            }
            else if (token.length() < 3)
            {
                string error_code = "Key khong hop le 1";
                MessageBoxA(GetConsoleWindow(), error_code.c_str(), "", MB_OK);
                exit(1);
            }
            printf("I'M HERE1\n");


        }
        curl_easy_cleanup(curl);
    }
    return response_code;
}
int send_open_app()
{
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_ALL);
    string result = "";
    long response_code = 999;
    struct curl_httppost* post = NULL;
    struct curl_httppost* last = NULL;
    curl = curl_easy_init();
    if (curl)
    {
        char key[1280];
        struct curl_httppost* formpost = NULL;
        struct curl_httppost* lastptr = NULL;
        struct curl_slist* headerlist = NULL;
        static const char buf[] = "Expect :";


        string header_key = "x-access-key: " + settings.Key;
        string header_token = "x-access-token: " + settings.token;
        headerlist = curl_slist_append(headerlist, header_key.c_str());
        headerlist = curl_slist_append(headerlist, header_token.c_str());
        headerlist = curl_slist_append(headerlist, buf);
        sprintf(key, "http://103.153.73.103/api/support/messages");
       
        curl_easy_setopt(curl, CURLOPT_URL, key);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        res = curl_easy_perform(curl);
        curl_slist_free_all(headerlist);
        curl_easy_cleanup(curl);
        cout << result << endl;
        tiny::TinyJson json;
        json.ReadJson(result);
        if (json.Get<string>("message") == "success")
        {
            return true;
        }
        
    }
    return false;
}