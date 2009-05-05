CONFIG += console

SOURCES = socket.cpp \
	main.cpp \
	app.cpp \
	messagerecv.cpp \
	clientinfo.cpp \
	udpsocket.cpp

HEADERS = socket.h \
	app.h \
	messagerecv.h \
	clientinfo.h \
	udpsocket.h

SOURCES += coloritem.cpp \
	tile.cpp \
	table.cpp \
	player.cpp \
	localplayer.cpp \
	game.cpp \
	networkplayer.cpp

HEADERS += coloritem.h \
	tile.h \
	table.h \
	player.h \
	localplayer.h \
	game.h \
	networkplayer.h

FORMS += racingForm.ui \
	optionsDialog.ui
