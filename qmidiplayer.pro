TEMPLATE = subdirs

!android {
SUBDIRS = \
    qmidiplayer-desktop \
    qmidiplayer-lite \
    sample-plugin \
    visualization
}
android {
SUBDIRS = \
    qmidiplayer-lite
}
