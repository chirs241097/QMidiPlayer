# The API documentation of QMidiPlayer for plugin developers

*This manual is not yet complete. It's only a working draft for the always-changing plugin system in QMP.*
*Handle with care.*

# 0. Overview

Plugin for QMidiPlayer is a dynamically-loaded library that exports the symbol "qmpPluginGetInterface".
Before starting developing your own plugin, make sure to have a look at the sample plugin in the "sample-plugin" folder.

# 1. "QMidiPlayer Plugin SDK"

SDK for developing QMidiPlayer plugins is merely the "qmpcorepublic.hpp" header found in the "include" directory in
the source tree. It includes classes used by QMidiPlayer's internal plugin infrastructure.

# 2. Basics for a working plugin.

First of all, you should make your library distinct from other libraries that are not QMidiPlayer plugins. You can achive
it by exporting the symbol "qmpPluginGetInterface". Specifically, what you should do is to add the following snipplet to
somewhere of your code:

> extern "C"{
>	EXPORTSYM qmpPluginIntf* qmpPluginGetInterface(qmpPluginAPI* api)
>	//semicolon or implementation here.
> }

The EXPORTSYM macro tells the compiler to export the following symbol. qmpPluginIntf is the abstract class which every
plugin class should derive from. The parameter api provides access to QMidiPlayer's plugin API, which should be stored
for future use.

Next you should create your own plugin class which implements the abstract class "qmpPluginIntf".

# 3. A Peek into the class "qmpPluginIntf"

It has 6 public members: one default constructor, one default destructor and four methods:

- void init()
  Called on start up if the plugin is loaded successfully and enabled.
- void deinit()
  Called on shutdown if the plugin is enabled.
- const char* pluginGetName()
  This function should return the display name of the plugin.
- const char* pluginGetVersion()
  This function should return the version of the plugin, which will be shown in the plugin manager.

Your plugin is expected to register handlers and functionalities when init() is called by the host,
and do clean-up jobs when deinit() is caled.
