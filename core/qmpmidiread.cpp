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
static const char* GM1SysX="\xF0\x7E\x7F\x09\x01\xF7";
static const char* GM2SysX="\xF0\x7E\x7F\x09\x03\xF7";
static const char* GSSysEx="\xF0\x41\x10\x42\x12\x40\x00\x7F\x00\x41\xF7";
static const char* XGSysEx="\xF0\x43\x10\x4C\x00\x00\x7E\x00\xF7";
#define assert(x) if(!(x))this->error(false,"assertion failure @ qmpmidiread.cpp:%d",__LINE__)
void CSMFReader::error(int fatal,const char* format,...)
{
	va_list ap;char buf[1024],bufr[1024];
	va_start(ap,format);vsnprintf(buf,1024,format,ap);va_end(ap);
	snprintf(bufr,1024,"%s at %#lx",buf,ftell(f));
	if(fatal)throw std::runtime_error(bufr);
	else fprintf(stderr,"CSMFReader W: %s.\n",bufr);
}
uint8_t CSMFReader::read_u8()
{
	uint8_t ret=0;
	int t=fgetc(f);
	if(!~t)error(1,"Unexpected EOF");
	ret=(uint8_t)t;
	return ret;
}
uint16_t CSMFReader::read_u16()
{
	uint16_t ret=0;
	size_t sz=fread(&ret,2,1,f);
	if(sz<1)error(1,"Unexpected EOF");
#if defined(_MSC_VER)&&defined(_WIN32)
	ret=_byteswap_ushort(ret);
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
	ret=__builtin_bswap16(ret);
#endif
	return ret;
}
uint32_t CSMFReader::read_u32()
{
	uint32_t ret=0;
	size_t sz=fread(&ret,4,1,f);
	if(sz<1)error(1,"Unexpected EOF");
#if defined(_MSC_VER)&&defined(_WIN32)
	ret=_byteswap_ulong(ret);
#elif __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
	ret=__builtin_bswap32(ret);
#endif
	return ret;
}
uint32_t CSMFReader::read_varlen()
{
	uint32_t ret=0,c=0;
	int t;
	do
	{
		t=fgetc(f);
		if(!~t)error(1,"Unexpected EOF");
		if(++c>4)error(1,"Variable length type overflow");
		ret<<=7;ret|=(t&0x7F);
	}while(t&0x80);
	return ret;
}
int CSMFReader::read_event()//returns 0 if End of Track encountered
{
	uint32_t delta=read_varlen();curt+=delta;
	uint8_t type=read_u8();uint32_t p1,p2;
	static uint8_t lasttype;eventdiscarded=false;
	if(!(type&0x80)){fseek(f,-1,SEEK_CUR);type=lasttype;}
	switch(type&0xF0)
	{
		case 0x80://Note Off
		case 0x90://Note On
		case 0xA0://Note Aftertouch
		case 0xB0://Controller Change
		case 0xE0://Pitch wheel
			p1=read_u8();p2=read_u8();
			curTrack->appendEvent(SEvent(curid,curt,type,p1,p2));
		break;
		case 0xC0://Patch Change
		case 0xD0://Channel Aftertouch
			p1=read_u8();
			curTrack->appendEvent(SEvent(curid,curt,type,p1,0));
		break;
		case 0xF0:
			if((type&0x0F)==0x0F)//Meta Event
			{
				uint8_t metatype=read_u8();
				uint32_t len=read_varlen();char* str=nullptr;
				if(len<=1024&&len>0)str=new char[len+8];
				if(str)fread(str,1,len,f);else fseek(f,len,SEEK_CUR);
				if(str)str[len]='\0';
				switch(metatype)
				{
					case 0x00://Sequence Number
						assert(len==2||len==0);
					break;
					case 0x20://Channel Prefix
						assert(len==1);
					break;
					case 0x2F://End of Track
						assert(len==0);
					return 0;
					case 0x51://Set Tempo
						assert(len==3);
						curTrack->appendEvent(SEvent(curid,curt,type,metatype,0,str));
					break;
					case 0x54://SMTPE offset, not handled.
						assert(len==5);
					break;
					case 0x58://Time signature
						assert(len==4);
						curTrack->appendEvent(SEvent(curid,curt,type,metatype,0,str));
					break;
					case 0x59://Key signature
						assert(len==2);
						curTrack->appendEvent(SEvent(curid,curt,type,metatype,0,str));
					break;
					case 0x01:case 0x02:case 0x03:
					case 0x04:case 0x05:case 0x06:
					case 0x07:case 0x7F:default://text-like meta
					{
						curTrack->appendEvent(SEvent(curid,curt,type,metatype,0,str));
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
					}
				}
				if(str)delete[] str;
			}
			else if((type&0x0F)==0x00||(type&0x0F)==0x07)//SysEx
			{
				uint32_t len=read_varlen();char* str=nullptr;
				str=new char[len+8];
				if((type&0x0F)==0x00)
				{
					str[0]=0xF0;++len;
					size_t sz=fread(str+1,1,len-1,f);
					if(sz<len-1)error(1,"Unexpected EOF");
				}
				else
				{
					size_t sz=fread(str,1,len,f);
					if(sz<len)error(1,"Unexpected EOF");
				}
				curTrack->appendEvent(SEvent(curid,curt,type,len,0,str));
				if(!strcmp(str,GM1SysX))ret->std=1;
				if(!strcmp(str,GM2SysX))ret->std=2;
				if(!strcmp(str,GSSysEx))ret->std=3;
				if(!strcmp(str,XGSysEx))ret->std=4;
				delete[] str;
			}
			else error(0,"Unknown event type %#x",type);
		break;
		default:
			error(0,"Unknown event type %#x",type);
	}
	lasttype=type;++curid;
	if(curTrack->eventList.size())
	{
		SEvent& le=curTrack->eventList.back();
		CMidiPlayer::getInstance()->callEventReaderCB(le);
	}
	return 1;
}
void CSMFReader::read_track()
{
	ret->tracks.push_back(CMidiTrack());
	curTrack=&ret->tracks.back();
	uint32_t chnklen=read_u32();byteread=ftell(f);curt=0;curid=0;
	while(read_event());
	byteread=ftell(f)-byteread;
	if(byteread<chnklen)
	{
		error(0,"Extra bytes after EOT event");
		for(;byteread<chnklen;++byteread)fgetc(f);
	}
	if(byteread>chnklen)
		error(1,"Read past end of track");
}
void CSMFReader::read_header()
{
	uint32_t chnklen=read_u32();byteread=ftell(f);
	if(chnklen<6)error(1,"Header chunk too short");
	if(chnklen>6)error(0,"Header chunk length longer than expected. Ignoring extra bytes");
	fmt=read_u16();trk=read_u16();ret->divs=read_u16();
	if(ret->divs&0x8000)error(1,"SMTPE format is not supported");
	for(byteread=ftell(f)-byteread;byteread<chnklen;++byteread){fgetc(f);}
}
uint32_t CSMFReader::read_chunk(int is_header)
{
	char hdr[6];
	fread(hdr,1,4,f);
	if(feof(f))error(1,"Unexpected EOF");
	if(is_header)
	{
		if(!strncmp(hdr,"RIFF",4))
		{
			fseek(f,4,SEEK_CUR);
			fread(hdr,1,4,f);
			if(strncmp(hdr,"RMID",4)){error(1,"Wrong file type in RIFF container");}
			fseek(f,8,SEEK_CUR);
			fread(hdr,1,4,f);
		}
		if(strncmp(hdr,"MThd",4)){error(1,"Wrong MIDI header.");}
		else return read_header(),0;
	}
	else
		if(strncmp(hdr,"MTrk",4))
		{
			error(0,"Wrong track chunk header. Ignoring the whole chunk");
			uint32_t chnklen=read_u32();fseek(f,chnklen,SEEK_CUR);return 0;
		}
		else return read_track(),1;
	return 0;
}
CSMFReader::CSMFReader()
{
	f=nullptr;
}
CMidiFile* CSMFReader::readFile(const char* fn)
{
	ret=new CMidiFile;
	ret->title=ret->copyright=nullptr;ret->std=0;ret->valid=1;
	try
	{
		if(!(f=fopen(fn,"rb")))
			throw std::runtime_error("Can't open file");
		read_chunk(1);
		for(uint32_t i=0;i<trk;i+=read_chunk(0));
		fclose(f);f=nullptr;
	}
	catch(std::runtime_error& e)
	{
		fprintf(stderr,"CSMFReader E: %s is not a supported file. Cause: %s.\n",fn,e.what());
		ret->valid=0;if(f)fclose(f);f=nullptr;
	}
	return ret;
}
CSMFReader::~CSMFReader()
{
}

