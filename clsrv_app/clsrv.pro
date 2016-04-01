QT += widgets network gui core

TARGET  = client-server

TEMPLATE    = app

HEADERS = client.h \
    splitter.h \
    MyServer.h

SOURCES = client.cpp \
    main.cpp \
    splitter.cpp \
    MyServer.cpp


 # install
# target.path = $$[QT_INSTALL_EXAMPLES]/network/fortuneclient
# sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS fortuneclient.pro
# sources.path = $$[QT_INSTALL_EXAMPLES]/network/fortuneclient
# INSTALLS += target sources

RESOURCES += \
    qt_form.qrc

