//CLI Midi file player based on libfluidsynth
//Midi file reading module
//Written by Chris Xiong, 2015
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cstdarg>
#include <algorithm>
#include "qmpmidiplay.hpp"
const char* GM1SysX={"\xF0\x7E\x7F\x09\x01\xF7"};
const char* GM2SysX={"\xF0\x7E\x7F\x09\x03\xF7"};
const char* GSSysEx={"\xF0\x41\x10\x42\x12\x40\x00\x7F\x00\x41\xF7"};
const char* XGSysEx={"\xF0\x43\x10\x4C\x00\x00\x7E\x00\xF7"};
void CSMFReader::error(int fatal,const char* format,...)
{
	va_list ap;
	va_start(ap,format);vfprintf(stderr,format,ap);va_end(ap);
	fprintf(stderr," at %#lx\n",ftell(f));
	if(fatal)throw std::runtime_error("fatal error");
}
uint32_t CSMFReader::readSW()
{
	byteread+=2;
	uint32_t ret=0;
	for(int i=0;i<2;++i){ret<<=8;ret|=((uint32_t)fgetc(f))&0xFF;}
	return ret;
}
uint32_t CSMFReader::readDW()
{
	byteread+=4;
	uint32_t ret=0;
	for(int i=0;i<4;++i){ret<<=8;ret|=((uint32_t)fgetc(f))&0xFF;}
	return ret;
}
uint32_t CSMFReader::readVL()
{
	uint32_t ret=0,t,c=0;
	do
	{
		t=fgetc(f);
		if(++c>4)error(1,"E: Variable length type overflow.");
		ret<<=7;ret|=(t&0x7F);
	}while(t&0x80);
	byteread+=c;
	return ret;
}
int CSMFReader::eventReader()//returns 0 if End of Track encountered
{
	uint32_t delta=readVL();curt+=delta;
	char type=fgetc(f);++byteread;uint32_t p1,p2;
	static char lasttype;eventdiscarded=0;
	if(!(type&0x80)){fseek(f,-1,SEEK_CUR);--byteread;type=lasttype;}
	switch(type&0xF0)
	{
		case 0x80://Note Off
			p1=fgetc(f);p2=fgetc(f);byteread+=2;
			ret->eventList.push_back(SEvent(curid,curt,type,p1,p2));
		break;
		case 0x90://Note On
			p1=fgetc(f);p2=fgetc(f);byteread+=2;
			if(p2)
				ret->eventList.push_back(SEvent(curid,curt,type,p1,p2));
			else
				ret->eventList.push_back(SEvent(curid,curt,(type&0x0F)|0x80,p1,p2));
		break;
		case 0xA0://Note Aftertouch
			p1=fgetc(f);p2=fgetc(f);byteread+=2;
			ret->eventList.push_back(SEvent(curid,curt,type,p1,p2));
		break;
		case 0xB0://Controller Change
			p1=fgetc(f);p2=fgetc(f);byteread+=2;
			ret->eventList.push_back(SEvent(curid,curt,type,p1,p2));
		break;
		case 0xC0://Patch Change
			p1=fgetc(f);++byteread;
			ret->eventList.push_back(SEvent(curid,curt,type,p1,0));
		break;
		case 0xD0://Channel Aftertouch
			p1=fgetc(f);++byteread;
			ret->eventList.push_back(SEvent(curid,curt,type,p1,0));
		break;
		case 0xE0://Pitch wheel
			p1=fgetc(f);p2=fgetc(f);byteread+=2;
			ret->eventList.push_back(SEvent(curid,curt,type,p1,p2));
		break;
		case 0xF0:
			if((type&0x0F)==0x0F)//Meta Event
			{
				char metatype=fgetc(f);++byteread;
				switch(metatype)
				{
					case 0x00://Sequence Number
						fgetc(f);fgetc(f);fgetc(f);
						byteread+=3;
					break;
					case 0x20://Channel Prefix
						fgetc(f);fgetc(f);byteread+=2;
					break;
					case 0x2F://End of Track
						fgetc(f);++byteread;
						return 0;
					break;
					case 0x51://Set Tempo
						p1=readDW();p1&=0x00FFFFFF;
						ret->eventList.push_back(SEvent(curid,curt,type,metatype,p1));
					break;
					case 0x54://SMTPE offset, not handled.
						fgetc(f);fgetc(f);fgetc(f);
						fgetc(f);fgetc(f);fgetc(f);
						byteread+=6;
					break;
					case 0x58://Time signature
						fgetc(f);++byteread;
						p1=readDW();
						ret->eventList.push_back(SEvent(curid,curt,type,metatype,p1));
					break;
					case 0x59://Key signature
						fgetc(f);++byteread;
						p1=readSW();
						ret->eventList.push_back(SEvent(curid,curt,type,metatype,p1));
					break;
					case 0x01:case 0x02:case 0x03:
					case 0x04:case 0x05:case 0x06:
					case 0x07:case 0x7F:default://text-like meta
					{
						uint32_t len=readVL(),c;char* str=NULL;
						if(len<=1024&&len>0)str=new char[len+8];
						for(c=0;c<len;++c)
						{
							++byteread;if(str)str[c]=fgetc(f);else fgetc(f);
						}
						if(str)str[c]='\0';ret->eventList.push_back(SEvent(curid,curt,type,metatype,0,str));
						if(str&&metatype==0x03&&!ret->title)
						{
							ret->title=new char[len+8];
							strcpy(ret->title,str);
						}
						if(str&&metatype==0x02&&!ret->copyright)
						{
							ret->copyright=new char[len+8];
							strcpy(ret->copyright,str);
						}
						if(len<=1024&&len>0)delete[] str;
					}
				}
			}
			else if((type&0x0F)==0x00||(type&0x0F)==0x07)//SysEx
			{
				uint32_t len=readVL(),c;char* str=NULL;
				str=new char[len+8];
				if((type&0x0F)==0x00)
				{
					str[0]=0xF0;++len;
					for(c=1;c<len;++c){++byteread;str[c]=fgetc(f);}
				}
				else for(c=0;c<len;++c){++byteread;str[c]=fgetc(f);}
				ret->eventList.push_back(SEvent(curid,curt,type,len,0,str));
				if(!strcmp(str,GM1SysX))ret->std=1;
				if(!strcmp(str,GM2SysX))ret->std=2;
				if(!strcmp(str,GSSysEx))ret->std=3;
				if(!strcmp(str,XGSysEx))ret->std=4;
				delete[] str;
			}
			else error(0,"W: Unknown event type %#x",type);
		break;
		default:
			error(0,"W: Unknown event type %#x",type);
	}
	lasttype=type;++curid;
	if(ret->eventList.size())
	{
		SEvent& le=ret->eventList[ret->eventList.size()-1];
		SEventCallBackData cbd(le.type,le.p1,le.p2,le.time);
		CMidiPlayer::getInstance()->callEventReaderCB(cbd);
	}
	return 1;
}
void CSMFReader::trackChunkReader()
{
	int chnklen=readDW();byteread=0;curt=0;curid=0;
	while(/*byteread<chnklen&&*/eventReader());
	if(byteread<chnklen)
	{
		error(0,"W: Extra bytes after EOT event.");
		while(byteread<chnklen){fgetc(f);++byteread;}
	}
	/*if(byteread>chnklen)
	{
		error(1,"E: Read past end of track.");
	}*/
}
void CSMFReader::headerChunkReader()
{
	int chnklen=readDW();byteread=0;
	if(chnklen<6)error(1,"E: Header chunk too short.");
	if(chnklen>6)error(0,"W: Header chunk length longer than expected. Ignoring extra bytes.");
	fmt=readSW();trk=readSW();ret->divs=readSW();
	if(ret->divs&0x8000)error(1,"E: SMTPE format is not supported.");
	for(;byteread<chnklen;++byteread){fgetc(f);}
}
int CSMFReader::chunkReader(int hdrXp)
{
	char hdr[6];
	fread(hdr,1,4,f);
	if(feof(f))error(1,"E: Unexpected EOF.");
	if(hdrXp)
	{
		if(!strncmp(hdr,"RIFF",4))
		{
			fseek(f,4,SEEK_CUR);
			fread(hdr,1,4,f);
			if(strncmp(hdr,"RMID",4)){error(1,"E: Wrong file type in RIFF container.");throw std::runtime_error("Wrong file type in RIFF container");}
			fseek(f,8,SEEK_CUR);
			fread(hdr,1,4,f);
		}
		if(strncmp(hdr,"MThd",4)){error(1,"E: Wrong MIDI header.");throw std::runtime_error("Wrong MIDI header");}
		else return headerChunkReader(),0;
	}
	else
		if(strncmp(hdr,"MTrk",4))
		{
			error(0,"W: Wrong track chunk header. Ignoring the whole chunk.");
			for(int chnklen=readDW();chnklen>0;--chnklen)fgetc(f);return 0;
		}
		else return trackChunkReader(),1;
}
CSMFReader::CSMFReader()
{
	f=NULL;
}
CMidiFile* CSMFReader::readFile(const char* fn)
{
	ret=new CMidiFile;
	ret->title=ret->copyright=NULL;ret->std=0;ret->valid=1;
	try
	{
		if(!(f=fopen(fn,"rb")))throw (fprintf(stderr,"E: file %s doesn't exist!\n",fn),std::runtime_error("File doesn't exist"));
		chunkReader(1);
		for(uint32_t i=0;i<trk;i+=chunkReader(0));
		fclose(f);f=NULL;
		std::sort(ret->eventList.begin(),ret->eventList.end());
	}
	catch(std::runtime_error&){fprintf(stderr,"E: %s is not a supported file.\n",fn);ret->valid=0;fclose(f);f=NULL;}
	return ret;
}
CSMFReader::~CSMFReader()
{
}

