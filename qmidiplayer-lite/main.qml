import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.0
import QtQuick.Dialogs 1.0
import org.chrisoft.qmpcore 1.0

Window {
	id: window1
	width: 420
	height: 240
	title: "QMidiPlayer Lite TP"
	visible: true
	property bool playing
	playing: false
	property string soundfont
	soundfont: ""
	SystemPalette
	{
		id: palette
		colorGroup: SystemPalette.Active
	}
	color: palette.window

	MouseArea {
		id: mouseArea1
		anchors.rightMargin: 0
		anchors.bottomMargin: 0
		anchors.leftMargin: 0
		anchors.topMargin: 0
		anchors.fill: parent

		Button {
			id: button2
			x: 170
			text: qsTr("Play")
			anchors.horizontalCenterOffset: 0
			anchors.top: parent.top
			anchors.topMargin: 213
			anchors.horizontalCenter: parent.horizontalCenter
			onClicked: {
				if(!playing)
				{
					qmpcore.loadFile(fileName.text);
					qmpcore.initFluidSynth(soundfont);
					qmpcore.playFile();
					playing=true;
					uiTimer.start();
					text="Stop";
				}
				else
				{
					qmpcore.stop();
					playing=false;
					text="Play";
					hsTimer.value=0;uiTimer.stop();
				}
			}
		}

		Slider {
			id: hsTimer
			y: 190
			height: 22
			anchors.right: parent.right
			anchors.rightMargin: 0
			anchors.left: parent.left
			anchors.leftMargin: 0
			anchors.bottom: parent.bottom
			anchors.bottomMargin: 28
			tickmarksEnabled: false
			stepSize: 1
			maximumValue: 100
			property bool autovalchange: false
			onValueChanged: {
				if(autovalchange||pressed)return;
				if(playing){qmpcore.setTCeptr(value);qmpcore.panic();}
			}
			onPressedChanged: {
				if(!pressed)
				{
					/*if(playing)
					{
						if(ui->hsTimer->value()==100){on_pbNext_clicked();return;}
						player->setTCeptr(player->getStamp(ui->hsTimer->value()),ui->hsTimer->value());
						player->playerPanic();
						offset=ui->hsTimer->value()/100.*player->getFtime();
						st=std::chrono::steady_clock::now();
					}
					else
					{
						player->setTCeptr(player->getStamp(ui->hsTimer->value()),ui->hsTimer->value());
						offset=ui->hsTimer->value()/100.*player->getFtime();
						char ts[100];
						sprintf(ts,"%02d:%02d",(int)(offset)/60,(int)(offset)%60);
						ui->lbCurTime->setText(ts);
					}*/
					if(playing){qmpcore.setTCeptr(value);qmpcore.panic();}
				}
			}
		}

		Button {
			id: button
			anchors.top: parent.top
			anchors.topMargin: 67
			anchors.horizontalCenter: parent.horizontalCenter
			x: 139
			text: qsTr("Load SoundFont")
			onClicked: {
				sfFileDialog.open()
			}
		}
	}

	CQMPCoreWrapper {
		id: qmpcore
	}
	Timer {
		id: uiTimer
		interval: 100
		running: false
		repeat: true
		onTriggered: {
			if(!hsTimer.pressed)
			{
				hsTimer.autovalchange=true;
				hsTimer.value=qmpcore.getProgress();
				hsTimer.autovalchange=false;
			}
		}
	}

	Text {
		id: fileName
		text: qsTr("...")
		horizontalAlignment: Text.AlignHCenter
		color: palette.text
		anchors.top: parent.top
		anchors.topMargin: 100
		anchors.horizontalCenter: parent.horizontalCenter
		x: 139
		width: 31
		height: 11
	}

	Button {
		id: button1
		x: 170
		text: qsTr("Open")
		anchors.top: parent.top
		anchors.topMargin: 130
		anchors.horizontalCenter: parent.horizontalCenter
		onClicked: {
			fileDialog.open();
		}
	}

	FileDialog {
		id: fileDialog
		title: qsTr("Select midi file")
		nameFilters: ["MIDI Files (*.mid *.midi)"]
		onAccepted: {
			fileName.text=fileUrl;
		}
	}
	FileDialog {
		id: sfFileDialog
		title: qsTr("Select SoundFont")
		nameFilters: ["SoundFont (*.sf2)"]
		onAccepted: {soundfont=fileUrl;}
	}
}

