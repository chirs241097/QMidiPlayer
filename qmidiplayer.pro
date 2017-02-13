TEMPLATE = subdirs

!android {
SUBDIRS = \
	qmidiplayer-desktop \
	qmidiplayer-lite \
	sample-plugin \
	visualization \
	midifmt-plugin
}
android {
SUBDIRS = \
	qmidiplayer-lite
}

SUBDIRS += \
