#include <QScopedPointer>
#include <QSettings>

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
	if(options.find(key)==options.end())
	{
		std::string nkey="Visualization/"+key;
		if(options.find(nkey)==options.end())
		{
			qDebug("invalid option key %s",key.c_str());
			return;
		}
		else key=nkey;
	}
	char *rptr;
	switch(options[key].type)
	{
		case qmpOptionR::ParameterType::parameter_int:
		{
			long long v=strtoll(val.c_str(),&rptr,10);
			if(rptr==val.c_str()||v>INT_MAX||v<INT_MIN)
				qDebug("invalid value for option %s",key.c_str());
			setOptionInt(key,static_cast<int>(v));
		}
		break;
		case qmpOptionR::ParameterType::parameter_uint:
		{
			long long v=strtoll(val.c_str(),&rptr,10);
			if(rptr==val.c_str()||v>UINT32_MAX||v<0)
				qDebug("invalid value for option %s",key.c_str());
			setOptionUint(key,static_cast<uint32_t>(v));
		}
		break;
		case qmpOptionR::ParameterType::parameter_double:
		{
			errno=0;
			double v=strtod(val.c_str(),&rptr);
			if(rptr==val.c_str()||errno)
				qDebug("invalid value for option %s",key.c_str());
			setOptionDouble(key,v);
		}
		break;
		case qmpOptionR::ParameterType::parameter_bool:
		{
			if(val!="true"&&val!="false")
				qDebug("invalid value for option %s",key.c_str());
			setOptionBool(key,val=="true");
		}
		break;
		case qmpOptionR::ParameterType::parameter_str:
		case qmpOptionR::ParameterType::parameter_url:
			setOptionString(key,val);
		break;
		case qmpOptionR::ParameterType::parameter_enum:
			setOptionEnumIntOptName(key,val);
		break;
		default:
		break;
	}
}
