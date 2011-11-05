#include "Global.h"
#include "Curl.h"

#include "curl/curl.h"
#include "Util.h"

void Curl::GlobalInit()
{
	curl_global_init(CURL_GLOBAL_ALL);
}

void Curl::Init()
{
	curl = curl_easy_init();
	if (curl == NULL)
	{
		throw string("libcurl initialization failure");
	}
}

void Curl::Quit()
{
	if (curl != NULL)
	{
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
}

string getworksentdata;
size_t GetWorkWriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	try 	
	{ 	
		for(uint i=0; i<size*nmemb; ++i) 	
		{ 		
			if(ptr!=NULL) 		
			{ 			
				char c = ((char*)ptr)[i]; 			
				getworksentdata.push_back(c); 		
			} 	
		}
	} 	
	catch(std::exception s) 	
	{ 		
		cout << "(1) Error: " << s.what() << endl; 	
	}
	return size*nmemb; 
}

string setworksentdata;
size_t SetWorkWriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	try 	
	{ 	
		for(uint i=0; i<size*nmemb; ++i) 	
		{ 		
			if(ptr!=NULL) 		
			{ 			
				char c = ((char*)ptr)[i]; 			
				setworksentdata.push_back(c); 		
			} 	
		}
	} 	
	catch(std::exception s) 	
	{ 		
		cout << "(2) Error: " << s.what() << endl; 	
	}
	return size*nmemb; 
}

string longpoll_url;
bool longpoll_active=false;
size_t HeaderCallback( void *ptr, size_t size, size_t nmemb, void *userdata)
{
	string hdr;
	for(uint i=0; i<size*nmemb; ++i)
	{
		char c = ((char*)ptr)[i];
		hdr.push_back(c);
	}
	if (!longpoll_active && hdr.length() >= 0x10 && hdr.substr(0,0xF) == "X-Long-Polling:")
	{
		longpoll_url = hdr.substr(0x10);
		longpoll_url = longpoll_url.substr(0, longpoll_url.length()-2);
		//cout << "Longpoll url -->" << longpoll_url << "<-- " << endl;
#ifdef LONGPOLLING
		longpoll_active = true;
#endif
	}
	return size*nmemb;
}

string Curl::GetWork(string path, uint timeout, bool post)
{
	getworksentdata.clear();

	curl_easy_setopt(curl, CURLOPT_URL, ("http://" + host + path).c_str());
	curl_easy_setopt(curl, CURLOPT_USERPWD, (username + ":" + password).c_str());
	curl_easy_setopt(curl, CURLOPT_PORT, port);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, GetWorkWriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);

	curl_slist* headerlist = NULL;
	headerlist = curl_slist_append(headerlist, "Content-Type: application/json");
	headerlist = curl_slist_append(headerlist, "Accept: application/json");
	headerlist = curl_slist_append(headerlist, "User-Agent: reaper/" REAPER_VERSION);

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);
	
	if (post) {
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, "{\"method\":\"sc_getwork\",\"params\":[],\"id\":1}");
	}
	else {
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, NULL);
		curl_easy_setopt(curl, CURLOPT_POST, 0);
	}

	CURLcode code = curl_easy_perform(curl);
	if(code != CURLE_OK)
	{
		if (code == CURLE_COULDNT_CONNECT)
		{
			cout << humantime() << "Could not connect. Server down?" << endl;
		}
		else
		{
			cout << humantime() << "Error " << code << " getting work. See http://curl.haxx.se/libcurl/c/libcurl-errors.html for error code explanations." << endl;
		}
		return string("");
	}
	curl_slist_free_all(headerlist);
	return getworksentdata;
}

string Curl::SetWork(string work)
{
	setworksentdata.clear();

	curl_easy_setopt(curl, CURLOPT_URL, ("http://" + host).c_str());
	curl_easy_setopt(curl, CURLOPT_USERPWD, (username + ":" + password).c_str());
	curl_easy_setopt(curl, CURLOPT_PORT, port);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, SetWorkWriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

	curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, ("{\"method\":\"sc_testwork\",\"id\":\"1\",\"params\":[\"" + work + "\"]}").c_str());

	curl_slist* headerlist = NULL;
	headerlist = curl_slist_append(headerlist, "Content-Type: application/json");
	headerlist = curl_slist_append(headerlist, "Accept: application/json");
	headerlist = curl_slist_append(headerlist, "User-Agent: reaper/" REAPER_VERSION);

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 1);

	CURLcode code = curl_easy_perform(curl);
	if(code != CURLE_OK)
	{
		if (code == CURLE_COULDNT_CONNECT)
		{
			cout << humantime() << "Could not connect. Server down?" << endl;
		}
		else
		{
			cout << humantime() << "Error " << code << " submitting work. See http://curl.haxx.se/libcurl/c/libcurl-errors.html for error code explanations." << endl;
		}
	}
	curl_slist_free_all(headerlist);
	return setworksentdata;
}

#undef SetPort
void Curl::SetPort(string port_) 
{ 
	port = FromString<unsigned short>(port_); 
}

string Curl::GetPort() 
{ 
	return ToString(port); 
}
