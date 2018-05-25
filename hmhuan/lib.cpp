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
	while (s[0] == ' ' || s[0] == '\t')
		s.erase(0, 1);
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
//hàm ls
void ls(SOCKET soc, struct sockaddr_in & data, string cmd)
{
	char buf[1024];
	int Bytes;
	SOCKET dataSocket;
	IP ip;
	int p1, p2;
	string ret;

	cmd.erase(0, 2);
	chuanHoa(cmd);
	int timeout = 0;
	setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
	//khoi tao datasocket
	dataSocket = socket(PF_INET, SOCK_STREAM, 0);
	data.sin_port++;

	ip.getIP(data);
	p1 = data.sin_port / 256;
	p2 = data.sin_port % 256;

	data.sin_port = htons(data.sin_port);
	bind(dataSocket, (struct sockaddr *)&data, sizeof data);
	listen(dataSocket, 0);
	//lenh PORT
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "PORT %d,%d,%d,%d,%d,%d\r\n", ip.a, ip.b, ip.c, ip.d, p1, p2);
	Bytes = send(soc, buf, strlen(buf), 0);
	
	memset(buf, 0, sizeof(buf));
	Bytes = recv(soc, buf, sizeof(buf), 0);
	if (Bytes == -1)
		return;
	printf("%s", buf);//	std::cout << buf; 
	//lenh NLST
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "NLST %s\r\n", cmd.c_str());
	Bytes = send(soc, buf, strlen(buf), 0);
	memset(buf, 0, sizeof buf);
	Bytes = recv(soc, buf, sizeof buf, 0);
	printf("%s", buf);//	std::cout << buf;

	/*data.sin_port = htons(data.sin_port);
	bind(dataSocket, (struct sockaddr *)&data, sizeof data);
	listen(dataSocket, 0);*/
	dataSocket = accept(dataSocket, NULL , NULL);

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
	data.sin_port = ntohs(data.sin_port);//hàm ngược hàm htons()
	
	closesocket(dataSocket);
	//connSocket trả về cmd phản hồi 
	memset(buf, 0, sizeof buf);
	Bytes = recv(soc, buf, sizeof buf, 0);
	printf("%s", buf);
}
//hàm dir
void dir(SOCKET soc, struct sockaddr_in &data,string cmd)
{
	char buf[1024];
	int Bytes;
	SOCKET dataSocket;
	IP ip;
	int p1, p2;
	string ret;

	cmd.erase(0, 3);
	chuanHoa(cmd);
	int timeout = 0;
	setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));

	//ket noi datasocket
	dataSocket = socket(PF_INET, SOCK_STREAM, 0);
	data.sin_port++;

	ip.getIP(data);
	p1 = data.sin_port / 256;
	p2 = data.sin_port % 256;

	//Tạo dataSocket và lắng nghe
	data.sin_port = htons(data.sin_port);
	bind(dataSocket, (struct sockaddr *)&data, sizeof data);
	listen(dataSocket, 0);

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "PORT %d,%d,%d,%d,%d,%d\r\n", ip.a, ip.b, ip.c, ip.d, p1, p2);
	Bytes = send(soc, buf, strlen(buf), 0);

	memset(buf, 0, sizeof(buf));
	Bytes = recv(soc, buf, sizeof(buf), 0);
	printf("%s", buf);//	std::cout << buf;

	memset(buf, 0, sizeof(buf));
	sprintf(buf, "LIST %s\r\n", cmd.c_str());
	Bytes = send(soc, buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	Bytes = recv(soc, buf, sizeof buf, 0);
	printf("%s", buf);//	std::cout << buf;

	/*data.sin_port = htons(data.sin_port);
	bind(dataSocket, (struct sockaddr *)&data, sizeof data);
	listen(dataSocket, 0);*/
	dataSocket = accept(dataSocket, NULL, NULL);

	while(1)
	{
		memset(buf, 0, sizeof buf);
		Bytes = recv(dataSocket, buf, 1023, 0);
		if (Bytes == 0)
			break;
		printf("%s", buf);
		if (Bytes != 1023)
			break;
	}

	closesocket(dataSocket);
	data.sin_port = ntohs(data.sin_port);
	//connSocket trả về cmd 
	memset(buf, 0, sizeof buf);
	Bytes = recv(soc, buf, sizeof buf, 0);
	printf("%s", buf);
}

