#include "lib.h"

void errexit(const char *format, ...)
{
	va_list	args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	WSACleanup();
	exit(1);
}
void pause(void)
{
	char c;
	printf("Press Enter to continue\n");
	scanf("%c", &c);
}
void replylogcode(int code)
{
	switch (code) {
	case 200:
		printf("Command okay");
		break;
	case 500:
		printf("Syntax error, command unrecognized.");
		printf("This may include errors such as command line too long.");
		break;
	case 501:
		printf("Syntax error in parameters or arguments.");
		break;
	case 202:
		printf("Command not implemented, superfluous at this site.");
		break;
	case 421:
		printf("Connection timed out.");
		break;
	case 502:
		printf("Command not implemented.");
		break;
	case 503:
		printf("Bad sequence of commands.");
		break;
	case 530:
		printf("Not logged in.");
		break;
	}
	printf("\n");
}

void chuanHoa(string &s)
{
	while (s[0] == ' ' || s[0] == '\t' || s[0] == '\n')
		s.erase(0, 1);
}

string getFileName(string &cmd)
{
	char buf[1024];
	string filename = "";
	chuanHoa(cmd);
	memset(buf, 0, sizeof buf);
	sscanf(cmd.c_str(), "%s", buf);
	cmd.erase(0, strlen(buf));
	filename = buf;
	if (buf[0] == '\"')
	{
		filename.erase(0, 1);
		while (filename[filename.length() - 1] != '\"' && cmd != "") {
			chuanHoa(cmd);
			memset(buf, 0, sizeof buf);
			sscanf(cmd.c_str(), "%s", buf);
			filename += ' ';
			filename += buf;
			cmd.erase(0, strlen(buf));
		}
		filename.erase(filename.length() - 1);
	}
	return filename;
}

