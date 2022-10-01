﻿#include <iostream>
#include <string>
#include <winsock2.h>
#include <WS2tcpip.h> //definition of function inet_pton()
#include <thread>
#pragma comment (lib,"ws2_32.lib")
using namespace std;

inline void showMenu1() {
	cout
		<< "Please select a fuction: " << endl
		<< "Input 1 : Connect to server" << endl
		<< "Input 2 : Quit" << endl;
}

inline void showMenu2() {
	cout<<endl
		<< "Please select a function: " << endl
		<< "Input 1 : Get server time" << endl
		<< "Input 2 : Get server name" << endl
		<< "Input 3 : Get list of active clients" << endl
		<< "Input 4 : Send message to another client" << endl
		<< "Input 5 : Disconnect and exit" << endl;
}

void rcv(SOCKET& sock) {
	while (1) {
		char buf[MAXBYTE] = { 0 };
		recv(sock, buf, MAXBYTE, 0);
		string msg(buf);
		if (buf[0] == '1') cout << "The server time is: " << msg.substr(1) << endl;
		else if (buf[0] == '2') cout << "The server name is: " << msg.substr(1) << endl;
		else if (buf[0] == '3') cout << "The list of active clients is: " << endl << msg.substr(1) << endl;
		else if (buf[0] == '4') cout << msg.substr(1) << endl;
		else if (buf[0] == '5') break;
	}
}

int main()
{
	//Initialize DLL
	WSADATA wsaData;
	if (!(WSAStartup(MAKEWORD(2, 2), &wsaData) == 0)) {
		cerr << "WSAStartup failed. The program will exit with code -1." << endl;
		return -1;
	}

	int state;
	while (1) {
		showMenu1();
		cin >> state;
		if (state == 1) break;
		else if (state == 2) return 0;
		else cout << "Invalid input. Please input an number again." << endl;
	}

	//Create socket
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	//Request server
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	string serverIP;
	u_short portNumber;
	cout << "Please input server IP:" << endl;
	cin >> serverIP;
	cout << "Please input port number:" << endl;
	cin >> portNumber;
	inet_pton(AF_INET, serverIP.c_str(), (void*)&sockAddr.sin_addr);
	sockAddr.sin_port = htons(portNumber);
	if (!( connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR)) == 0 )) {
		cerr << "Failed to connect to server. The process will exit with code -1.";
		return -1;
	};
	cout << endl << "Successfully connected to the server." << endl;
	cout << endl;
	
	//Get message from server.
	thread getMsg(rcv, ref(sock));

	while (1) {
		showMenu2();
		cin >> state;
		if (state == 1) {
			//Get server time
			string msg = "1";
			send(sock, msg.c_str(), msg.length() + sizeof(char), 0);
		}
		else if (state == 2) {
			//Get server name
			string msg = "2";
			send(sock, msg.c_str(), msg.length() + sizeof(char), 0);
		}
		else if (state == 3) {
			//Get list of active clients
			string msg = "3";
			send(sock, msg.c_str(), msg.length() + sizeof(char), 0);
			
		}
		else if (state == 4) {
			//Send message to another client
			string msg = "4";
			cout << "Please input number of target client: " << endl;
			string target;
			cin >> target;
			msg += target+" ";
			cout << "Please input your message and end with an empty line." << endl;
			string temp;
			getline(cin, temp); //Get the rest '\n'
			while (1) {
				getline(cin, temp);
				if (temp.empty()) break;
				else msg += temp + '\n';
			}
			send(sock, msg.c_str(), msg.length() + sizeof(char), 0);
		}
		else if (state == 5) {
			//Disconnect and exit
			string msg = "5";
			send(sock, msg.c_str(), msg.length() + sizeof(char), 0);
			break;
		}
		else cout << "Invalid input. Please input an number again." << endl;
		Sleep(100);
	}
	getMsg.join();
	//Close socket
	closesocket(sock);

	WSACleanup();
	
}
