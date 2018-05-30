#include "lib.h"

int main(int argc, char* argv[])
{
	WORD wVersionRequested;
	WSADATA wsaData;
	SOCKET socket_descriptor;
	HOSTENT *pHostEnt;
	struct sockaddr_in sin;

	char ServerName[64];
	char Buffer[4096];
	int retcode;
	bool pasvMode = false;

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

	socket_descriptor = socket(PF_INET, SOCK_STREAM, 0);
	if (socket_descriptor == INVALID_SOCKET)
		errexit("Socket creation failed: %d\n", WSAGetLastError());

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(21);

	if (argc > 1) strcpy(ServerName, argv[1]);
		else strcpy(ServerName, "127.0.0.1");

	if (pHostEnt = gethostbyname(ServerName)) 
	{
		memcpy(&sin.sin_addr, pHostEnt->h_addr_list[0], pHostEnt->h_length);
	}
	else errexit("Can't get %s\" host entry: %d\n", ServerName, WSAGetLastError());

	retcode = connect(socket_descriptor, (struct sockaddr *) &sin, sizeof(sin));
	if (retcode == SOCKET_ERROR)
		errexit("Connect failed: %d\n", WSAGetLastError());

	char buf[BUFSIZ];
	int tmpres, status;
	char * str;
	int codeftp;

	printf("Connection established, waiting for welcome message...\n");
	//How to know the end of welcome message:http://stackoverflow.com/questions/13082538/how-to-know-the-end-of-ftp-welcome-message
	memset(buf, 0, sizeof buf);
	while ((tmpres = recv(socket_descriptor, buf, BUFSIZ, 0)) > 0) 
	{
		sscanf(buf, "%d", &codeftp);
		printf("%s", buf);
		if (codeftp != 220) //120, 240, 421: something wrong
		{
			replylogcode(codeftp);
			exit(1);
		}
		str = strstr(buf, "220");//Why ???
		if (str != NULL) {
			break;
		}
		memset(buf, 0, tmpres);
	}
	
	LoginFTP(socket_descriptor, pHostEnt);
	//set timeout cho recv()
	int timeout = 50; //in milliseconds. this is 30 seconds

	wstring cmd1;
	string temp, cmd;

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
		
		if (temp.substr(0,4) == "QUIT" || temp.substr(0,4) == "EXIT")
		{
			quit(socket_descriptor);
			break;
		}
		else if (temp.substr(0,2) == "LS")
		{	
			ls(socket_descriptor, cmd, pasvMode);
		}
		else if (temp.substr(0,3) == "DIR")
		{
			dir(socket_descriptor, cmd, pasvMode);
		}
		else if (temp.substr(0,3) == "PWD")
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
		else if (temp.substr(0,3) == "GET")
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
		else if (temp.substr(0,3) == "PUT")
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
		else if (temp.substr(0,2) == "CD")
		{
			cd(socket_descriptor, cmd);
		}
		else if (temp.substr(0, 3) == "LCD")
		{
			//lcd(cmd);
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
			cout << "\tpwd\t\tpasv\t\tactv\t\theplp\n";
			cout << "\t!\t\tquit\t\texit\n";
		}
		else
			cout << "Invalid command.\n";
	} while (1);
	
	retcode = closesocket(socket_descriptor);
	if (retcode == SOCKET_ERROR)
		errexit("Close socket failed: %d\n", WSAGetLastError());

	retcode = WSACleanup();
	if (retcode == SOCKET_ERROR)
		errexit("Cleanup failed: %d\n", WSAGetLastError());
	return 0;
}