void LoginFTP(SOCKET socket_descriptor, HOSTENT * pHostEnt)
{
	int codeftp = 421;
	char buf[BUFSIZ];
	//Send Username
	char info[50];

	//thiet lap man hinh nhap/xuat console cho password
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode = 0;
	GetConsoleMode(hStdin, &mode);

	printf("User (%s): ", pHostEnt->h_name);
	memset(buf, 0, sizeof buf);
	scanf("%s", info);
	sprintf(buf, "USER %s\r\n", info);
	send(socket_descriptor, buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	recv(socket_descriptor, buf, BUFSIZ, 0);

	sscanf(buf, "%d", &codeftp);
	if (codeftp != 331)
	{
		replylogcode(codeftp);
		exit(1);
	}
	printf("%s", buf);

	//Send Password
	SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
	memset(info, 0, sizeof info);
	printf("Password: ");
	memset(buf, 0, sizeof buf);
	scanf("%s", info);
	SetConsoleMode(hStdin, mode);
	std::cout << "\n";
	sprintf(buf, "PASS %s\r\n", info);
	send(socket_descriptor, buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	recv(socket_descriptor, buf, BUFSIZ, 0);
	//nhan codeftp 
	sscanf(buf, "%d", &codeftp);
	if (codeftp != 230)
	{
		replylogcode(codeftp);
		exit(1);
	}
	printf("%s", buf);
}

bool passiveMode(SOCKET soc, SOCKET &dsoc) {
	char buf[BUFSIZ];
	int tmpres;
	unsigned int port = 0; // Port server opened for client connect into
	string addr = ""; // Address of server ftp
	sockaddr_in sin_data;
	hostent *pHost;

	// Config cmd to sen PASV to server
	memset(buf, 0, sizeof buf);
	sprintf(buf, "PASV\r\n");
	// Send request to connect by passive mode
	tmpres = send(soc, buf, strlen(buf), 0);
	if (tmpres > -1) 
	{
		// Receive information about address and port server sent down
		memset(buf, 0, sizeof buf);
		recv(soc, buf, BUFSIZ, 0);

		//kiem tra co phai ma loi 227
		sscanf(buf, "%d", &tmpres);
		// get address and port of server
		if (tmpres == 227)
		{
			//tien hanh boc tach de lay port
			string receive(buf);
			int pos1 = receive.find('(');
			int pos2 = receive.find(')');
			string c1 = receive.substr(pos1 + 1, pos2 - pos1 - 1);

			for (int i = 1; i <= 4; i++) 
			{
				pos1 = c1.find(',');
				if (addr == "") {
					addr += c1.substr(0, pos1);
				}
				else {
					addr += "." + c1.substr(0, pos1);
				}
				c1 = c1.substr(pos1 + 1, c1.length() - pos1 - 1);
			}

			pos1 = c1.find(',');
			int h1 = atoi(c1.substr(0, pos1).c_str());
			int h2 = atoi(c1.substr(pos1 + 1, c1.length() - pos1 - 1).c_str());
			port = h1 * 256 + h2;

			// Config sin_data
			memset(&sin_data, 0, sizeof(sin_data));
			sin_data.sin_family = AF_INET;
			sin_data.sin_port = htons(port);

			if (pHost = gethostbyname(addr.c_str())) {
				memcpy(&sin_data.sin_addr, pHost->h_addr_list[0], pHost->h_length);
			}

			// Open socket
			dsoc = socket(PF_INET, SOCK_STREAM, 0);
			if (dsoc == INVALID_SOCKET)
				errexit("Socket creation failed: %d\n", WSAGetLastError());

			// connect to server
			int retcode;
			
			retcode = connect(dsoc, (struct sockaddr *) &sin_data, sizeof(sin_data));
			
			if (retcode == SOCKET_ERROR)
				errexit("Connect failed: %d\n", WSAGetLastError());
		}
		else 
		{
			return false;
		}
	}
	else 
	{
		return false;
	}
	return true;
}

bool activeMode(SOCKET soc, SOCKET &dsoc, int stat)
{
	char buf[BUFSIZ + 1];
	int tmpres;
	static int port_add = 1;
	sockaddr_in dataAddr;
	
	int len = sizeof(dataAddr),p;

	// Open socket dsoc
	dsoc = socket(PF_INET, SOCK_STREAM, 0);
	if (dsoc == INVALID_SOCKET)
		errexit("Socket creation failed: %d\n", WSAGetLastError());
	// Config dataAddr
	memset(&dataAddr, 0, sizeof dataAddr);
	dataAddr.sin_family = AF_INET;

	if (getsockname(soc, (sockaddr*)&dataAddr, &len) > -1) {
		dataAddr.sin_port = dataAddr.sin_port + port_add;
		p = dataAddr.sin_port;
		port_add++;
	}
	else {
		return false;
	}
	dataAddr.sin_port = htons(dataAddr.sin_port);
	if (bind(dsoc, (sockaddr*)&dataAddr, len) > -1) 
	{
		if (listen(dsoc, SOMAXCONN) > -1) {
			// Prepare port for client to listen from server
			IP ip;
			int p1, p2;
			ip.getIP(dataAddr);
			p1 = p / 256;
			p2 = p % 256;

			// Send port to server
			sprintf(buf, "PORT %d,%d,%d,%d,%d,%d\r\n", ip.a, ip.b, ip.c, ip.d, p1, p2);
			tmpres = send(soc, buf, strlen(buf), 0);
			memset(buf, 0, sizeof buf);
			recv(soc, buf, BUFSIZ, 0);
			if (stat == 0)
				printf("%s", buf);
		}
		else 
		{
			return false;
		}
	}
	else 
	{
		return false;
	}
	return true;
}
//hàm ls
void ls(SOCKET soc, string cmd, bool modePasv)
{
	char buf[1024];
	int Bytes;
	bool check;
	SOCKET dataSocket;
	
	int timeout = 0;
	setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));

	cmd.erase(0, 2);
	chuanHoa(cmd);

	if (modePasv)
		check = passiveMode(soc, dataSocket);
	else
		check = activeMode(soc, dataSocket, 0);
	//lenh NLST
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "NLST %s\r\n", cmd.c_str());
	send(soc, buf, strlen(buf), 0);
	memset(buf, 0, sizeof buf);
	Bytes = recv(soc, buf, sizeof buf, 0);
	printf("%s", buf);
	sscanf(buf, "%d", &Bytes);
	
	if (check && Bytes != 550 && Bytes != 501)
	{
		if (!modePasv)
			dataSocket = accept(dataSocket, NULL, NULL);
		
		while (1)
		{
			memset(buf, 0, sizeof buf);
			Bytes = recv(dataSocket, buf, 1023, 0);
			if (Bytes == 0)
				break;
			printf("%s", buf);
			if (Bytes != 1023)
				break;
		}
		//connSocket trả về cmd phản hồi 
		memset(buf, 0, sizeof buf);
		Bytes = recv(soc, buf, sizeof buf, 0);
		printf("%s", buf);
	}
	closesocket(dataSocket);
}
//hàm dir
void dir(SOCKET soc, string cmd, bool modePasv)
{
	char buf[1024];
	int Bytes;
	bool check;
	SOCKET dataSocket;

	cmd.erase(0, 3);
	chuanHoa(cmd);
	int timeout = 0;
	setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));

	if (modePasv)
		check = passiveMode(soc, dataSocket);
	else
		check = activeMode(soc, dataSocket, 0);
	//lenh NLST
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "LIST %s\r\n", cmd.c_str());
	Bytes = send(soc, buf, strlen(buf), 0);
	memset(buf, 0, sizeof buf);
	Bytes = recv(soc, buf, sizeof buf, 0);
	printf("%s", buf);//	std::cout << buf;
	sscanf(buf, "%d", &Bytes);
	if (check && Bytes != 550)
	{
		if (!modePasv)
			dataSocket = accept(dataSocket, NULL, NULL);
		while (1)
		{
			memset(buf, 0, sizeof buf);
			Bytes = recv(dataSocket, buf, 1023, 0);
			if (Bytes == 0)
				break;
			printf("%s", buf);
			if (Bytes != 1023)
				break;
		}
		//connSocket trả về cmd phản hồi 
		memset(buf, 0, sizeof buf);
		Bytes = recv(soc, buf, sizeof buf, 0);
		printf("%s", buf);
	}
	closesocket(dataSocket);
}

