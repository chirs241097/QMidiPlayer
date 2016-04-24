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
		virtual ~qmpPluginIntf(){}
		virtual void init(){}
		virtual void deinit(){}
		virtual const char* pluginGetName(){return "";}
		virtual const char* pluginGetVersion(){return "";}
};
class qmpVisualizationIntf
{
	public:
		virtual void show();
		virtual void close();
};
extern "C"{
class qmpPluginAPI
{
	public:
		uint32_t getDivision();
		uint32_t getRawTempo();
		double getRealTempo();
		uint32_t getTimeSig();
		int getKeySig();
		uint32_t getNoteCount();
		uint32_t getCurrentPolyphone();
		uint32_t getMaxPolyphone();
		uint32_t getCurrentTimeStamp();
		int registerVisualizationIntf(qmpVisualizationIntf* i);
		void unregisterVisualizationIntf(int intfhandle);
		int registerEventReaderIntf(IMidiCallBack* cb,void* userdata);
		void unregisterEventReaderIntf(int intfhandle);
		int registerEventHandlerIntf(IMidiCallBack* cb,void* userdata);
		void unregisterEventHandlerIntf(int intfhandle);
		void registerOptionInt(std::string desc,std::string key,int defaultval);
		int getOptionInt(std::string key);
		void registerOptionDouble(std::string desc,std::string key,double defaultval);
		double getOptionDouble(std::string key);
		void registerOptionString(std::string desc,std::string key,std::string defaultval);
		std::string getOptionString(std::string key);
};
}
typedef qmpPluginIntf*(*qmpPluginEntry)(qmpPluginAPI*);
#endif // QMPCOREPUBLIC_H
