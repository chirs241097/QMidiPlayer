#ifndef QMPSETTINGS_H
#define QMPSETTINGS_H

#include <functional>
#include <string>
#include <map>
#include <vector>
#include <QWidget>
#include <QSettings>

struct qmpOption
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

	std::string tab;
	std::string desc;
	ParameterType type;
	QWidget* widget;
	QVariant defaultval,minv,maxv;
	std::function<void*()> save_func;
	std::function<void(void*)> load_func;
	std::vector<std::string> enumlist;

	qmpOption():widget(nullptr){}
	qmpOption(std::string _tab,std::string _desc,
			ParameterType _t,QWidget* _w=nullptr,
			QVariant _def=QVariant(),QVariant _min=QVariant(),QVariant _max=QVariant(),
			std::function<void*()> _save=nullptr,std::function<void(void*)> _load=nullptr):
		tab(_tab),
		desc(_desc),
		type(_t),
		widget(_w),
		defaultval(_def),
		minv(_min),
		maxv(_max),
		save_func(_save),
		load_func(_load){}
};
class qmpSettings
{
	public:
		qmpSettings();
		~qmpSettings();
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
		void registerOptionCustom(std::string tab,std::string desc,std::string key,void* widget,void* defaultval,std::function<void*()> save_func,std::function<void(void*)> load_func);
		void* getOptionCustom(std::string key);
		void setOptionCustom(std::string key,void* val);

		void setOptionRaw(QString key,QVariant val);
		QVariant getOptionRaw(QString key,QVariant defval=QVariant());

	private:
		static QSettings *settings;
		std::map<std::string,qmpOption> options;
		std::vector<std::string> optionlist;

	friend class qmpSettingsWindow;
};

#endif
