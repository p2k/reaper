#ifndef CURLMUNACPP
#define CURLMUNACPP

class Curl
{
private:
	void* curl;

	string username;
	string password;
	string host;
	unsigned short port;

	enum EXEC_TYPE
	{
		GETWORK,
		GETWORK_LP,
		TESTWORK,
	};

	string Execute(Curl::EXEC_TYPE type, string work, string path, uint timeout);

public:
	string proxy;

	Curl() { curl = NULL; }
	~Curl() {}

	static void GlobalInit();

	void Init();
	void Quit();

	string GetWork_LP(string path="", uint timeout = 60);
	string GetWork(string path="", uint timeout = 5);
	string TestWork(string work);

	void SetUsername(string username_) { username = username_; }
	void SetPassword(string password_) { password = password_; }
	void SetHost(string host_) { host = host_; }
	void SetPort(string port_);

	string GetUsername() { return username; }
	string GetPassword() { return password; }
	string GetHost() { return host; }
	string GetPort();
};

#endif
