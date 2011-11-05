#ifndef CONFIG_H
#define CONFIG_H

#include <map>
using std::map;

#include <string>
using std::string;

#include "Util.h"

class Config
{
private:
	map<string, vector<string> > config;
	string configfilename;

	struct CombiKey
	{
		string base;
		size_t id;
		string prop;

		CombiKey(){id=-1;}
		~CombiKey(){}
	};

	//returns default (base "", id -1, prop "") if key is not a CombiKey
	CombiKey GetCombiKey(string keyname)
	{
		CombiKey cs;
		size_t dotplace = keyname.find('.');
		if (dotplace == string::npos || dotplace == keyname.length()-1)
			return cs;
		string base_plus_id = keyname.substr(0,dotplace);
		string prop = keyname.substr(dotplace+1);
		if (base_plus_id == "" || prop == "")
			return cs;
		size_t idplace = base_plus_id.find_last_not_of("0123456789");
		if (idplace == base_plus_id.length()-1)
			return cs;
		string base = base_plus_id.substr(0,idplace+1);
		if (base == "")
			return cs;
		size_t id = FromString<uint>(base_plus_id.substr(idplace+1));
		cs.base = base;
		cs.id = id;
		cs.prop = prop;
		return cs;
	}

public:
	void Save();
	void Load(string filename);

	template<typename T>
	void SetValue(string key, uint index, T val)
	{
		if (config[key].size() <= index)
			return;
		config[key][index] = ToString(val);
		Save();
	}

	template<typename T>
	void SetCombiValue(string base, size_t id, string prop, uint index, T val)
	{
		string key = base + ToString(id) + "." + prop;
		SetValue<T>(key, index, val);
	}

	template<typename T>
	T GetValue(string key, uint index = 0)
	{
		if (config[key].size() <= index)
			return T();
		return FromString<T>(config[key][index]);
	}

	template<typename T>
	T GetCombiValue(string base, size_t id, string prop, uint index = 0)
	{
		string key = base + ToString(id) + "." + prop;
		return GetValue<T>(key, index);
	}

	uint GetValueCount(string key)
	{
		return (uint)config[key].size();
	}
	string GetConfigFileName();
	void SetConfigFileName( string filename );
};
extern Config config;
#endif
