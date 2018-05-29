#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <WinSock2.h>
#include <string>
#include <fstream>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

struct IP { 
	int a, b, c, d;
	void getIP(struct sockaddr_in s)
	{
		a = s.sin_addr.S_un.S_un_b.s_b1;
		b = s.sin_addr.S_un.S_un_b.s_b2;
		c = s.sin_addr.S_un.S_un_b.s_b3;
		d = s.sin_addr.S_un.S_un_b.s_b4;
	}
};
void chuanHoa(string &s);
string getFileName(string &cmd);


void errexit(const char *, ...);
void pause(void);
void replylogcode(int code);

//1. 
void LoginFTP(SOCKET socket_descriptor, HOSTENT * pHostEnt);

//2. Liệt kê được danh sách các thư mục, tập tin trên Server (ls, dir)
void dir(SOCKET soc, string cmd, bool modePasv);
void ls(SOCKET soc, string cmd, bool modePasv);

//3. Upload một file đến Server
void put(SOCKET soc, string cmd, bool modePasv);

//4. Download một file từ Server
void get(SOCKET soc, string cmd, bool modePasv);

//5. Upload nhiều file đến Server
void mput(SOCKET soc, string cmd, bool modePasv);

//6. Download nhiều file từ Server
void mget(SOCKET soc, string cmd, bool modePasv);

//7. Thay đổi đường dẫn trên Server
void cd(SOCKET soc, string cmd);

//8. Thay đổi đường dẫn dưới client
void lcd(char cmd[]);

//9 Xóa file
void Delete(SOCKET soc, string cmd);
void mDelete(SOCKET soc, string cmd, bool modePasv);

//11. Tạo thư mục trên Server
void mkdir(SOCKET soc, string cmd);

//12. Xóa thư mục rỗng trên Server
void rmdir(SOCKET soc, string cmd);

//13. hiển thi đường dẫn hiện tại
void pwd(SOCKET soc);

//14. Chuyển sang trạng thái passive
bool passiveMode(SOCKET soc, SOCKET &dsoc);
bool activeMode(SOCKET soc, SOCKET &dsoc);

//15. Thoát khải Server
void quit(SOCKET soc);