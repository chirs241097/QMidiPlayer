TEMPLATE = subdirs

!android {
SUBDIRS = \
	qmidiplayer-desktop \
	qmidiplayer-lite \
	visualization
}
android {
SUBDIRS = \
	qmidiplayer-lite
}