void lcd(string cmd)
{	
	TCHAR buf[1024];
	DWORD ret;
	
	ret = GetCurrentDirectory(sizeof buf, buf);
	_tprintf(TEXT("Set current directory to %s\n"), buf);

}
//Hàm cd
void cd(SOCKET soc, string cmd)
{
	char buf[1024];
	int Bytes;

	cmd = cmd.erase(0, 2);
	while (cmd[0] == ' ')
		cmd.erase(0, 1);
	if (cmd.find(" ") != -1)
		cmd.erase(cmd.find(" "), cmd.length());
	if (cmd == "")
	{
		std::cout << "Remote directory: ";
		cin >> cmd;
	}
	memset(buf, 0, sizeof buf);
	sprintf(buf, "CWD %s\r\n", cmd.c_str());
	Bytes = send(soc, buf, strlen(buf), 0);
	//connSocket trả về cmd 
	memset(buf, 0, sizeof buf);
	Bytes = recv(soc, buf, sizeof buf, 0);
	printf("%s", buf);
}
//Hàm pwd
void pwd(SOCKET soc)
{
	char buf[1024];
	int Bytes;

	memset(buf, 0, sizeof buf);
	sprintf(buf, "PWD\r\n");
	Bytes = send(soc, buf, strlen(buf), 0);
	//connSocket trả về cmd 
	memset(buf, 0, sizeof buf);
	Bytes = recv(soc, buf, sizeof buf, 0);
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
void get(SOCKET soc, struct sockaddr_in &data, string cmd)
{
	char buf[1024], lcfile[1024], rmfile[1024]; //??2048 4096
	int Bytes, p1, p2;
	SOCKET dataSocket;
	IP ip;
	ofstream fOut;
	string temp;

	//TH1: get <rmfile> [<lcfile>]
	memset(lcfile, 0, sizeof lcfile);
	memset(rmfile, 0, sizeof rmfile);
	sscanf(cmd.c_str(), "%s%s", rmfile, lcfile);

	if (strcmp(lcfile, "") == 0)
		strcpy(lcfile, rmfile);
	temp = lcfile;
	//int t1, t2;
	////Xóa /|\ trong tên file llocal
	//while (1)
	//{
	//	t1 = temp.find("\\");
	//	if (t1 != -1)
	//		temp.erase(0, t1 + 1);
	//	t2 = temp.find("//");
	//	if (t2 != -1)
	//		temp.erase(0, t2 + 1);
	//	if (t1 == -1 && t2 == -1)
	//		break;
	//}
	//strcpy(lcfile, temp.c_str());
	//TH: get
	if (cmd == "")
	{
		std::cout << "Remote file: ";
		cin >> rmfile;
		std::cout << "Local file: ";
		cin >> lcfile; //ten file up len server
	}

	//khoi tao datasocket
	dataSocket = socket(PF_INET, SOCK_STREAM, 0);
	data.sin_port++;

	ip.getIP(data); p1 = data.sin_port / 256; p2 = data.sin_port % 256;

	data.sin_port = htons(data.sin_port);
	bind(dataSocket, (struct sockaddr *)&data, sizeof data);
	listen(dataSocket, 0);
	//lenh PORT 
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "PORT %d,%d,%d,%d,%d,%d\r\n", ip.a, ip.b, ip.c, ip.d, p1, p2);
	Bytes = send(soc, buf, strlen(buf), 0);
	memset(buf, 0, sizeof(buf));
	Bytes = recv(soc, buf, sizeof buf, 0);
	std::cout << buf;

	memset(buf, 0, sizeof buf);
	sprintf(buf, "RETR %s\r\n", rmfile);
	Bytes = send(soc, buf, strlen(buf), 0);

	memset(buf, 0, sizeof buf);
	Bytes = recv(soc, buf, sizeof buf, 0);
	std::cout << buf;

	sscanf(buf, "%d", &Bytes);
	if (Bytes != 550 && Bytes != 501) //kiểm tra File not found và syntax
	{
		fOut.open(lcfile, ios::out | ios::binary);
		if (fOut.is_open())
		{
			dataSocket = accept(dataSocket, (struct sockaddr *)&data, NULL);
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
	data.sin_port = ntohs(data.sin_port);
	closesocket(dataSocket);
}
//Hàm tải về nhiều file
void mget(SOCKET soc, struct sockaddr_in &data, string cmd) 
{
	char buf[1024], flnm[1024], c;
	string temp;
	SOCKET dataSocket;
	IP ip;
	int p1, p2, Bytes, k;

	int timeout = 0;
	setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
	//xóa mget và chuẩn hóa chuỗi cmd
	cmd.erase(0, 4);
	chuanHoa(cmd);
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
		memset(flnm, 0, sizeof flnm);
		sscanf(cmd.c_str(), "%s", flnm);//đọc file name của tệp cần lấy từ cmd

		//khoi tao datasocket
		dataSocket = socket(PF_INET, SOCK_STREAM, 0);
		data.sin_port++;

		ip.getIP(data);
		p1 = data.sin_port / 256;
		p2 = data.sin_port % 256;

		data.sin_port = htons(data.sin_port);
		bind(dataSocket, (struct sockaddr *)&data, sizeof data);
		listen(dataSocket, 0);
		//lenh PORT
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "PORT %d,%d,%d,%d,%d,%d\r\n", ip.a, ip.b, ip.c, ip.d, p1, p2);
		send(soc, buf, strlen(buf), 0);

		memset(buf, 0, sizeof(buf));
		recv(soc, buf, sizeof(buf), 0);
		//printf("%s", buf);//	std::cout << buf; 
		//lenh NLST
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "NLST %s\r\n", flnm);
		send(soc, buf, strlen(buf), 0);
		memset(buf, 0, sizeof buf);
		recv(soc, buf, sizeof buf, 0);
		//printf("%s", buf);//	std::cout << buf;

		sscanf(buf, "%d", &k);
		if (k != 550)
		{
			dataSocket = accept(dataSocket, NULL, NULL);
			temp = "";
			while (1)
			{
				memset(buf, 0, sizeof buf);
				Bytes = recv(dataSocket, buf, sizeof buf, 0);
				temp += buf;
				if (Bytes != 1024)
					break;
			}
			//cout << temp;
			//connSocket trả về cmd phản hồi 
			memset(buf, 0, sizeof buf);
			Bytes = recv(soc, buf, sizeof buf, 0);
			//printf("%s", buf);
		}
		else
			printf("#[%s]: %s", flnm, buf);
		data.sin_port = ntohs(data.sin_port);//hàm ngược hàm htons()
		closesocket(dataSocket);

		while (temp != "") {
			memset(buf, 0, sizeof buf);
			sscanf(temp.c_str(), "%s", buf);
			printf("mget %s?", buf);
			c = cin.get();
			if (c == 'y' || c == 'Y' || c == '\n')
				get(soc, data, cmd);
			rewind(stdin);
			temp.erase(0, strlen(buf) + 2); //xóa thêm 2 kí tự \r\n
			chuanHoa(temp);
		}
		cmd.erase(0, strlen(flnm));
	}
}

