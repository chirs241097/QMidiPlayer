#include <QScopedPointer>
#include <QSettings>

#include <cstdio>

#include "qmpsettingsro.hpp"

qmpSettingsRO::qmpSettingsRO()
{
}

void qmpSettingsRO::registerOptionInt(std::string tab,std::string desc,std::string key,int min,int max,int defaultval)
{
	Q_UNUSED(tab)
	optionlist.push_back(key);
	options[key]=qmpOptionR(desc,qmpOptionR::ParameterType::parameter_int,defaultval,min,max);
}
int qmpSettingsRO::getOptionInt(std::string key)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOptionR::ParameterType::parameter_int)
		return settings.value(QString(key.c_str()),options[key].defaultval).toInt();
	return options[key].defaultval.toInt();
}
void qmpSettingsRO::setOptionInt(std::string key,int val)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOptionR::ParameterType::parameter_int)
		settings.insert(QString(key.c_str()),val);
}

void qmpSettingsRO::registerOptionUint(std::string tab,std::string desc,std::string key,unsigned min,unsigned max,unsigned defaultval)
{
	Q_UNUSED(tab)
	optionlist.push_back(key);
	options[key]=qmpOptionR(desc,qmpOptionR::ParameterType::parameter_uint,defaultval,min,max);
}
unsigned qmpSettingsRO::getOptionUint(std::string key)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOptionR::ParameterType::parameter_uint)
		return settings.value(QString(key.c_str()),options[key].defaultval).toUInt();
	return options[key].defaultval.toUInt();
}
void qmpSettingsRO::setOptionUint(std::string key,unsigned val)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOptionR::ParameterType::parameter_uint)
		settings.insert(QString(key.c_str()),val);
}

void qmpSettingsRO::registerOptionBool(std::string tab,std::string desc,std::string key,bool defaultval)
{
	Q_UNUSED(tab)
	optionlist.push_back(key);
	options[key]=qmpOptionR(desc,qmpOptionR::ParameterType::parameter_bool,defaultval);
}
bool qmpSettingsRO::getOptionBool(std::string key)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOptionR::ParameterType::parameter_bool)
		return settings.value(QString(key.c_str()),options[key].defaultval).toBool();
	return options[key].defaultval.toBool();
}
void qmpSettingsRO::setOptionBool(std::string key,bool val)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOptionR::ParameterType::parameter_bool)
		settings.insert(QString(key.c_str()),val);
}

void qmpSettingsRO::registerOptionDouble(std::string tab,std::string desc,std::string key,double min,double max,double defaultval)
{
	Q_UNUSED(tab)
	optionlist.push_back(key);
	options[key]=qmpOptionR(desc,qmpOptionR::ParameterType::parameter_double,defaultval,min,max);
}
double qmpSettingsRO::getOptionDouble(std::string key)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOptionR::ParameterType::parameter_double)
		return settings.value(QString(key.c_str()),options[key].defaultval).toDouble();
	return options[key].defaultval.toDouble();
}
void qmpSettingsRO::setOptionDouble(std::string key,double val)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOptionR::ParameterType::parameter_double)
		settings.insert(QString(key.c_str()),val);
}

void qmpSettingsRO::registerOptionString(std::string tab,std::string desc,std::string key,std::string defaultval,bool is_url)
{
	Q_UNUSED(tab)
	optionlist.push_back(key);
	options[key]=qmpOptionR(desc,
		is_url?qmpOptionR::ParameterType::parameter_url:qmpOptionR::ParameterType::parameter_str,
		QString(defaultval.c_str()));
}
std::string qmpSettingsRO::getOptionString(std::string key)
{
	if(options.find(key)!=options.end()&&
		(options[key].type==qmpOptionR::ParameterType::parameter_str||options[key].type==qmpOptionR::ParameterType::parameter_url))
		return settings.value(QString(key.c_str()),options[key].defaultval).toString().toStdString();
	return options[key].defaultval.toString().toStdString();
}
void qmpSettingsRO::setOptionString(std::string key,std::string val)
{
	if(options.find(key)!=options.end()&&
		(options[key].type==qmpOptionR::ParameterType::parameter_str||options[key].type==qmpOptionR::ParameterType::parameter_url))
		settings.insert(QString(key.c_str()),QString(val.c_str()));
}

