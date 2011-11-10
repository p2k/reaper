#include "Global.h"
#include "Config.h"

#include <fstream>

#include <map>
using std::map;
using std::pair;
using std::ifstream;
using std::ofstream;

#include <cstdio>
#include <algorithm>

void Config::Clear()
{
	config.clear();
}

void Config::Load(string filename, vector<string> included_already)
{
	included_already.push_back(filename);
	{
		FILE* filu = fopen(filename.c_str(), "r");
		if (filu == NULL)
			throw string("Config file " + filename + " not found.");
		fclose(filu);
	}

	map<string, string> config_values;
	config_values["aggression"] = "uint";
	config_values["worksize"] = "uint";
	config_values["threads_per_gpu"] = "uint";
	config_values["device"] = "array";
	config_values["kernel"] = "string";
	config_values["save_binaries"] = "bool";
	config_values["cpu_mining_threads"] = "uint";
	config_values["platform"] = "uint";
	config_values["enable_graceful_shutdown"] = "bool";
	config_values["host"] = "string";
	config_values["port"] = "string";
	config_values["user"] = "string";
	config_values["pass"] = "string";
	config_values["proxy"] = "string";
	config_values["long_polling"] = "bool";
	config_values["include"] = "string";

	ifstream filu(filename.c_str());
	while(!filu.eof())
	{
		string prop;
		filu >> prop;

		string value;
		filu >> value;

		if (prop == "include")
		{
			if (std::find(included_already.begin(), included_already.end(), value) != included_already.end())
			{
				cout << "Circular include: ";
				for(uint i=0; i<included_already.size(); ++i)
					cout << included_already[i] << " -> ";
				cout << value << endl;
			}
			else
			{
				Config includedconfig;
				includedconfig.Load(value, included_already);
				for(map<string,vector<string> >::iterator it = includedconfig.config.begin(); it != includedconfig.config.end(); ++it)
					for(uint i=0; i<it->second.size(); ++i)
						config[it->first] = it->second;
			}
		}

		if (config_values.find(prop) == config_values.end())
		{
			bool fail = true;
			CombiKey c = GetCombiKey(prop);
			if (c.base != "" && c.id != -1 && c.prop != "")
			{
				fail = false;
			}
			if (fail)
			{
				if (prop != "")
				{
					cout << "Warning: unknown property \"" << prop << "\" in configuration file." << endl;
					if (prop == "threads_per_device")
						cout << "The property \"threads_per_device\" should be called \"threads_per_gpu\". Please change it in the config." << endl;
				}
				continue;
			}
		}
		config[prop].push_back(value);
	}
	included_already.pop_back();
}

#include <algorithm>

/*
//function disabled because of "include" functionality
void Config::Save(string filename)
{
	ofstream filu(filename.c_str());

	vector<string> prioritykeys;
	prioritykeys.push_back("host");
	prioritykeys.push_back("port");
	prioritykeys.push_back("user");
	prioritykeys.push_back("pass");

	for(vector<string>::iterator it = prioritykeys.begin(); it != prioritykeys.end(); ++it)
	{
		if (config.find(*it) == config.end())
			continue;
		for(uint i=0; i<config[*it].size(); ++i)
		{
			filu << *it << " " << config[*it][i] << endl;
		}
	}
	filu << endl;

	for(map<string,vector<string> >::iterator it = config.begin(); it != config.end(); ++it)
	{
		if (std::find(prioritykeys.begin(), prioritykeys.end(),  it->first) != prioritykeys.end())
			continue;
		for(uint i=0; i<it->second.size(); ++i)
		{
			filu << it->first << " " << it->second[i] << endl;
		}
	}
}
*/