void put(SOCKET soc, struct sockaddr_in &data, string cmd)
{
	char buf[4096], lcfile[1024], rmfile[1024]; //co the tang buffer them??
	string temp;
	int Bytes, p1, p2;
	SOCKET dataSocket;
	IP ip;
	ifstream fIn;

	//TH1: put <lcfile> [<rmfile>]
	memset(lcfile, 0, sizeof lcfile);
	memset(rmfile, 0, sizeof rmfile);
	sscanf(cmd.c_str(), "%s%s", lcfile, rmfile);

	if (strcmp(rmfile, "") == 0)
		strcpy(rmfile, lcfile);
	temp = rmfile;
	int t1, t2;
	//Xóa /|\ trong tên file remote
	while (1)
	{
		t1 = temp.find("\\");
		if (t1 != -1)
			temp.erase(0, t1 + 1);
		t2 = temp.find("//");
		if (t2 != -1)
			temp.erase(0, t2 + 1);
		if (t1 == -1 && t2 == -1)
			break;
	}
	strcpy(rmfile, temp.c_str());
	//TH: put
	if (cmd == "")
	{
		std::cout << "Local file: ";
		cin >> lcfile;
		std::cout << "Remote file: ";
		cin >> rmfile; //ten file up len server
	}

	fIn.open(lcfile, ios::in | ios::binary);
	if (fIn.is_open())
	{
		//khoi tao datasocket
		dataSocket = socket(PF_INET, SOCK_STREAM, 0);
		data.sin_port++;
		//Lấy IP:PORT
		ip.getIP(data);
		p1 = data.sin_port / 256;
		p2 = data.sin_port % 256;

		data.sin_port = htons(data.sin_port);
		bind(dataSocket, (struct sockaddr *)&data, sizeof data);
		listen(dataSocket, 0);

		//lenh PORT
		memset(buf, 0, sizeof buf);
		sprintf(buf, "PORT %d,%d,%d,%d,%d,%d\r\n", ip.a, ip.b, ip.c, ip.d, p1, p2);
		Bytes = send(soc, buf, strlen(buf), 0);
		memset(buf, 0, sizeof buf);
		Bytes = recv(soc, buf, sizeof buf, 0);
		cout << "\n" << buf;

		memset(buf, 0, sizeof buf);
		sprintf(buf, "STOR %s\r\n", rmfile);
		Bytes = send(soc, buf, strlen(buf), 0);
		memset(buf, 0, sizeof buf);
		Bytes = recv(soc, buf, sizeof buf, 0);
		cout << buf;

		sscanf(buf, "%d", &Bytes);
		if (Bytes != 550 && Bytes != 501)
		{
			/*data.sin_port = htons(data.sin_port);
			bind(dataSocket, (struct sockaddr *)&data, sizeof data);
			listen(dataSocket, 0);*/
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
			data.sin_port = ntohs(data.sin_port);
			//connSocket trả về cmd 
			int timeout = 0;
			setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));
			memset(buf, 0, sizeof buf);
			Bytes = recv(soc, buf, sizeof buf, 0);
			printf("%s", buf);
			fIn.close();

		}
	}
	else
	{
		cout << "File not found\n";
	}

}