void qmpSettingsRO::registerOptionEnumInt(std::string tab,std::string desc,std::string key,std::vector<std::string> enumlist,int defaultval)
{
	Q_UNUSED(tab)
	optionlist.push_back(key);
	options[key]=qmpOptionR(desc,qmpOptionR::ParameterType::parameter_enum,defaultval);
	options[key].enumlist=enumlist;
}
int qmpSettingsRO::getOptionEnumInt(std::string key)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOptionR::ParameterType::parameter_enum)
	{
		std::string curitm=settings.value(QString(key.c_str()),options[key].defaultval).toString().toStdString();
		auto curidx=std::find(options[key].enumlist.begin(),options[key].enumlist.end(),curitm);
		if(curidx!=options[key].enumlist.end())
			return static_cast<int>(curidx-options[key].enumlist.begin());
		else
		{
			return options[key].defaultval.toInt();
		}
	}
	return options[key].defaultval.toInt();
}
std::string qmpSettingsRO::getOptionEnumIntOptName(std::string key)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOptionR::ParameterType::parameter_enum)
	{
		std::string curitm=settings.value(QString(key.c_str()),options[key].defaultval).toString().toStdString();
		auto curidx=std::find(options[key].enumlist.begin(),options[key].enumlist.end(),curitm);
		if(curidx!=options[key].enumlist.end())
			return curitm;
		else
		{
			return options[key].enumlist[static_cast<size_t>(options[key].defaultval.toInt())];
		}
	}
	return options[key].enumlist[static_cast<size_t>(options[key].defaultval.toInt())];
}
void qmpSettingsRO::setOptionEnumInt(std::string key,int val)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOptionR::ParameterType::parameter_enum)
	{
		if(static_cast<size_t>(val)<options[key].enumlist.size())
			settings.insert(QString(key.c_str()),QString(options[key].enumlist[static_cast<size_t>(val)].c_str()));
	}
}
void qmpSettingsRO::setOptionEnumIntOptName(std::string key,std::string valname)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOptionR::ParameterType::parameter_enum)
	{
		auto curidx=std::find(options[key].enumlist.begin(),options[key].enumlist.end(),valname);
		if(curidx!=options[key].enumlist.end())
			settings.insert(QString(key.c_str()),QString(valname.c_str()));
	}
}

void qmpSettingsRO::load(const char *path)
{
	QScopedPointer<QSettings> qsettings(new QSettings(path,QSettings::Format::IniFormat));
	settings.clear();
	for(QString&k:qsettings->allKeys())
	{
		settings.insert(k,qsettings->value(k));
	}
}

void qmpSettingsRO::setopt(std::string key, std::string val)
{
	settings.insert(QString(key.c_str()),QString(val.c_str()));
	if(key.find("Visualization/")!=0)
		settings.insert("Visualization/"+QString(key.c_str()),QString(val.c_str()));
}

void qmpSettingsRO::listopt()
{
	for(auto&k:optionlist)
	{
		printf("Option key: %s\n",k.c_str());
		if(options[k].desc.length())
			printf("Description: %s\n",options[k].desc.c_str());
		switch(options[k].type)
		{
			case qmpOptionR::ParameterType::parameter_int:
				printf("Type: int\n");
				printf("Range: [%d,%d]\n",options[k].minv.toInt(),options[k].maxv.toInt());
				printf("Default value: %d\n",options[k].defaultval.toInt());
			break;
			case qmpOptionR::ParameterType::parameter_uint:
				printf("Type: uint\n");
				printf("Range: [%u,%u]\n",options[k].minv.toUInt(),options[k].maxv.toUInt());
				printf("Default value: %u\n",options[k].defaultval.toUInt());
			break;
			case qmpOptionR::ParameterType::parameter_double:
				printf("Type: double\n");
				printf("Range: [%.2f,%.2f]\n",options[k].minv.toDouble(),options[k].maxv.toDouble());
				printf("Default value: %.f2\n",options[k].defaultval.toDouble());
			break;
			case qmpOptionR::ParameterType::parameter_bool:
				printf("Type: bool\n");
				printf("Default value: %s\n",options[k].defaultval.toBool()?"true":"false");
			break;
			case qmpOptionR::ParameterType::parameter_str:
				printf("Type: str\n");
				printf("Default value: %s\n",options[k].defaultval.toString().toStdString().c_str());
			break;
			case qmpOptionR::ParameterType::parameter_url:
				printf("Type: url\n");
				printf("Default value: %s\n",options[k].defaultval.toString().toStdString().c_str());
			break;
			case qmpOptionR::ParameterType::parameter_enum:
				printf("Type: enum\n");
				printf("Possible values: ");
				for(size_t i=0;i<options[k].enumlist.size();++i)
					printf("%s%s",options[k].enumlist[i].c_str(),i==options[k].enumlist.size()-1?"\n":", ");
				printf("Default value: %s\n",options[k].enumlist[options[k].defaultval.toInt()].c_str());
			break;
			default:
				printf("Type: unknown\n");
		}
		puts("");
	}
}