void lcd(char direct[]) {
	static wchar_t rootPath[BUFSIZ] = L""; // Lưu lại path của thư mục gốc ban đầu
	wchar_t info[BUFSIZ];
	int check = 0;

	// Establish mode for print unicode text
	_setmode(_fileno(stdin), _O_U16TEXT);
	_setmode(_fileno(stdout), _O_U16TEXT);

	// Kiểm tra ban đầu nếu path gốc chưa lưu lại thì lưu lại
	if (wcscmp(rootPath, L"") == 0) {
		wmemset(rootPath, 0, sizeof rootPath);
		_wgetcwd(rootPath, sizeof(rootPath));
	}

	if (strcmp(direct, "lcd") == 0) {
		check = SetCurrentDirectoryW(rootPath);
	}
	else {
		//check = SetCurrentDirectory(direct + 4);
	}

	if (check) {
		GetCurrentDirectoryW(BUFSIZ, info);
		wcout << "Local directory now " << info << "." << endl;
	}
	else {
		wcout << "File not found" << endl;
	}
	// Establish mode for print ascii text
	_setmode(_fileno(stdin), _O_TEXT);
	_setmode(_fileno(stdout), _O_TEXT);
}
//Hàm cd
void cd(SOCKET soc, string cmd)
{
	char buf[1024];
	int Bytes;
	cmd = cmd.erase(0, 2);
	chuanHoa(cmd);
	if (cmd == "")
	{
		std::cout << "Remote directory: ";
		cin >> cmd;
	}
	memset(buf, 0, sizeof buf);
	sprintf(buf, "CWD %s\r\n", cmd.c_str());
	send(soc, buf, strlen(buf), 0);
	//connSocket trả về cmd 
	memset(buf, 0, sizeof buf);
	Bytes = recv(soc, buf, sizeof buf, 0);
	if (Bytes != -1)
		printf("%s", buf);
}
//Hàm pwd
void pwd(SOCKET soc)
{
	char buf[1024];

	memset(buf, 0, sizeof buf);
	sprintf(buf, "PWD\r\n");
	send(soc, buf, strlen(buf), 0);
	//connSocket trả về cmd
	memset(buf, 0, sizeof buf);
	recv(soc, buf, sizeof buf, 0);
	printf("%s", buf);
}
//hàm quit
void quit(SOCKET soc)
{
	char buf[1024];
	int Bytes;

	memset(buf, 0, sizeof buf);
	sprintf(buf, "QUIT\r\n");
	Bytes = send(soc, buf, strlen(buf), 0);
	//connSocket trả về cmd 
	memset(buf, 0, sizeof buf);
	Bytes = recv(soc, buf, sizeof buf, 0);
	printf("%s\n", buf);
}
//ha,f tải về 1 file
void get(SOCKET soc, string cmd, bool modePasv)
{
	char buf[1024]; //??2048 4096
	int Bytes;
	SOCKET dataSocket;
	ofstream fOut;
	string temp, rm, lc;

	int timeout = 0;
	setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));

	if (cmd == "")
	{
		std::cout << "Remote file: ";
		getline(cin, rm);
		std::cout << "Local file: ";
		getline(cin, lc); //ten file up len server
	}
	else
	{
		rm = getFileName(cmd);
		lc = getFileName(cmd);
	}
	if (lc == "")
	{
		for (int i = rm.length(); i >= 0; i--)
		{
			if (rm[i] == '\\' || rm[i] == '/')
				break;
			lc = rm[i] + lc;
		}
	}
	bool check;
	if (modePasv)
		check = passiveMode(soc, dataSocket);
	else
		check = activeMode(soc, dataSocket, 0);

	memset(buf, 0, sizeof buf);
	sprintf(buf, "RETR %s\r\n", rm.c_str());
	Bytes = send(soc, buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	Bytes = recv(soc, buf, sizeof buf, 0);
	printf("%s", buf);
	sscanf(buf, "%d", &Bytes);
	if (check && Bytes != 550 && Bytes != 501)
	{
		fOut.open(lc, ios::out | ios::binary);
		if (fOut.is_open())
		{
			if (!modePasv)
				dataSocket = accept(dataSocket, NULL, NULL);
			while (1)
			{
				memset(buf, 0, sizeof buf);
				Bytes = recv(dataSocket, buf, sizeof buf, 0);
				fOut.write(buf, Bytes);
				if (Bytes != sizeof buf)
					break;
			}
			fOut.close();
		}
		else
		{
			cout << "> R: No such process\n";
		}
		//connSocket trả về thông báo gửi thành công/ thất bại 
		memset(buf, 0, sizeof buf);
		Bytes = recv(soc, buf, sizeof buf, 0);
		printf("%s", buf);
	}
	closesocket(dataSocket);
}
//Hàm tải về nhiều file
void mget(SOCKET soc, string cmd, bool modePasv)
{
	char buf[1024], flnm[1024], c;
	bool check;
	string temp, filename;
	SOCKET dataSocket;
	int Bytes, k;

	int timeout = 0;
	setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
	
	//nếu cmd rỗng tiến hành đọc tên file
	if (cmd == "")
	{
		cout << "Remote files: ";
		rewind(stdin);
		getline(cin, cmd);
	}
	
	while (cmd != "")
	{
		chuanHoa(cmd);
		filename = getFileName(cmd);
		if (modePasv)
			check = passiveMode(soc, dataSocket);
		else
			check = activeMode(soc, dataSocket, 1);

		//lenh NLST
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "NLST %s\r\n", filename.c_str());
		send(soc, buf, strlen(buf), 0);
		memset(buf, 0, sizeof buf);
		recv(soc, buf, sizeof buf, 0);
		sscanf(buf, "%d", &k);
		temp = "";
		
		if (check && k != 550 && k != 501)
		{
			if (!modePasv)
				dataSocket = accept(dataSocket, NULL, NULL);

			while (1)
			{
				memset(buf, 0, sizeof buf);
				Bytes = recv(dataSocket, buf, sizeof buf, 0);
				temp += buf;
				if (Bytes != 1024)
					break;
			}
			//connSocket trả về cmd phản hồi 
			memset(buf, 0, sizeof buf);
			recv(soc, buf, sizeof buf, 0);	
		}
		else
		{
			cout <<"["<< filename << "]: ";
			printf("%s", buf);
		}
		closesocket(dataSocket);
		while (temp != "") 
		{
			memset(buf, 0, sizeof buf);
			sscanf(temp.c_str(), "%[^\r]", buf);
			printf("mget %s?", buf);
			filename = buf;
			if (filename.find(" ") != -1)
				filename = '\"' + filename + '\"';
			c = cin.get();
			if (c == 'y' || c == 'Y' || c == '\n')
				get(soc, filename, modePasv);
			rewind(stdin);
			temp.erase(0, strlen(buf) + 2); //xóa thêm 2 kí tự \r\n
			chuanHoa(temp);
		}
	}
}