void CSMFReader::discardCurrentEvent()
{
	if(eventdiscarded)return;eventdiscarded=1;
	ret->eventList.pop_back();
}
void CSMFReader::commitEventChange(SEventCallBackData d)
{
	ret->eventList[ret->eventList.size()-1].time=d.time;
	ret->eventList[ret->eventList.size()-1].type=d.type;
	ret->eventList[ret->eventList.size()-1].p1=d.p1;
	ret->eventList[ret->eventList.size()-1].p2=d.p2;
}

void CMidiFileReaderCollection::destructFile(CMidiFile*& f)
{
	if(!f)return;
	if(f->copyright)delete[] f->copyright;
	if(f->title)delete[] f->title;
	delete f;
	f=NULL;
}
void CMidiFileReaderCollection::dumpFile()
{
	if(!file)return;
	std::vector<SEvent> &eventList=file->eventList;
	for(uint32_t i=0;i<eventList.size();++i)
		if(eventList[i].str.length())
			printf("type %x #%d @%d p1 %d p2 %d str %s\n",eventList[i].type,
			eventList[i].iid,eventList[i].time,eventList[i].p1,eventList[i].p2,eventList[i].str.c_str());
		else
			printf("type %x #%d @%d p1 %d p2 %d\n",eventList[i].type,
			eventList[i].iid,eventList[i].time,eventList[i].p1,eventList[i].p2);
}
CMidiFileReaderCollection::CMidiFileReaderCollection()
{
	file=NULL;readers.clear();currentReader=NULL;
	registerReader(new CSMFReader(),"Default SMF Reader");
}
CMidiFileReaderCollection::~CMidiFileReaderCollection()
{
	if(file)destructFile(file);
	delete readers[0].first;
}
void CMidiFileReaderCollection::registerReader(IMidiFileReader* reader,std::string name)
{
	for(unsigned i=0;i<readers.size();++i)
		if(readers[i].second==name)return;
	readers.push_back(std::make_pair(reader,name));
}
void CMidiFileReaderCollection::unregisterReader(std::string name)
{
	for(auto i=readers.begin();i!=readers.end();++i)
	if(i->second==name)
	{
		readers.erase(i);
		return;
	}
}
void CMidiFileReaderCollection::readFile(const char* fn)
{
	if(file)destructFile(file);
	for(unsigned i=0;i<readers.size();++i)
	{
		currentReader=readers[i].first;
		CMidiPlayer::getInstance()->notes=0;
		CMidiFile* t=readers[i].first->readFile(fn);
		if(t->valid){file=t;break;}
		else destructFile(t);
	}
	currentReader=NULL;
	if(file)
	maxtk=file->eventList[file->eventList.size()-1].time;
}
void CMidiFileReaderCollection::destructFile()
{destructFile(file);}
IMidiFileReader* CMidiFileReaderCollection::getCurrentReader()
{return currentReader;}
bool CMidiFileReaderCollection::isValid()
{return file&&file->valid;}
const char* CMidiFileReaderCollection::getTitle()
{return file?file->title:NULL;}
const char* CMidiFileReaderCollection::getCopyright()
{return file?file->copyright:NULL;}
const SEvent* CMidiFileReaderCollection::getEvent(uint32_t id)
{return file?&(file->eventList[id]):NULL;}
uint32_t CMidiFileReaderCollection::getEventCount()
{return file?file->eventList.size():0;}
uint32_t CMidiFileReaderCollection::getDivision()
{return file?file->divs:0;}
uint32_t CMidiFileReaderCollection::getMaxTick()
{return maxtk;}
uint32_t CMidiFileReaderCollection::getStandard()
{return file?file->std:0;}
