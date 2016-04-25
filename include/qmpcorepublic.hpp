#ifndef QMPCOREPUBLIC_H
#define QMPCOREPUBLIC_H
#include <cstdint>
#include <string>
struct SEventCallBackData
{
	uint32_t time,type,p1,p2;
	SEventCallBackData(uint32_t _t,uint32_t _p1,uint32_t _p2,uint32_t _tm){type=_t;p1=_p1;p2=_p2;time=_tm;}
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
		qmpPluginIntf(){}
		virtual ~qmpPluginIntf(){}
		virtual void init(){}
		virtual void deinit(){}
		virtual const char* pluginGetName(){return "";}
		virtual const char* pluginGetVersion(){return "";}
};
class qmpVisualizationIntf
{
	public:
		qmpVisualizationIntf(){}
		virtual void show()=0;
		virtual void close()=0;
		virtual void start()=0;
		virtual void stop()=0;
		virtual void pause()=0;
		virtual void reset()=0;
		virtual ~qmpVisualizationIntf(){}
};
extern "C"{
class qmpPluginAPI
{
	public:
		virtual uint32_t getDivision();
		virtual uint32_t getRawTempo();
		virtual double getRealTempo();
		virtual uint32_t getTimeSig();
		virtual int getKeySig();
		virtual uint32_t getNoteCount();
		virtual uint32_t getCurrentPolyphone();
		virtual uint32_t getMaxPolyphone();
		virtual uint32_t getCurrentTimeStamp();
		virtual int registerVisualizationIntf(qmpVisualizationIntf* i);
		virtual void unregisterVisualizationIntf(int intfhandle);
		virtual int registerEventReaderIntf(IMidiCallBack* cb,void* userdata);
		virtual void unregisterEventReaderIntf(int intfhandle);
		virtual int registerEventHandlerIntf(IMidiCallBack* cb,void* userdata);
		virtual void unregisterEventHandlerIntf(int intfhandle);
		virtual void registerOptionInt(std::string desc,std::string key,int defaultval);
		virtual int getOptionInt(std::string key);
		virtual void registerOptionDouble(std::string desc,std::string key,double defaultval);
		virtual double getOptionDouble(std::string key);
		virtual void registerOptionString(std::string desc,std::string key,std::string defaultval);
		virtual std::string getOptionString(std::string key);
};
}
typedef qmpPluginIntf*(*qmpPluginEntry)(qmpPluginAPI*);
#endif // QMPCOREPUBLIC_H
