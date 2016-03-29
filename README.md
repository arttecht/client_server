#*******************
# client-server
#*******************

Build environment: QT5.2.1
Linux 3.13.0-65-generic #105~precise1-Ubuntu SMP Tue Sep 22 13:23:18 UTC 2015 i686 i686 i386 GNU/Linux

App Description:

Framework for current application were taken from training example QT5.2.1.

Transfer the file from Client to Server.
Currrent project has two simple subapplications. Client and Server applications
have GUI window notifications.

Client application:

To transfer the file you need to select the three fields:

Server name: has IP address for server.

Server Port: the port which opens on server side (has defaul value: 22222).

File name:   filename on the host side which must be transferred.


Server application:

Display the own IP address and Port which must be selected on client side.

#****************
# Build:
#****************
The projet has two build folders:

./build/build-client

./build/build-server

Make generating the Makefiles with the help of qmake:

cd ./build/build-client

qmake /full_path_to_project_folder/client_server/client/client.pro -r -spec linux-g++ CONFIG+=debug CONFIG+=declarative_debug CONFIG+=qml_debug

make -f Makefile


cd ./build/build-server

qmake /full_path_to_project_folder/client_server/server/server.pro -r -spec linux-g++ CONFIG+=debug CONFIG+=declarative_debug CONFIG+=qml_debug

make -f Makefile
