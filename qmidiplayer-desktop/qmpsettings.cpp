#include "qmpsettings.hpp"
#include <QFile>
#include <QStandardPaths>

#define QMP_CONFIGURATION_FILE_REV 1

QSettings* qmpSettings::settings=nullptr;
qmpSettings::qmpSettings()
{
	qRegisterMetaTypeStreamOperators<QPair<QString,QString>>();
	QString confpath=QStandardPaths::writableLocation(QStandardPaths::StandardLocation::ConfigLocation)+QString("/qmprc");
	settings=new QSettings(confpath,QSettings::IniFormat);
	if(settings->value("ConfigurationFileRevision").toInt()!=QMP_CONFIGURATION_FILE_REV&&
		QFile::exists(confpath))
	{
		qWarning("Your current configuration file is not compatible with this version of QMidiPlayer. "
				 "QMidiPlayer will start with its default configuration. A backup of the old configuration "
				 "is automatically saved as qmprc.old.");
		QFile::remove(confpath+".old");
		QFile::copy(confpath,confpath+".old");
		settings->clear();
		settings->setValue("ConfigurationFileRevision",QMP_CONFIGURATION_FILE_REV);
	}
}

qmpSettings::~qmpSettings()
{
	delete settings;
	settings=nullptr;
}

void qmpSettings::registerOptionInt(std::string tab,std::string desc,std::string key,int min,int max,int defaultval)
{
	optionlist.push_back(key);
	options[key]=qmpOption(tab,desc,qmpOption::ParameterType::parameter_int,nullptr,defaultval,min,max);
}
int qmpSettings::getOptionInt(std::string key)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOption::ParameterType::parameter_int)
		return settings->value(QString(key.c_str()),options[key].defaultval).toInt();
	qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
	return options[key].defaultval.toInt();
}
void qmpSettings::setOptionInt(std::string key,int val)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOption::ParameterType::parameter_int)
		settings->setValue(QString(key.c_str()),val);
	else
		qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
	//call qmpSettingsWindow::load(key)?
}

void qmpSettings::registerOptionUint(std::string tab,std::string desc,std::string key,unsigned min, unsigned max,unsigned defaultval)
{
	optionlist.push_back(key);
	options[key]=qmpOption(tab,desc,qmpOption::ParameterType::parameter_uint,nullptr,defaultval,min,max);
}
unsigned qmpSettings::getOptionUint(std::string key)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOption::ParameterType::parameter_uint)
		return settings->value(QString(key.c_str()),options[key].defaultval).toUInt();
	qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
	return options[key].defaultval.toUInt();
}
void qmpSettings::setOptionUint(std::string key,unsigned val)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOption::ParameterType::parameter_uint)
		settings->setValue(QString(key.c_str()),val);
	else
		qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
}

void qmpSettings::registerOptionBool(std::string tab,std::string desc,std::string key,bool defaultval)
{
	optionlist.push_back(key);
	options[key]=qmpOption(tab,desc,qmpOption::ParameterType::parameter_bool,nullptr,defaultval);
}
bool qmpSettings::getOptionBool(std::string key)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOption::ParameterType::parameter_bool)
		return settings->value(QString(key.c_str()),options[key].defaultval).toBool();
	qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
	return options[key].defaultval.toBool();
}
void qmpSettings::setOptionBool(std::string key,bool val)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOption::ParameterType::parameter_bool)
		settings->setValue(QString(key.c_str()),val);
	else
		qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
}

void qmpSettings::registerOptionDouble(std::string tab, std::string desc, std::string key, double min, double max, double defaultval)
{
	optionlist.push_back(key);
	options[key]=qmpOption(tab,desc,qmpOption::ParameterType::parameter_double,nullptr,defaultval,min,max);
}
double qmpSettings::getOptionDouble(std::string key)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOption::ParameterType::parameter_double)
		return settings->value(QString(key.c_str()),options[key].defaultval).toDouble();
	qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
	return options[key].defaultval.toDouble();
}
void qmpSettings::setOptionDouble(std::string key,double val)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOption::ParameterType::parameter_double)
		settings->setValue(QString(key.c_str()),val);
	else
		qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
}

void qmpSettings::registerOptionString(std::string tab,std::string desc,std::string key,std::string defaultval,bool is_url)
{
	optionlist.push_back(key);
	options[key]=qmpOption(tab,desc,
				is_url?qmpOption::ParameterType::parameter_url:qmpOption::ParameterType::parameter_str,
						   nullptr,QString(defaultval.c_str()));
}
std::string qmpSettings::getOptionString(std::string key)
{
	if(options.find(key)!=options.end()&&
		(options[key].type==qmpOption::ParameterType::parameter_str||options[key].type==qmpOption::ParameterType::parameter_url))
		return settings->value(QString(key.c_str()),options[key].defaultval).toString().toStdString();
	qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
	return options[key].defaultval.toString().toStdString();
}
void qmpSettings::setOptionString(std::string key,std::string val)
{
	if(options.find(key)!=options.end()&&
		(options[key].type==qmpOption::ParameterType::parameter_str||options[key].type==qmpOption::ParameterType::parameter_url))
		settings->setValue(QString(key.c_str()),QString(val.c_str()));
	else
		qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
}

