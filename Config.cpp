#include "Global.h"
#include "Config.h"

#include <fstream>

#include <map>
using std::map;
using std::pair;
using std::ifstream;
using std::ofstream;

#include <cstdio>

void Config::Load( string filename )
{
	SetConfigFileName(filename);
	{
		FILE* filu = fopen(GetConfigFileName().c_str(), "r");
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

	ifstream filu(GetConfigFileName().c_str());
	while(!filu.eof())
	{
		string prop;
		filu >> prop;

		string value;
		filu >> value;

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
					cout << "Warning: unknown property \"" << prop << "\" in configuration file." << endl;
				continue;
			}
		}
		config[prop].push_back(value);
	}
}

void Config::Save()
{
	ofstream filu(GetConfigFileName().c_str());
	for(map<string,vector<string> >::iterator it = config.begin(); it != config.end(); ++it)
	{
		for(uint i=0; i<it->second.size(); ++i)
		{
			filu << it->first << " " << it->second[i] << endl;
		}
	}
}

string Config::GetConfigFileName()
{
	return configfilename;
}

void Config::SetConfigFileName( string filename )
{
	configfilename = filename;
}