void CSMFReader::discardCurrentEvent()
{
	if(eventdiscarded)return;eventdiscarded=true;
	curTrack->eventList.pop_back();
}
void CSMFReader::commitEventChange(SEvent d)
{
	curTrack->eventList.back().time=d.time;
	curTrack->eventList.back().type=d.type;
	curTrack->eventList.back().p1=d.p1;
	curTrack->eventList.back().p2=d.p2;
}

CMidiFileReaderCollection::CMidiFileReaderCollection()
{
	readers.clear();currentReader=nullptr;
	registerReader(new CSMFReader(),"Default SMF Reader");
}
CMidiFileReaderCollection::~CMidiFileReaderCollection()
{
	delete readers[0].first;
}
void CMidiFileReaderCollection::registerReader(qmpFileReader* reader,std::string name)
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
CMidiFile* CMidiFileReaderCollection::readFile(const char* fn)
{
	CMidiFile *file=nullptr;
	for(unsigned i=0;i<readers.size();++i)
	{
		currentReader=readers[i].first;
		CMidiPlayer::getInstance()->notes=0;
		CMidiFile* t=readers[i].first->readFile(fn);
		if(t->valid){file=t;break;}
		else delete t;
	}
	currentReader=nullptr;
	return file;
}
qmpFileReader* CMidiFileReaderCollection::getCurrentReader()
{return currentReader;}
