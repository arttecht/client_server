QT += widgets network gui core

TARGET  = client

TEMPLATE    = app

HEADERS = client.h

SOURCES = client.cpp \
    main.cpp


 # install
# target.path = $$[QT_INSTALL_EXAMPLES]/network/fortuneclient
# sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS fortuneclient.pro
# sources.path = $$[QT_INSTALL_EXAMPLES]/network/fortuneclient
# INSTALLS += target sources
