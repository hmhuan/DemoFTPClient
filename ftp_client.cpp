#include "lib.h"

int main(int argc, char* argv[])
{
	WORD wVersionRequested;
	WSADATA wsaData;
	SOCKET socket_descriptor = -1;
	HOSTENT *pHostEnt;
	char ServerName[64];
	char Buffer[4096];
	int retcode;
	bool pasvMode = false;
	bool logined = false;

	SetConsoleOutputCP(65001);

	wVersionRequested = MAKEWORD(2, 2);
	retcode = WSAStartup(wVersionRequested, &wsaData);
	if (retcode != 0)
		errexit("Startup failed: %d\n", retcode);

	if (LOBYTE(wsaData.wVersion) != LOBYTE(wVersionRequested) || HIBYTE(wsaData.wVersion) != HIBYTE(wVersionRequested))
	{
		printf("Supported version is too low\n");
		WSACleanup();
		return 0;
	}

	if (argc > 1) {
		strcpy(ServerName, argv[1]);
		connectSocket(socket_descriptor, pHostEnt, ServerName);
		logined = LoginFTP(socket_descriptor, pHostEnt);
	}
	
	//set timeout cho recv()
	int timeout = 50; //in milliseconds. this is 30 seconds

	string temp, cmd;
	char buf[BUFSIZ];
	int codeftp;

	do {
		timeout = 50;
		setsockopt(socket_descriptor, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));

		printf("ftp>");
		rewind(stdin);
		getline(cin, cmd);
		//printf("%s\n", cmd.c_str());
		//check 421 timed out connection
		memset(buf, 0, sizeof(buf));
		recv(socket_descriptor, buf, sizeof buf, 0);
		sscanf(buf, "%d", &codeftp);
		if (codeftp == 421)
		{
			replylogcode(codeftp);
			break;
		}

		chuanHoa(cmd);
		temp = cmd;
		//UPPERCASE temp command
		for (auto & c : temp) c = toupper(c);
		if (socket_descriptor == -1) {
			if (temp.substr(0, 4) == "QUIT" || temp.substr(0, 4) == "EXIT")
			{
				break;
			}
			else if (temp.substr(0, 4) == "OPEN")
			{
				cmd.erase(0, 4);
				chuanHoa(cmd);
				if (cmd == "") {
					printf("To ");
					getline(cin, cmd);
				}
				chuanHoa(cmd);
				connectSocket(socket_descriptor, pHostEnt, (char*)cmd.c_str());
				logined = LoginFTP(socket_descriptor, pHostEnt);
			}
			else if (temp.substr(0, 4) == "HELP")
			{
				cout << "Commands may be abbreviated.  Commands are:\n\n";
				cout << "\tls\t\tdir\t\tcd\t\tlcd\n";
				cout << "\tput\t\tget\t\tmput\t\tmget\n";
				cout << "\tdelete\t\tmdelete\t\tmkdir\t\trmdir\n";
				cout << "\tpwd\t\tpasv\t\tactv\t\thelp\n";
				cout << "\topen\t\tdisconnect\tlogin\t\tlogout\n";
				cout << "\tquit\t\texit\n";
			}
			else {
				printf("Not connected.\n");
			}
		}
		else {
			if (temp.substr(0, 4) == "QUIT" || temp.substr(0, 4) == "EXIT")
			{
				quit(socket_descriptor);
				disconnectSocket(socket_descriptor);
				break;
			}
			else if (temp.substr(0, 5) == "LOGIN")
			{
				if (logined) {
					cout << "Please logout firstly.\n";
				}
				else {
					logined = LoginFTP(socket_descriptor, pHostEnt);
				}
				
			}
			else if (temp.substr(0, 6) == "LOGOUT")
			{
				logined = false;
			}
			else if (temp.substr(0, 2) == "LS")
			{
				ls(socket_descriptor, cmd, pasvMode);
			}
			else if (temp.substr(0, 3) == "DIR")
			{
				dir(socket_descriptor, cmd, pasvMode);
			}
			else if (temp.substr(0, 3) == "PWD")
			{
				pwd(socket_descriptor);
			}
			else if (temp.substr(0, 6) == "DELETE")
			{
				cmd.erase(0, 6);
				Delete(socket_descriptor, cmd);
			}
			else if (temp.substr(0, 7) == "MDELETE")
			{
				mDelete(socket_descriptor, cmd, pasvMode);
			}
			else if (temp.substr(0, 5) == "RMDIR")
			{
				rmdir(socket_descriptor, cmd);
			}
			else if (temp.substr(0, 5) == "MKDIR")
			{
				mkdir(socket_descriptor, cmd);
			}
			else if (temp.substr(0, 3) == "GET")
			{
				cmd.erase(0, 3);
				chuanHoa(cmd);
				get(socket_descriptor, cmd, pasvMode);
			}
			else if (temp.substr(0, 4) == "MGET")
			{
				cmd.erase(0, 4);
				chuanHoa(cmd);
				mget(socket_descriptor, cmd, pasvMode);
			}
			else if (temp.substr(0, 3) == "PUT")
			{
				cmd.erase(0, 3);
				chuanHoa(cmd);
				put(socket_descriptor, cmd, pasvMode);
			}
			else if (temp.substr(0, 4) == "MPUT")
			{
				cmd.erase(0, 4);
				chuanHoa(cmd);
				mput(socket_descriptor, cmd, pasvMode);
			}
			else if (temp.substr(0, 2) == "CD")
			{
				cd(socket_descriptor, cmd);
			}
			else if (temp.substr(0, 3) == "LCD")
			{
				lcd((char*)cmd.c_str());
			}
			else if (temp.substr(0, 4) == "PASV")
			{
				if (!pasvMode)
					printf("Passive Mode On\n");
				pasvMode = true;
			}
			else if (temp.substr(0, 4) == "ACTV")
			{
				if (pasvMode)
					printf("Active Mode On\n");
				pasvMode = false;
			}
			else if (temp.substr(0, 4) == "HELP")
			{
				cout << "Commands may be abbreviated.  Commands are:\n\n";
				cout << "\tls\t\tdir\t\tcd\t\tlcd\n";
				cout << "\tput\t\tget\t\tmput\t\tmget\n";
				cout << "\tdelete\t\tmdelete\t\tmkdir\t\trmdir\n";
				cout << "\tpwd\t\tpasv\t\tactv\t\thelp\n";
				cout << "\topen\t\tdisconnect\tlogin\t\tlogout\n";
				cout << "\tquit\t\texit\n";
			}
			else if (temp.substr(0, 10) == "DISCONNECT")
			{
				quit(socket_descriptor);
				disconnectSocket(socket_descriptor);
				//logined = false;
				socket_descriptor = -1;
			}
			else
				cout << "Invalid command.\n";
		}		
	} while (1);

	retcode = WSACleanup();
	if (retcode == SOCKET_ERROR)
		errexit("Cleanup failed: %d\n", WSAGetLastError());
	return 0;
}