void mput(SOCKET soc, struct sockaddr_in &data, string cmd)
{
	char buf[1024];
	//xóa mput và chuẩn hóa chuỗi cmd
	cmd.erase(0, 4);
	chuanHoa(cmd);
	//nếu cmd rỗng tiến hành đọc tên file
	if (cmd == "")
	{
		cout << "Local files: ";
		rewind(stdin);
		getline(cin, cmd);
	}

	while (1)
	{
		memset(buf, 0, sizeof buf);
		sscanf(cmd.c_str(), "%s", buf);
		if (buf[0] != '\0')
		{
			cout << "[" << buf << "]: ";
			put(soc, data, buf);
		}
		else
		{
			cout << "Warning!! Local file is Null.\n";
		}
		cmd.erase(0, strlen(buf));
		chuanHoa(cmd);
		if (cmd == "")
			break;
	}
}

//Hàm xóa 1 file
void Delete(SOCKET soc, string cmd)
{
	char buf[1024];
	int Bytes;

	chuanHoa(cmd);
	if (cmd == "")
	{
		std::cout << "Remote file: ";
		cin >> cmd;
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
void mDelete(SOCKET soc, struct sockaddr_in & data, string cmd)
{
	char buf[1024], flnm[512], c;
	SOCKET dataSocket;
	IP ip;
	int p1, p2, Bytes, k;
	string temp, code;

	int timeout = 0;
	setsockopt(soc, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(int));

	cmd.erase(0, 7);//xoa mdelete
	chuanHoa(cmd);
	/*memset(buf, 0, sizeof buf);
	sprintf(buf, "TYPE A\r\n");
	send(soc, buf, sizeof buf, 0);
	
	memset(buf, 0, sizeof buf);
	recv(soc, buf, strlen(buf), 0);*/
	if (cmd == "") //doc file neu cmd chi co lenh mdelete
	{
		std::cout << "Remote files: ";
		getline(cin, cmd);
	}

	while (cmd != "")
	{
		chuanHoa(cmd);
		memset(flnm, 0, sizeof flnm);
		sscanf(cmd.c_str(), "%s", flnm);
		
		//khoi tao datasocket
		dataSocket = socket(PF_INET, SOCK_STREAM, 0);
		data.sin_port++;

		ip.getIP(data);
		p1 = data.sin_port / 256;
		p2 = data.sin_port % 256;

		data.sin_port = htons(data.sin_port);
		bind(dataSocket, (struct sockaddr *)&data, sizeof data);
		listen(dataSocket, 0);
		//lenh PORT
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "PORT %d,%d,%d,%d,%d,%d\r\n", ip.a, ip.b, ip.c, ip.d, p1, p2);
		send(soc, buf, strlen(buf), 0);

		memset(buf, 0, sizeof(buf));
		recv(soc, buf, sizeof(buf), 0);
		//printf("%s", buf);//	std::cout << buf; 
		//lenh NLST
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "NLST %s\r\n", flnm);
		send(soc, buf, strlen(buf), 0);
		memset(buf, 0, sizeof buf);
		recv(soc, buf, sizeof buf, 0);
		//printf("%s", buf);//	std::cout << buf;

		sscanf(buf, "%d", &k);
		if (k != 550)
		{
			dataSocket = accept(dataSocket, NULL, NULL);
			temp = "";
			while (1)
			{
				memset(buf, 0, sizeof buf);
				Bytes = recv(dataSocket, buf, sizeof buf, 0);
				temp += buf;
				if (Bytes != 1024)
					break;
			}
			//cout << temp;
			//connSocket trả về cmd phản hồi 
			memset(buf, 0, sizeof buf);
			Bytes = recv(soc, buf, sizeof buf, 0);
			//printf("%s", buf);
		}
		else
			printf("#[%s]: %s", flnm, buf);
		data.sin_port = ntohs(data.sin_port);//hàm ngược hàm htons()
		closesocket(dataSocket);

		while (temp != "") {
			memset(buf, 0, sizeof buf);
			sscanf(temp.c_str(), "%s", buf);
			printf("mdelete %s?", buf);
			c = cin.get();
			if (c == 'y' || c == 'Y' || c == '\n')
				Delete(soc, buf);
			rewind(stdin);
			temp.erase(0, strlen(buf) + 2); //xóa thêm 2 kí tự \r\n
			chuanHoa(temp);
		}
		cmd.erase(0, strlen(flnm));
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
	chuanHoa(cmd);
	
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