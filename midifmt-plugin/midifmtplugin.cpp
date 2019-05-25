#include <cstring>
#include <algorithm>
#include <stdexcept>
#include "midifmtplugin.hpp"
qmpPluginAPI* qmpMidiFmtPlugin::api=nullptr;

uint32_t CMidiStreamReader::readDWLE()
{
	uint32_t ret=0;
	for(uint32_t i=0;i<4;++i)ret|=((uint32_t)fgetc(f))<<(i<<3);
	return ret;
}
bool CMidiStreamReader::RIFFHeaderReader()
{
	char hdr[9];
	fread(hdr,1,4,f);
	if(strncmp(hdr,"RIFF",4))return false;
	fseek(f,4,SEEK_CUR);
	fread(hdr,1,8,f);
	if(strncmp(hdr,"MIDSfmt ",8))return false;
	if(readDWLE()!=0x0C)return false;
	ret->divs=readDWLE();
	readDWLE();
	fmt=readDWLE();
	return true;
}
bool CMidiStreamReader::midsBodyReader()
{
	char buf[9];
	fread(buf,1,4,f);
	if(strncmp(buf,"data",4))return false;
	readDWLE();//size
	uint32_t cblocks=readDWLE();
	uint32_t curid=0,cts=0;
	for(uint32_t i=0;i<cblocks;++i)
	{
		readDWLE();
		uint32_t blocksz=readDWLE(),cpos=ftell(f);
		while(ftell(f)-cpos<blocksz)
		{
			cts+=readDWLE();
			if(!(fmt&1))readDWLE();
			uint32_t e=readDWLE();
			SEvent ev;
			if(e>>24==1)//set tempo
			{
				char s[3]={'\0'};
				for(int i=0;i<3;++i)
					s[i]=(e>>(8*(2-i)))&0xff;
				ev=SEvent(curid,cts,0xFF,0x51,0);
				ev.str=std::string(s,3);
			}
			else if(e>>24==0)//midishortmsg
			ev=SEvent(curid,cts,e&0xFF,(e>>8)&0xFF,(e>>16)&0xFF);
			else return false;
			ret->tracks.back().appendEvent(ev);eventdiscarded=0;
			qmpMidiFmtPlugin::api->callEventReaderCB(ev);
			if(eventdiscarded)ret->tracks.back().eventList.pop_back();
			++curid;
		}
	}
	return true;
}
CMidiFile* CMidiStreamReader::readFile(const char *fn)
{
	ret=new CMidiFile;
	ret->title=ret->copyright=nullptr;ret->std=0;ret->valid=1;
	ret->tracks.push_back(CMidiTrack());
	try
	{
		if(!(f=fopen(fn,"rb")))throw std::runtime_error("File doesn't exist");
		if(!RIFFHeaderReader())throw std::runtime_error("Wrong RIFF header");
		if(!midsBodyReader())throw std::runtime_error("MIDS data error");
	}catch(std::runtime_error& e)
	{
		fprintf(stderr,"CMidiStreamReader E: %s is not a supported file. Cause: %s.\n",fn,e.what());
		ret->valid=0;if(f)fclose(f);f=nullptr;
	}
	return ret;
}
void CMidiStreamReader::discardCurrentEvent()
{
	eventdiscarded=1;
}
void CMidiStreamReader::commitEventChange(SEvent d)
{
	ret->tracks.back().eventList.back().time=d.time;
	ret->tracks.back().eventList.back().type=d.type;
	ret->tracks.back().eventList.back().p1=d.p1;
	ret->tracks.back().eventList.back().p2=d.p2;
}
CMidiStreamReader::CMidiStreamReader()
{
	ret=nullptr;f=nullptr;
}
CMidiStreamReader::~CMidiStreamReader()
{
}

qmpMidiFmtPlugin::qmpMidiFmtPlugin(qmpPluginAPI *_api)
{api=_api;}
qmpMidiFmtPlugin::~qmpMidiFmtPlugin()
{api=nullptr;}
void qmpMidiFmtPlugin::init()
{
	api->registerFileReader(mdsreader=new CMidiStreamReader,"MIDS reader");
}
void qmpMidiFmtPlugin::deinit()
{
	api->unregisterFileReader("MIDS reader");
	delete mdsreader;
}
const char* qmpMidiFmtPlugin::pluginGetName()
{return "QMidiPlayer extra midi formats plugin";}
const char* qmpMidiFmtPlugin::pluginGetVersion()
{return "0.8.6";}
