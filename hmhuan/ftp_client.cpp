#include "lib.h"

int main(int argc, char* argv[])
{
	WORD wVersionRequested;
	WSADATA wsaData;
	SOCKET socket_descriptor;
	HOSTENT *pHostEnt;
	struct sockaddr_in sin, data;

	char ServerName[64];
	char Buffer[4096];
	int retcode;

	/*
	[1] WSAStartup() must be called before any other socket
	routines. The following code prints all returned
	information valid in WinSock 2.

	WSAStartup returns the following information in the
	WSAData structure.

	struct WSAData 
	{
		WORD            wVersion;
		WORD            wHighVersion;
		char            szDescription[WSADESCRIPTION_LEN+1];
		char            szSystemStatus[WSASYSSTATUS_LEN+1];
		unsigned short  iMaxSockets; (ignore in WinSock 2)
		unsigned short  iMaxUdpDg;   (ignore in WinSock 2)
		char FAR *      lpVendorInfo;(ignore in WinSock 2)
	};

	Note that WSAGetLastError() should not be used since the
	error code may not be saved properly if winsock.dll did
	not load.
	*/
	
	//printf("WSAStartup()\n"); // not need

	wVersionRequested = MAKEWORD(2, 2);
	retcode = WSAStartup(wVersionRequested, &wsaData); 
	if (retcode != 0)
		errexit("Startup failed: %d\n", retcode);

	/*printf("Return Code: %i\n", retcode);
	printf("Version Used: %i.%i\n", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
	printf("Version Supported:  %i.%i\n", LOBYTE(wsaData.wHighVersion), HIBYTE(wsaData.wHighVersion));
	printf("Implementation: %s\n", wsaData.szDescription);
	printf("System Status: %s\n", wsaData.szSystemStatus);
	printf("\n");*/
	
	if (LOBYTE(wsaData.wVersion) != LOBYTE(wVersionRequested) || HIBYTE(wsaData.wVersion) != HIBYTE(wVersionRequested))
	{
		printf("Supported version is too low\n");
		WSACleanup();
		return 0;
	}

	/*
	[2] Once WSAStartup has been called, the socket can be
	created using the socket() call. The following creates an
	Internet protocol family (PF_INET) socket providing stream
	service (SOCK_STREAM).
	*/

	//printf("socket()\n");
	socket_descriptor = socket(PF_INET, SOCK_STREAM, 0);
	if (socket_descriptor == INVALID_SOCKET)
		errexit("Socket creation failed: %d\n", WSAGetLastError());
	//printf("Socket Descriptor: %i\n", socket_descriptor);
	//printf("\n");

	/*
	[3] Before making a connection, an Internet address
	family structure must be initialized.

	struct sockaddr_in 
	{
		short sin_family;
		u_short sin_port;
		struct in_addr sin_addr;
		char sin_zero[8];
	};

	It is first set to all zeros.  The family is AF_INET.
	We'll use 4984 as the port number for this example.
	htons() converts to network byte ordering.

	gethostbyname() is used to determine the address of the
	remote host.  For this example, we connect to the host
	specified in the first command line argument or the local
	host ("localhost") if no command line arguments are
	provided, using the first address for this host in the
	h_addr_list.

	gethostbyname() returns a pointer to the following
	structure.  Some of the contents are printed below.

	struct hostent 
{
		char FAR * h_name;
		char FAR * FAR * h_aliases;
		short h_addrtype;
		short h_length;
		char FAR * FAR * h_addr_list;
	};
	*/

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(21);

	memset(&data, 0, sizeof data);
	data.sin_family = AF_INET;
	
	//neu truyen vao tham so thi argv[1] la ServerName, nguoc lai mac dinh "localhost"
	if (argc > 1) strcpy(ServerName, argv[1]);
		else strcpy(ServerName, "127.0.0.1");
	//printf("gethostbyname(\"%s\")\n", ServerName);
	if (pHostEnt = gethostbyname(ServerName)) 
	{
		memcpy(&sin.sin_addr, pHostEnt->h_addr_list[0], pHostEnt->h_length);
		/*printf("Address Length: %d\n", pHostEnt->h_length);
		printf("Host Address: %s\n", inet_ntoa(sin.sin_addr));
		printf("Host Name: %s\n", pHostEnt->h_name);
		printf("\n");*/
	}
	else errexit("Can't get %s\" host entry: %d\n", ServerName, WSAGetLastError());
	/*
	[4] connect() is used to establish a connection to a remote
	endpoint.
	*/

	//printf("connect()\n");
	retcode = connect(socket_descriptor, (struct sockaddr *) &sin, sizeof(sin));
	if (retcode == SOCKET_ERROR)
		errexit("Connect failed: %d\n", WSAGetLastError());
	//printf("Return Code: %d\n", retcode);
	//printf("\n");

	/*
	[5] send() is used to send a message to the server.  No
	flags are set.


	printf("send()\n");

	retcode = send(socket_descriptor, Message, sizeof Message, 0);
	if(retcode == SOCKET_ERROR)
	errexit("Send failed: %d\n", WSAGetLastError());

	printf("Bytes Sent: %d\n", retcode);
	printf("\n");
	pause();
	*/

	char buf[BUFSIZ];
	int tmpres, status;
	/*
	Connection Establishment
	120
	220
	220
	421
	Login
	USER
	230
	530
	500, 501, 421
	331, 332
	PASS
	230
	202
	530
	500, 501, 503, 421
	332
	*/
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

	//lấy địa chỉ IP, PORT của cln để tạo cho socketData sau này (***)	
	int len = sizeof(data);
	getsockname(socket_descriptor, (struct sockaddr *)&data, &len);
	
	LoginFTP(socket_descriptor, pHostEnt);

	//set timeout cho recv()
	int timeout = 50; //in milliseconds. this is 30 seconds
	

	//tu code
	string cmd,temp;
	do {	
		setsockopt(socket_descriptor, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
		cout << "ftp>";
		rewind(stdin);
		getline(cin, cmd);

		memset(buf, 0, sizeof(buf));
		recv(socket_descriptor, buf, sizeof buf, 0);
		sscanf(buf, "%d", &codeftp);
		if (codeftp == 421)
		{
			replylogcode(codeftp);
			break;
		}

		chuanHoa(cmd);
		if (cmd.find(" ") != -1)
			temp = cmd.substr(0, cmd.find(" "));
		else
			temp = cmd;	
		//UPPERCASE CMD command
		for (auto & c : temp) c = toupper(c);

		if (temp == "QUIT" || temp == "EXIT")
		{
			quit(socket_descriptor);
			break;
		}
		else if (temp == "LS")
		{	
			ls(socket_descriptor, data, cmd);
		}
		else if (temp == "DIR")
		{
			dir(socket_descriptor, data, cmd);
		}
		else if (temp == "PWD")
		{
			pwd(socket_descriptor);
		}
		else if (temp == "DELETE")
		{
			cmd.erase(0, 6);
			Delete(socket_descriptor, cmd);
		}
		else if (temp == "MDELETE")
		{
			mDelete(socket_descriptor, data, cmd);
		}
		else if (temp == "RMDIR")
		{
			rmdir(socket_descriptor, cmd);
		}
		else if (temp == "MKDIR")
		{
			mkdir(socket_descriptor, cmd);
		}
		else if (temp.substr(0,3) == "GET")
		{
			cmd.erase(0, 3);
			get(socket_descriptor, data, cmd);
		}
		else if (temp.substr(0, 4) == "MGET")
		{
			mget(socket_descriptor, data, cmd);
		}
		else if (temp.substr(0,3) == "PUT")
		{
			cmd.erase(0, 3);
			put(socket_descriptor, data, cmd);
		}
		else if (temp.substr(0, 4) == "MPUT")
		{
			mput(socket_descriptor, data, cmd);
		}
		else if (temp == "CD")
		{
			cd(socket_descriptor, cmd);
		}
		else if (temp.substr(0, 3) == "LCD")
		{
			lcd(cmd);
		}
		else if (temp.substr(0, 4) == "HELP")
		{
			cout << "Commands may be abbreviated.  Commands are:\n\n";
			cout << "\tls\t\tdir\t\tcd\t\tlcd\n";
			cout << "\tput\t\tget\t\tmput\t\tmget\n";
			cout << "\tdelete\t\tmdelete\t\tmkdir\t\trmdir\n";
			cout << "\tpwd\t\tpasv\t\tquit\t\texit\n";
			cout << "\t!\t\thelp\n";
		}
		else
			cout << "Invalid command.\n";
	} while (1);
	

	/*
	[7] recv() is used to receive a message from the server.
	No flags are set.  This code assumes that all data is
	read with the first and only recv() call.  In general,
	this is not a good assumption since a stream transport
	service is used.


	printf("recv()\n");

	length = recv(socket_descriptor, Buffer, sizeof Buffer, 0);
	if(length == SOCKET_ERROR)
	errexit("Receive failed: %d\n", WSAGetLastError());

	printf("Bytes received: %d\n", length);
	printf("Message: %s\n", Buffer);
	printf("\n");
	pause();
	*/

	/*
	[8] The client closes its socket using closesocket();
	*/

	//printf("closesocket()\n");
	retcode = closesocket(socket_descriptor);
	if (retcode == SOCKET_ERROR)
		errexit("Close socket failed: %d\n", WSAGetLastError());
	//printf("Return Code: %d\n", retcode);
	//printf("\n");

	/*
	[9] WSACleanup() is used to terminate use of socket services.
	*/

	//printf("WSACleanup()\n");
	retcode = WSACleanup();
	if (retcode == SOCKET_ERROR)
		errexit("Cleanup failed: %d\n", WSAGetLastError());
	//printf("Return Code: %d\n", retcode);
	//printf("\n");
	return 0;
}