void put(SOCKET soc, string cmd, bool modePasv)
{
	char buf[4096]; //co the tang buffer them??
	string temp, lc, rm;
	int Bytes;
	SOCKET dataSocket;
	ifstream fIn;

	int timeout = 0, flag;
	setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
	chuanHoa(cmd);
	////TH1: put <lcfile> [<rmfile>]
	if (cmd == "")
	{
		printf("Remote file: ");
		getline(cin, lc);
		printf("Local file: ");
		getline(cin, rm); //ten file up len server
	}
	else
	{
		lc = getFileName(cmd);
		rm = getFileName(cmd);
	}
	if (rm == "")
	{
		for (int i = lc.length(); i >= 0; i--)
		{
			if (lc[i] == '\\' || lc[i] == '/')
				break;
			rm = lc[i] + rm;
		}
	}

	fIn.open(lc.c_str(), ios::in | ios::binary);
	if (fIn.is_open())
	{
		bool check;
		if (modePasv)
			check = passiveMode(soc, dataSocket);
		else
			check = activeMode(soc, dataSocket, 0);

		memset(buf, 0, sizeof buf);
		sprintf(buf, "STOR %s\r\n", rm.c_str());
		Bytes = send(soc, buf, strlen(buf), 0);
		memset(buf, 0, sizeof buf);
		Bytes = recv(soc, buf, sizeof buf, 0);
		printf("%s", buf);

		sscanf(buf, "%d", &Bytes);
		if (check && Bytes != 550 && Bytes != 501)
		{
			if (!modePasv)
				dataSocket = accept(dataSocket, NULL, NULL);

			fIn.seekg(0, fIn.end);
			long long int Size = (fIn.tellg()) % (sizeof buf);
			fIn.seekg(0, fIn.beg);

			while (!fIn.eof())
			{
				memset(buf, 0, sizeof buf);
				fIn.read(buf, sizeof buf);
				if (fIn.eof())
				{
					send(dataSocket, buf, Size, 0);
					break;
				}
				send(dataSocket, buf, sizeof buf, 0);
			}
			closesocket(dataSocket);
			//connSocket trả về cmd 
			memset(buf, 0, sizeof buf);
			Bytes = recv(soc, buf, sizeof buf, 0);
			printf("%s", buf);
			fIn.close();
		}
	}
	else
	{
		printf("%s:", lc.c_str());
		cout << " File not found\n";
	}
}

