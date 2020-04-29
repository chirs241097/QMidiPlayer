#ifndef QMPSETTINGSRO_H
#define QMPSETTINGSRO_H

#include <map>
#include <string>
#include <vector>

#include <QVariant>
#include <QPointer>

struct qmpOptionR
{
	enum ParameterType{
		parameter_int=0,
		parameter_uint,
		parameter_bool,
		parameter_double,
		parameter_str,
		parameter_enum,
		parameter_url,
		parameter_custom=0x100
	};

	std::string desc;
	ParameterType type;
	QVariant defaultval,minv,maxv;
	std::vector<std::string> enumlist;

	qmpOptionR(){}
	qmpOptionR(
			std::string _desc,
			ParameterType _t,QVariant _def=QVariant(),
			QVariant _min=QVariant(),QVariant _max=QVariant()):
		desc(_desc),
		type(_t),
		defaultval(_def),
		minv(_min),
		maxv(_max){}
};

class qmpSettingsRO
{
public:
	qmpSettingsRO();
	void registerOptionInt(std::string tab,std::string desc,std::string key,int min,int max,int defaultval);
	int getOptionInt(std::string key);
	void setOptionInt(std::string key,int val);
	void registerOptionUint(std::string tab,std::string desc,std::string key,unsigned min,unsigned max,unsigned defaultval);
	unsigned getOptionUint(std::string key);
	void setOptionUint(std::string key,unsigned val);
	void registerOptionBool(std::string tab,std::string desc,std::string key,bool defaultval);
	bool getOptionBool(std::string key);
	void setOptionBool(std::string key,bool val);
	void registerOptionDouble(std::string tab,std::string desc,std::string key,double min,double max,double defaultval);
	double getOptionDouble(std::string key);
	void setOptionDouble(std::string key,double val);
	void registerOptionString(std::string tab,std::string desc,std::string key,std::string defaultval,bool is_url);
	std::string getOptionString(std::string key);
	void setOptionString(std::string key,std::string val);
	void registerOptionEnumInt(std::string tab,std::string desc,std::string key,std::vector<std::string> enumlist,int defaultval);
	int getOptionEnumInt(std::string key);
	std::string getOptionEnumIntOptName(std::string key);
	void setOptionEnumInt(std::string key,int val);
	void setOptionEnumIntOptName(std::string key,std::string valname);

	void load(const char* path);
private:
	std::map<std::string,qmpOptionR> options;
	std::vector<std::string> optionlist;
	QVariantMap settings;
};

#endif
