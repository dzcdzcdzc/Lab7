﻿#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>
#include <WS2tcpip.h> //definition of function inet_pton() 
#include <Windows.h>
#include <thread>
#pragma comment (lib,"ws2_32.lib")
using namespace std;

string getTime() {
    time_t timep;
    time(&timep);
    char tmp[64];
    struct tm nowTime;
    localtime_s(&nowTime, &timep);
    strftime(tmp, sizeof(tmp), "%Y-%m-%d-%H:%M:%S", &nowTime);
    return string(tmp);
}

void serve(SOCKET* clntSock,string* addr,vector<pair<SOCKET*, SOCKADDR*>>& activeClients) {
    cout << "Received a request." << endl;

    //Answer requests
    char EOM = -1;
    bool ctn = true;
    string temp;
    while (ctn) {
        char szBuffer[MAXBYTE] = { 0 };
        recv(*clntSock, szBuffer, MAXBYTE, 0);
        string msg(szBuffer);
        for(char c : msg){
            if (c == EOM) {
                if (temp[0] == '1') {
                    string head = "The server time is: ";
                    string nowTime = head + getTime() + EOM;
                    send(*clntSock, nowTime.c_str(), nowTime.length() + sizeof(char), 0);
                }
                else if (temp[0] == '2') {
                    WCHAR computerName[256];
                    u_long size = 256;
                    GetComputerName(computerName, &size);

                    int pSize = WideCharToMultiByte(CP_OEMCP, 0, computerName, wcslen(computerName), NULL, 0, NULL, NULL);
                    char* pCStrKey = new char[pSize + 1];
                    WideCharToMultiByte(CP_OEMCP, 0, computerName, wcslen(computerName), pCStrKey, pSize, NULL, NULL);
                    pCStrKey[pSize] = '\0';

                    string res = "The server name is: " + string(pCStrKey) + EOM;
                    send(*clntSock, res.c_str(), res.length() + sizeof(char), 0);
                }
                else if (temp[0] == '3') {
                    string msg = "The list of active clients is:\nSeq\tAddress\t\tPort\n";
                    for (int i = 0; i < activeClients.size(); i++) {
                        msg += '0' + i;
                        msg += "\t";
                        IN_ADDR ip = ((SOCKADDR_IN*)activeClients[i].second)->sin_addr;
                        USHORT port = ((SOCKADDR_IN*)activeClients[i].second)->sin_port;
                        char ipdotdec[20];
                        inet_ntop(AF_INET, (void*)&ip, ipdotdec, sizeof(ipdotdec));
                        msg += ipdotdec;
                        msg += "\t";
                        msg += to_string(ntohs(port));
                        msg += "\n";
                        msg += EOM;
                    }
                    send(*clntSock, msg.c_str(), msg.length() + sizeof(char), 0);
                }
                else if (temp[0] == '4') {
                    string rcvmsg(szBuffer);
                    string targetstr;
                    int i = 1;
                    for (; rcvmsg[i] != ' '; i++) targetstr += rcvmsg[i];
                    string sdmsg = "\nMessage from " + *addr + ":\n" + rcvmsg.substr(i + 1);
                    int target = stoi(targetstr);

                    string errmsg = "Send massage failed : unvalid number.";
                    string seccessmsg = "Message sent successfully." ;
                    errmsg += EOM;
                    seccessmsg += EOM;
                    sdmsg += EOM;
                    if (target >= activeClients.size()) send(*clntSock, errmsg.c_str(), errmsg.length() + sizeof(char), 0);
                    else {
                        send(*activeClients[target].first, sdmsg.c_str(), sdmsg.length() + sizeof(char), 0);
                        send(*clntSock, seccessmsg.c_str(), seccessmsg.length() + sizeof(char), 0);
                    }
                }
                else if (temp[0] == '5') {
                    cout << "A client disconnected." << endl;
                    string msg = "5";
                    msg += EOM;
                    send(*clntSock, msg.c_str(), msg.length() + sizeof(char), 0);
                    ctn = false;
                    break;
                }
                temp.clear();
            }
            else temp += c;
        }
        
    }
    //Disconnect
    for (auto i = activeClients.begin(); i != activeClients.end(); i++) {
        if (i->first == clntSock) {
            activeClients.erase(i);
            break;
        }
    }
    closesocket(*clntSock);
}

int main()
{
    //Initialize DLL
    WSADATA wsaData;
    if (!(WSAStartup(MAKEWORD(2, 2), &wsaData) == 0)) {
        cerr << "WSAStartup failed. The process will exit with code -1." << endl;
        return -1;
    }
    //Create socket
    SOCKET servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    //Bind IP address and port number to the socket
    sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = PF_INET;
    string serverIP = "127.0.0.1";
    u_short portNumber = 4203;
    inet_pton(AF_INET, serverIP.c_str(), (void*)&sockAddr.sin_addr); //Server IP address
    sockAddr.sin_port = htons(portNumber);
    bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
    
    //Start listening from network.
    listen(servSock, 20);

    vector<pair<SOCKET*,SOCKADDR*>> activeClients; //Store activeClients
    vector<thread*> requests;
    
   //Create client socket
    while (1) {
        SOCKADDR* clntAddr = new SOCKADDR();
        int nSize = sizeof(SOCKADDR);
        SOCKET* clntSock = new SOCKET();
        *clntSock = accept(servSock, (SOCKADDR*)clntAddr, &nSize);
        activeClients.push_back({ clntSock,(SOCKADDR*)clntAddr });
        IN_ADDR ip = ((SOCKADDR_IN*)clntAddr)->sin_addr;
        USHORT port = ((SOCKADDR_IN*)clntAddr)->sin_port;
        char ipdotdec[20];
        inet_ntop(AF_INET, (void*)&ip, ipdotdec, sizeof(ipdotdec));
        string* addr = new string();
        *addr = "[ " + string(ipdotdec) + ":" + to_string(ntohs(port)) + " ]";
        thread* thrd = new thread(serve, clntSock, addr, ref(activeClients));
        requests.push_back(thrd);
    }
    //Close server socket
    closesocket(servSock);

    //Cleanup DLL
    WSACleanup();
}

