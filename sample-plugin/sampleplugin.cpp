#include <cstdio>
#include "sampleplugin.hpp"

qmpSamplePlugin::qmpSamplePlugin(qmpPluginAPI* _api){api=_api;}
qmpSamplePlugin::~qmpSamplePlugin(){api=nullptr;}
void qmpSamplePlugin::init()
{fputs("Hello world from plugin init!\n",stderr);}
void qmpSamplePlugin::deinit()
{fputs("Bye!\n",stderr);}
const char* qmpSamplePlugin::pluginGetName(){return "QMidiPlayer Sample Plugin";}
const char* qmpSamplePlugin::pluginGetVersion(){return "0.0.0";}
