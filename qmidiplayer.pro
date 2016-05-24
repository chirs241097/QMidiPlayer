TEMPLATE = subdirs

!android {
SUBDIRS = \
	qmidiplayer-desktop \
	qmidiplayer-lite \
	sample-plugin
}
android {
SUBDIRS = \
	qmidiplayer-lite
}

!win32 {
SUBDIRS += visualization\
}
