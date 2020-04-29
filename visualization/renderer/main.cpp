#include <QProcess>
#include <QCommandLineParser>

#include "qmpvisrendercore.hpp"

int main(int argc,char **argv)
{
	QCoreApplication::setApplicationName("qmpvisrender");
	QCoreApplication a(argc,argv);
	QCommandLineParser clp;
	clp.setApplicationDescription("Renderer a visualization of a midi file.");
	clp.addHelpOption();
	clp.parse(a.arguments());
	qmpVisRenderCore core;
	core.loadVisualizationLibrary();
	if(clp.positionalArguments().size())
		core.setMIDIFile(clp.positionalArguments().front().toStdString().c_str());
	core.startRender();
	int retval=a.exec();
	core.unloadVisualizationLibrary();
	return retval;
}
