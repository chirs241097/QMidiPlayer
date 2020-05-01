#include <QProcess>
#include <QCommandLineParser>

#include "qmpvisrendercore.hpp"
#include "qmpsettingsro.hpp"

int main(int argc,char **argv)
{
	QCoreApplication::setApplicationName("qmpvisrender");
	QCoreApplication::setApplicationVersion("0.0.0");
	QCoreApplication a(argc,argv);
	QCommandLineParser clp;
	clp.setApplicationDescription("Renderer a visualization of a midi file.");
	clp.addHelpOption();
	clp.addVersionOption();
	clp.addOption({{"f","output-file"},"File name of the output file.","filename","output.mp4"});
	clp.addOption({
		"receiver",
		"Specify a program and its arguments to process the rendered frames. Supports parameter substitution. See documentation for details.",
		"command",
		"ffmpeg -f rawvideo -pixel_format rgba -video_size %wx%h -framerate %r -i pipe: -vf vflip -pix_fmt yuv420p -c:v libx264 -preset slow -crf 22 %o"
	});
	clp.addOption({
		{"e","receiver-execution"},
		"Execution mode of the receiver command. Valid options are 'one-shot' and 'per-frame'",
		"mode",
		"one-shot"
	});
	clp.addOption({{"s","show-window"},"Do not hide the visualization window."});
	clp.addOption({{"c","config"},"Load options from the configuration file.","qmprc file"});
	clp.addOption({{"o","option"},"Set option for the visualization module.","key-value pair"});
	clp.addPositionalArgument("file","MIDI file to render");
	clp.process(a.arguments());
	qmpVisRenderCore core(&clp);
	if(clp.positionalArguments().empty())
		clp.showHelp(1);
	core.loadSettings();
	if(!core.loadVisualizationLibrary())
		return 1;
	if(clp.positionalArguments().size())
		core.setMIDIFile(clp.positionalArguments().front().toStdString().c_str());
	core.startRender();
	int retval=a.exec();
	core.unloadVisualizationLibrary();
	return retval;
}