void mput(SOCKET soc, string cmd, bool modePasv)
{
	char buf[1024], c;
	string filename,temp;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	//nếu cmd rỗng tiến hành đọc tên file
	if (cmd == "")
	{
		cout << "Local files: ";
		rewind(stdin);
		getline(cin, cmd);
	}

	while (cmd != "")
	{
		filename = getFileName(cmd);
		int t1 = filename.find_last_of('\\'), t2 = filename.find_last_of('/');

		TCHAR patter[MAX_PATH];

		memset(patter, 0x00, MAX_PATH);
		
		mbstowcs(patter, filename.c_str(), MAX_PATH);
		hFind = FindFirstFile(patter, &FindFileData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			printf("FindFirstFile failed (%d)\n", GetLastError());
			printf("[%s]: File not found\n", filename.c_str());
		}
		else
		{
			do
			{//ignore current and parent directories
				if (_tcscmp(FindFileData.cFileName, TEXT(".")) == 0 || _tcscmp(FindFileData.cFileName, TEXT("..")) == 0)
					continue;
				if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					//ignore directories
				}
				else
				{
					memset(buf, 0, sizeof buf);
					wcstombs(buf, FindFileData.cFileName, sizeof buf);
					temp = "";
		
					if (t1 != -1)
						temp = filename.substr(0, t1 + 1);
					else if (t2 != -1)
						temp = filename.substr(0, t2 + 1);
					temp += buf;
					temp = '\"' + temp + '\"';
					printf("mput %s?", buf);
					c = cin.get();
					if (c == 'y' || c == 'Y' || c == '\n')
						put(soc, temp, modePasv);
					rewind(stdout);
					rewind(stdin);
				}
			} while (FindNextFile(hFind, &FindFileData));
			FindClose(hFind);
		}
	}
}

