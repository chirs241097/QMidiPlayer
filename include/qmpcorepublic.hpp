#ifndef QMPCOREPUBLIC_H
#define QMPCOREPUBLIC_H
#include <cstdint>
#include <string>
struct SEventCallBackData
{
	uint32_t type,p1,p2;
	SEventCallBackData(uint32_t _t,uint32_t _p1,uint32_t _p2){type=_t;p1=_p1;p2=_p2;}
};
class IMidiCallBack
{
	public:
		IMidiCallBack(){}
		virtual void callBack(void* callerdata,void* userdata)=0;
		virtual ~IMidiCallBack(){}
};
class qmpPluginIntf
{
	public:
		virtual void init();
		virtual void deinit();
		virtual const char* pluginGetName();
		virtual const char* pluginGetVersion();
};
class qmpVisualizationIntf
{
	public:
		virtual void show();
		virtual void close();
};
class qmpPluginAPI
{
	public:
		uint32_t getDivision();
		uint32_t getRawTempo();
		double getRealTempo();
		uint32_t getTimeSig();
		uint32_t getKeySig();
		uint32_t getNoteCount();
		uint32_t getCurrentPolyphone();
		uint32_t getMaxPolyphone();
		uint32_t getCurrentTimeStamp();
		int registerVisualizationIntf(qmpVisualizationIntf* i);
		void unregisterVisualizationIntf(int intfhandle);
		int registerEventReadHandlerIntf(IMidiCallBack* cb,void* userdata);
		void unregisterEventReadHandlerIntf(IMidiCallBack* cb,void* userdata);
		int registerEventHandlerIntf(IMidiCallBack* cb,void* userdata);
		void unregisterEventHandlerIntf(int intfhandle);
		void registerOptionInt(std::string desc,std::string key,int defaultval);
		int getOptionInt(std::string key);
		void registerOptionDouble(std::string desc,std::string key,double defaultval);
		double getOptionDouble(std::string key);
		void registerOptionString(std::string desc,std::string key,std::string defaultval);
		std::string getOptionString(std::string key);
};
typedef qmpPluginIntf*(*qmpPluginEntry)(qmpPluginAPI*);
#endif // QMPCOREPUBLIC_H
