/*
Chú ý:
	_setmode(_fileno(stdin), _O_U16TEXT);
	_setmode(_fileno(stdout), _O_U16TEXT);
	Khi set chế độ này thì các cin, cout, scanf, printf đều phải chuyển sang wcin, wcout, wscanf, wprintf.
	Trong trường hợp dùng trong một chức năng nhỏ nào đó thì set mode unicode xong rồi set lại mode ascii
	sẽ không ảnh hưởng đến cin, cout, scanf, printf bên ngoài
*/
/*
	Hàm lcd: tham số đầu vào là dòng cmd người dùng nhập vào
*/
void lcd(char direct[]) {
	static wchar_t rootPath[BUFSIZ]=L""; // Lưu lại path của thư mục gốc ban đầu
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
		check = SetCurrentDirectory(direct + 4);
	}

	if (check) {
		GetCurrentDirectoryW(BUFSIZ, info);
		wcout<< "Local directory now " << info << "." << endl;
	}
	else {
		wcout << "File not found" << endl;
	}
	// Establish mode for print ascii text
	_setmode(_fileno(stdin), _O_TEXT);
	_setmode(_fileno(stdout), _O_TEXT);
}

/*
Hàm passiveMode:
	+  Tham số đầu vào: socket control, socket data, cmd cần gửi lên server
	+ Trả về: bool để biết được thiết lập kết nối thành công hay chưa
*/
bool passiveMode(SOCKET soc, SOCKET &dsoc, char cmd[]) {
	char buf[BUFSIZ + 1];
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
	Sleep(500);
	if (tmpres > -1) {
		
		// Receive information about address and port server sent down
		memset(buf, 0, sizeof buf);
		tmpres = recv(soc, buf, BUFSIZ, 0);
		
		//kiem tra co phai ma loi 227
		char temp_p[8];
		strncpy(temp_p, buf, 3);
		temp_p[3] = '\0';		
		// get address and port of server
		if (strcmp("227", temp_p) > -1) {
			//tien hanh boc tach de lay port
			string receive(buf);
			int pos1 = receive.find('(');
			int pos2 = receive.find(')');
			string c1 = receive.substr(pos1 + 1, pos2 - pos1 - 1);

			for (int i = 1; i <= 4; i++) {
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

			// Send cmd
			send(soc, cmd, strlen(cmd), 0);
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}

	return true;
}

/*
Hàm passiveMode:
	+  Tham số đầu vào: socket control, socket data, cmd cần gửi lên server
	+ Trả về: bool để biết được thiết lập kết nối thành công hay chưa
*/
bool activeMode(SOCKET soc, SOCKET &dsoc, char cmd[])
{
	char buf[BUFSIZ + 1];
	//int p1 = 223, p2 = 0;
	int tmpres;
	static int port_add = 1;
	sockaddr_in dataAddr;
	int len = sizeof(dataAddr);
	

	// Open socket dsoc
	dsoc = socket(PF_INET, SOCK_STREAM, 0);
	if (dsoc == INVALID_SOCKET)
		errexit("Socket creation failed: %d\n", WSAGetLastError());
	// Config dataAddr
	dataAddr.sin_family = AF_INET;
	memset(dataAddr.sin_zero, 0, 8);

	if (getsockname(soc, (sockaddr*)&dataAddr, &len) > -1) {
		dataAddr.sin_port = htons(ntohs(dataAddr.sin_port) + port_add);
		port_add++;
	}
	else {
		return false;
	}

	if (bind(dsoc, (sockaddr*)&dataAddr, len) > -1) {
		if (listen(dsoc, SOMAXCONN) > -1) {
			// Prepare port for client to listen from server
			unsigned int a, b, c, d, p1, p2, p;
			a = dataAddr.sin_addr.S_un.S_un_b.s_b1;
			b = dataAddr.sin_addr.S_un.S_un_b.s_b2;
			c = dataAddr.sin_addr.S_un.S_un_b.s_b3;
			d = dataAddr.sin_addr.S_un.S_un_b.s_b4;
			p = ntohs(dataAddr.sin_port);
			p1 = p / 256;
			p2 = p % 256;

			// Send port to server
			sprintf(buf, "PORT %d,%d,%d,%d,%d,%d\r\n", a, b, c, d, p1, p2);
			tmpres = send(soc, buf, strlen(buf), 0);
			memset(buf, 0, sizeof buf);
			recv(soc, buf, BUFSIZ, 0);
			cout << buf << endl;

			// Send cmd to server
			send(soc, cmd, strlen(cmd), 0);

			// Accept connection
			len = sizeof(dataAddr);
			dsoc = accept(dsoc, (sockaddr*)&dataAddr, &len);
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}

	return true;
}