//Hàm xóa 1 file
void Delete(SOCKET soc, string cmd)
{
	char buf[1024];
	int Bytes;

	int timeout = 0;
	setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
	chuanHoa(cmd);
	if (cmd == "")
	{
		std::cout << "Remote file: ";
		getline(cin, cmd);
	}

	memset(buf, 0, sizeof buf);
	sprintf(buf, "DELE %s\r\n",cmd.c_str());
	Bytes = send(soc, buf, strlen(buf), 0);
	//connSocket trả về cmd 
	memset(buf, 0, sizeof buf);
	Bytes = recv(soc, buf, sizeof buf, 0);
	printf("%s", buf);
}
//Hàm xóa nhiều file
void mDelete(SOCKET soc, string cmd, bool modePasv)
{
	char buf[1024], c;
	SOCKET dataSocket;
	int Bytes, k;
	string temp, filename;

	int timeout = 0;
	setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));

	cmd.erase(0, 7);//xoa mdelete
	chuanHoa(cmd);
	
	if (cmd == "") //doc file neu cmd chi co lenh mdelete
	{
		printf("Remote files: ");
		getline(cin, cmd);
	}
	int pos1, pos2;
	while (cmd != "")
	{
		filename = getFileName(cmd);
	//	pos1 = filename.find_last_of('\\'); pos2 = filename.find_last_of('/');
		bool check;
		if (modePasv)
			check = passiveMode(soc, dataSocket);
		else
			check = activeMode(soc, dataSocket, 1);
		//lenh NLST
		memset(buf, 0, sizeof buf);
		sprintf(buf, "NLST %s\r\n", filename.c_str());
		send(soc, buf, strlen(buf), 0);
		memset(buf, 0, sizeof buf);
		recv(soc, buf, sizeof buf, 0);
		printf("%s", buf);

		sscanf(buf, "%d", &k);
		if (check && k != 550 && k != 501)
		{
			temp = "";
			if (!modePasv)
				dataSocket = accept(dataSocket, NULL, NULL);
			while (1)
			{
				memset(buf, 0, sizeof buf);
				Bytes = recv(dataSocket, buf, sizeof buf, 0);
				temp += buf;
				if (Bytes != 1024)
					break;
			}
			//connSocket trả về cmd phản hồi 
			memset(buf, 0, sizeof buf);
			Bytes = recv(soc, buf, sizeof buf, 0);
		}
		closesocket(dataSocket);

		while (temp != "") {
			memset(buf, 0, sizeof buf);
			sscanf(temp.c_str(), "%[^\r]", buf);

			printf("mdelete %s?", buf);
			c = cin.get();
			if (c == 'y' || c == 'Y' || c == '\n')
				Delete(soc, buf);
			rewind(stdin);
			temp.erase(0, strlen(buf) + 2); //xóa thêm 2 kí tự \r\n
			chuanHoa(temp);
		}
	}
}
//Hàm tạo thư mục mới
void mkdir(SOCKET soc, string cmd)
{
	char buf[1024];
	int Bytes;

	cmd = cmd.erase(0, 5);
	chuanHoa(cmd);
	
	if (cmd == "")
	{
		std::cout << "Directory name: ";
		getline(cin, cmd);
	}
	memset(buf, 0, sizeof buf);
	sprintf(buf, "MKD %s\r\n", cmd.c_str());
	Bytes = send(soc, buf, strlen(buf), 0);
	//connSocket trả về cmd 
	memset(buf, 0, sizeof buf);
	Bytes = recv(soc, buf, sizeof buf, 0);
	printf("%s", buf);
}
//Hàm xóa thư mục
void rmdir(SOCKET soc, string cmd)
{
	char buf[1024];
	int Bytes;

	cmd = cmd.erase(0, 5);

	if (cmd == "")
	{
		std::cout << "Directory name: ";
		getline(cin, cmd);
	}		

	memset(buf, 0, sizeof buf);
	sprintf(buf, "RMD %s\r\n", cmd.c_str());

	Bytes = send(soc, buf, strlen(buf), 0);
	//connSocket trả về cmd 
	memset(buf, 0, sizeof buf);
	Bytes = recv(soc, buf, sizeof buf, 0);
	printf("%s", buf);
}