void qmpSettings::registerOptionEnumInt(std::string tab,std::string desc,std::string key,std::vector<std::string> enumlist,int defaultval)
{
	optionlist.push_back(key);
	options[key]=qmpOption(tab,desc,qmpOption::ParameterType::parameter_enum,nullptr,defaultval);
	options[key].enumlist=enumlist;
}
int qmpSettings::getOptionEnumInt(std::string key)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOption::ParameterType::parameter_enum)
	{
		std::string curitm=settings->value(QString(key.c_str()),options[key].defaultval).toString().toStdString();
		auto curidx=std::find(options[key].enumlist.begin(),options[key].enumlist.end(),curitm);
		if(curidx!=options[key].enumlist.end())
			return static_cast<int>(curidx-options[key].enumlist.begin());
		else
		{
			qWarning("Invalid value set for option \"%s\".",key.c_str());
			return options[key].defaultval.toInt();
		}
	}
	qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
	return options[key].defaultval.toInt();
}

std::string qmpSettings::getOptionEnumIntOptName(std::string key)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOption::ParameterType::parameter_enum)
	{
		std::string curitm=settings->value(QString(key.c_str()),options[key].defaultval).toString().toStdString();
		auto curidx=std::find(options[key].enumlist.begin(),options[key].enumlist.end(),curitm);
		if(curidx!=options[key].enumlist.end())
			return curitm;
		else
		{
			qWarning("Invalid value set for option \"%s\".",key.c_str());
			return options[key].enumlist[static_cast<size_t>(options[key].defaultval.toInt())];
		}
	}
	qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
	return options[key].enumlist[static_cast<size_t>(options[key].defaultval.toInt())];
}
void qmpSettings::setOptionEnumInt(std::string key,int val)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOption::ParameterType::parameter_enum)
	{
		if(static_cast<size_t>(val)<options[key].enumlist.size())
			settings->setValue(QString(key.c_str()),QString(options[key].enumlist[static_cast<size_t>(val)].c_str()));
		else
			qWarning("Trying to set invalid value for option \"%s\".",key.c_str());
	}
	else
		qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
}

void qmpSettings::setOptionEnumIntOptName(std::string key,std::string valname)
{
	if(options.find(key)!=options.end()&&options[key].type==qmpOption::ParameterType::parameter_enum)
	{
		auto curidx=std::find(options[key].enumlist.begin(),options[key].enumlist.end(),valname);
		if(curidx!=options[key].enumlist.end())
			settings->setValue(QString(key.c_str()),QString(valname.c_str()));
		else
			qWarning("Trying to set invalid value for option \"%s\".",key.c_str());
	}
	else
		qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
}

void qmpSettings::registerOptionCustom(std::string tab,std::string desc,std::string key,void* widget, void* defaultval,std::function<void*()> save_func,std::function<void(void*)> load_func)
{
	optionlist.push_back(key);
	options[key]=qmpOption(tab,desc,qmpOption::parameter_custom,
						static_cast<QWidget*>(widget),
						*static_cast<QVariant*>(defaultval),
						QVariant(),QVariant(),save_func,load_func);
}
void* qmpSettings::getOptionCustom(std::string key)
{
	if(options.find(key)!=options.end()||options[key].type!=qmpOption::ParameterType::parameter_custom)
		return static_cast<void*>(new QVariant(settings->value(QString(key.c_str()),options[key].defaultval)));
	qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
	return nullptr;
}
void qmpSettings::setOptionCustom(std::string key,void *val)
{
	if(options.find(key)!=options.end()||options[key].type!=qmpOption::ParameterType::parameter_custom)
		settings->setValue(QString(key.c_str()),*static_cast<QVariant*>(val));
	else
		qWarning("Unregistered option or mismatching option type: %s.",key.c_str());
}

void qmpSettings::setOptionRaw(QString key,QVariant val)
{
	settings->setValue(key,val);
}

QVariant qmpSettings::getOptionRaw(QString key,QVariant defval)
{
	return settings->value(key,defval);
}

QDataStream &operator<<(QDataStream &out,const QPair<QString,QString> &o)
{
	out<<o.first<<o.second;
	return out;
}

QDataStream &operator>>(QDataStream &in,QPair<QString,QString> &o)
{
	in>>o.first>>o.second;
	return in;
}
