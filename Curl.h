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
public:
	Curl() { curl = NULL; }
	~Curl() {}

	static void GlobalInit();

	void Init();
	void Quit();

	string GetWork(string path="", uint timeout = 5, bool post = true);
	string SetWork(string work);

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
