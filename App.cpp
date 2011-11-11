#include "Global.h"
#include "App.h"
#include "Util.h"

#include "json/json.h"

uchar HexToChar(char data)
{
	if (data <= '9')
		return data-'0';
	else if (data <= 'Z')
		return data-'7';
	else
		return data-'W';
}

#include "AppOpenCL.h"

#ifndef CPU_MINING_ONLY
extern vector<_clState> GPUstates;
#endif
extern vector<Reap_CPU_param> CPUstates;

uchar HexToChar(char h, char l)
{
	return HexToChar(h)*16+HexToChar(l);
}

const char* hextable[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f"};
string CharToHex(uchar c)
{
	return string(hextable[c/16]) + string(hextable[c%16]);
}

vector<uchar> HexStringToVector(string str)
{
	vector<uchar> ret;
	ret.assign(str.length()/2, 0);
	for(uint i=0; i<str.length(); i+=2)
	{
		ret[i/2] = HexToChar(str[i+0], str[i+1]);
	}
	return ret;
}
string VectorToHexString(vector<uchar> vec)
{
	string ret;
	for(uint i=0; i<vec.size(); i++)
	{
		ret += CharToHex(vec[i]);
	}
	return ret;
}

void DoubleSHA256(uint* output2, uint* workdata, uint* midstate);

void LineClear()
{
	cout << "\r                                                                      \r";
}

#ifdef WIN32
void Wait_ms(uint n);
#undef SetPort
#else
void Wait_ms(uint n);
#endif

#include "Config.h"

Config config;
GlobalConfs globalconfs;

ullint shares_valid = 0;
ullint shares_invalid = 0;
ullint shares_hwinvalid = 0;
clock_t current_work_time = 0;

extern Work current_work;

bool ShareTest(uint* workdata);

bool getwork_now = false;

void SubmitShare(Curl& curl, vector<uchar>& w)
{
	if (w.size() != 128)
	{
		cout << "SubmitShare: Size of share is " << w.size() << ", should be 128" << endl;
		return;
	}
	try 	
	{ 	
		string ret = curl.TestWork(VectorToHexString(w));
		Json::Value root;
		Json::Reader reader;
		bool parse_success = reader.parse(ret, root);
		if (parse_success)
		{
			Json::Value result = root.get("result", "null");
			if (result.isObject())
			{
				Json::Value work = result.get("work", "null");
				if (work.isArray())
				{
					Json::Value innerobj = work.get(Json::Value::UInt(0), "");
					if (innerobj.isObject())
					{
						Json::Value share_valid = innerobj.get("share_valid", "null");
						if (share_valid.isBool())
						{
							if (share_valid.asBool())
							{
								++shares_valid;
							}
							else
							{
								getwork_now = true;
								++shares_invalid;
							}
						}
						//Json::Value block_valid = innerobj.get("block_valid");
					}
				}
			}
		}
		else
		{
			cout << "Weird response from server." << endl;
		}
	}
	catch(std::exception s)
	{
		cout << "(3) Error: " << s.what() << endl;
	}
}

bool sharethread_active;
void* ShareThread(void* param)
{
	cout << "Share thread started" << endl;
	Curl curl;
	curl.Init();

	Curl* parent_curl = (Curl*)param;
	
	curl.SetUsername(parent_curl->GetUsername());
	curl.SetPassword(parent_curl->GetPassword());
	curl.SetHost(parent_curl->GetHost());
	curl.SetPort(parent_curl->GetPort());

	while(!shutdown_now)
	{
		sharethread_active = true;
		Wait_ms(100);
#ifndef CPU_MINING_ONLY
		foreachgpu()
		{
			if (!it->shares_available)
				continue;
			pthread_mutex_lock(&it->share_mutex);
			it->shares_available = false;
			deque<vector<uchar> > v;
			v.swap(it->shares);
			pthread_mutex_unlock(&it->share_mutex);
			while(!v.empty())
			{
				SubmitShare(curl, v.back());
				v.pop_back();
			}
		}
#endif
		foreachcpu()
		{
			if (!it->shares_available)
				continue;
			pthread_mutex_lock(&it->share_mutex);
			it->shares_available = false;
			deque<vector<uchar> > v;
			v.swap(it->shares);
			pthread_mutex_unlock(&it->share_mutex);
			while(!v.empty())
			{
				SubmitShare(curl, v.back());
				v.pop_back();
			}
		}
	}
	pthread_exit(NULL);
	return NULL;
}

extern string longpoll_url;
extern bool longpoll_active;

struct LongPollThreadParams
{
	Curl* curl;
	App* app;
};

#include "RSHash.h"

void* LongPollThread(void* param)
{
	LongPollThreadParams* p = (LongPollThreadParams*)param; 
	Curl curl;
	curl.Init();

	Curl* parent_curl = p->curl;
	
	curl.SetUsername(parent_curl->GetUsername());
	curl.SetPassword(parent_curl->GetPassword());
	curl.SetHost(parent_curl->GetHost());
	curl.SetPort(parent_curl->GetPort());

	string LP_url = longpoll_url;
	string LP_path;

	cout << "Long polling URL: [" << LP_url << "]. trying to parse." << endl;
	clock_t lastcall = 0;

	{//parsing LP address
		vector<string> exploded = Explode(LP_url, '/');
		if (exploded.size() >= 2 && exploded[0] == "http:")
		{
			vector<string> exploded2 = Explode(exploded[1], ':');
			if (exploded2.size() != 2)
				goto couldnt_parse;
			cout << "LP Host: " << exploded2[0] << endl;
			curl.SetHost(exploded2[0]);
			cout << "LP Port: " << exploded2[1] << endl;
			curl.SetPort(exploded2[1]);
			if (exploded.size() <= 2)
				LP_path = '/';
			else
				LP_path = "/" + exploded[2];
			cout << "LP Path: " << LP_path << endl;
		}
		else if (LP_url.length() > 0 && LP_url[0] == '/')
		{
			LP_path = LP_url;
			cout << "LP Path: " << LP_path << endl;
		}
		else
		{
			goto couldnt_parse;
		}
	}

	while(!shutdown_now)
	{
		clock_t ticks = ticker();
		if (ticks-lastcall < 5000)
		{
			Wait_ms(ticks-lastcall);
		}
		lastcall = ticks;
		p->app->Parse(curl.GetWork_LP(LP_path, 60));
	}
	pthread_exit(NULL);
	return NULL;

couldnt_parse:
	cout << "Couldn't parse long polling URL [" << LP_url << "]. turning LP off." << endl;
	pthread_exit(NULL);
	return NULL;
}

bool shutdown_now=false;
void* ShutdownThread(void* param)
{
	cout << "Press [Q] and [Enter] to quit" << endl;
	while(shutdown_now == false)
	{
		string s;
		std::cin >> s;
		if (s == "q" || s == "Q")
			shutdown_now = true;
	}
	cout << "Quitting." << endl;
	pthread_exit(NULL);
	return NULL;
}

#include <sstream>
using std::stringstream;

void App::Main(vector<string> args)
{
	cout << "\\|||||||||||||||||||||/" << endl;
	cout << "-  Reaper " << REAPER_VERSION << " " << REAPER_PLATFORM << "  -" << endl;
	cout << "-   coded by mtrlt    -" << endl;
	cout << "/|||||||||||||||||||||\\" << endl;
	cout << endl;
	string config_name = "reaper.conf";
	bool old_args = false;
	if (args.size() < 5) // new arg format
	{
		if (args.size() >= 2)
			config_name = args[1];
	}
	else //old arg format
	{
		if (args.size() >= 6)
		{
			config_name = args[5];
		}
		old_args = true;
	}
	getworks = 0;
	config.Load(config_name);
	if (!old_args)
	{
		if (config.GetValue<string>("host") == "" ||
			config.GetValue<string>("port") == "" ||
			config.GetValue<string>("user") == "" ||
			config.GetValue<string>("pass") == "")
			throw string("The config is missing one of host/port/user/pass.");
	}
	globalconfs.local_worksize = config.GetValue<uint>("worksize");
	{
		if (config.GetValue<string>("aggression") == "max")
		{
			globalconfs.global_worksize = 1<<11;
			globalconfs.max_aggression = true;
		}
		else
		{
			globalconfs.global_worksize = (1<<config.GetValue<uint>("aggression"));
			globalconfs.max_aggression = false;
		}
	}
	globalconfs.threads_per_gpu = config.GetValue<uint>("threads_per_gpu");
	if (config.GetValue<string>("kernel") == "")
		config.SetValue("kernel", 0, "reaper.cl");
	globalconfs.save_binaries = config.GetValue<bool>("save_binaries");
	uint numdevices = config.GetValueCount("device");
	for(uint i=0; i<numdevices; ++i)
		globalconfs.devices.push_back(config.GetValue<uint>("device", i));

	globalconfs.cputhreads = config.GetValue<uint>("cpu_mining_threads");

#ifdef CPU_MINING_ONLY
	if (globalconfs.cputhreads == 0)
	{
		throw string("cpu_mining_threads is zero. Nothing to do, quitting.");
	}
#endif
	if (globalconfs.cputhreads == 0 && globalconfs.threads_per_gpu == 0)
	{
		throw string("No CPU or GPU mining threads.. please set either cpu_mining_threads or threads_per_gpu to something other than 0.");
	}
	globalconfs.platform = config.GetValue<uint>("platform");

	if (globalconfs.local_worksize > globalconfs.global_worksize)
	{
		cout << "Aggression is too low for the current worksize. Increasing." << endl;
		globalconfs.global_worksize = globalconfs.local_worksize;
	}

	BlockHash_Init();
	current_work.old = true;

	Curl::GlobalInit();
	curl.Init();
	if (old_args)
	{
		curl.SetHost(args[1]);
		curl.SetPort(args[2]);
		curl.SetUsername(args[3]);
		curl.SetPassword(args[4]);
	}
	else
	{
		curl.SetHost(config.GetValue<string>("host"));
		curl.SetPort(config.GetValue<string>("port"));
		curl.SetUsername(config.GetValue<string>("user"));
		curl.SetPassword(config.GetValue<string>("pass"));
	}
	curl.proxy = config.GetValue<string>("proxy");

	pthread_t sharethread;
	pthread_create(&sharethread, NULL, ShareThread, &curl);

	opencl.Init();
	cpuminer.Init();

	Parse(curl.GetWork());


	const int work_update_period_ms = 2000;

	pthread_t longpollthread;
	LongPollThreadParams lp_params;
	if (config.GetValue<bool>("long_polling") && longpoll_active)
	{
		cout << "Activating long polling." << endl;
		lp_params.app = this;
		lp_params.curl = &curl;
		pthread_create(&longpollthread, NULL, LongPollThread, &lp_params);
	}

	if (config.GetValue<bool>("enable_graceful_shutdown"))
	{
		pthread_t shutdownthread;
		pthread_create(&shutdownthread, NULL, ShutdownThread, NULL);
	}

	clock_t ticks = ticker();
	clock_t starttime = ticker();
	workupdate = ticker();

	clock_t sharethread_update_time = ticker();

	while(!shutdown_now)
	{
		Wait_ms(100);
		clock_t timeclock = ticker();
		if (timeclock - current_work_time >= WORK_EXPIRE_TIME_SEC*1000)
		{
			if (!current_work.old)
			{
				cout << humantime() << "Work too old... waiting for getwork.    " << endl;
			}
			current_work.old = true;
		}
		if (sharethread_active)
		{
			sharethread_active = false;
			sharethread_update_time = timeclock;
		}
		if (timeclock-sharethread_update_time >= SHARE_THREAD_RESTART_THRESHOLD_SEC*1000)
		{
			cout << humantime() << "Share thread messed up. Starting another one.   " << endl;
			pthread_create(&sharethread, NULL, ShareThread, &curl);
		}
		if (getwork_now || timeclock - workupdate >= work_update_period_ms)
		{
			Parse(curl.GetWork());
			getwork_now = false;
		}
		if (timeclock - ticks >= 1000)
		{
			ullint totalhashesGPU=0;
#ifndef CPU_MINING_ONLY
			foreachgpu()
			{
				totalhashesGPU += it->hashes;
			}
#endif
			ullint totalhashesCPU=0;
			foreachcpu()
			{
				totalhashesCPU += it->hashes;
			}

			ticks += (timeclock-ticks)/1000*1000;
			float stalepercent = 0.0f;
			if (shares_valid+shares_invalid+shares_hwinvalid != 0)
				stalepercent = 100.0f*float(shares_invalid+shares_hwinvalid)/float(shares_invalid+shares_valid+shares_hwinvalid);
			if (ticks-starttime == 0)
				cout << dec << "   ??? kH/s, shares: " << shares_valid << "|" << shares_invalid << "|" << shares_hwinvalid << ", invalid " << stalepercent << "%, " << (ticks-starttime)/1000 << "s    \r";
			else
			{
				stringstream stream;
				stream.precision(4);
				if (totalhashesGPU != 0)
					stream << "GPU " << double(totalhashesGPU)/(ticks-starttime) << "kH/s, ";
				if (totalhashesCPU != 0)
					stream << "CPU " << double(totalhashesCPU)/(ticks-starttime) << "kH/s, ";

				cout << dec << stream.str() << "shares: " << shares_valid << "|" << shares_invalid << "|" << shares_hwinvalid << ", invalid " << stalepercent << "%, " << (ticks-starttime)/1000 << "s    \r";
				cout.flush();
			}
		}
	}
	cpuminer.Quit();
	opencl.Quit();
	curl.Quit();

	BlockHash_DeInit();
}

bool targetprinted=false;

pthread_mutex_t current_work_mutex = PTHREAD_MUTEX_INITIALIZER;
Work current_work;

void App::Parse(string data)
{
	workupdate = ticker();
	if (data == "")
	{
		cout << humantime() << "Couldn't connect to server. Trying again in a few seconds... " << endl;
		return;
	}
	Json::Value root, result, error;
	Json::Reader reader;
	bool parsing_successful = reader.parse( data, root );
	if (!parsing_successful)
	{
		goto got_error;
	}

	result = root.get("result", "null");
	error = root.get("error", "null");
	
	if (result.isObject())
	{
		Json::Value::Members members = result.getMemberNames();
		uint neededmembers=0;
		for(Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
		{
			if (*it == "data")
				++neededmembers;
		}
		if (neededmembers != 1 || !result["data"].isString())
		{
			goto got_error;
		}
	
		++getworks;
		Work newwork;
		newwork.data = HexStringToVector(result["data"].asString());
		newwork.old = false;
		newwork.time = ticker();
		current_work_time = ticker();

		if (!targetprinted)
		{
			targetprinted = true;
			cout << "target_share: " << result["target_share"].asString() << endl;
		}
		newwork.target_share = HexStringToVector(result["target_share"].asString().substr(2));
		newwork.ntime_at_getwork = (*(ullint*)&newwork.data[76]) + 1;

		current_work.time = ticker();
		pthread_mutex_lock(&current_work_mutex);
		current_work = newwork;
		pthread_mutex_unlock(&current_work_mutex);
		return;
	}
	else if (!error.isNull())
	{
		cout << humantime() << error.asString() << endl;
		cout << humantime() << "Code " << error["code"].asInt() << ", \"" << error["message"].asString() << "\"" << endl;
	}
got_error:
	cout << humantime() << "Error with server: " << data << endl;
	return;
}
