# gtk version
# highly recommended gtk3
gtkv=gtk3
output=bin/nvsm
# flags=-g

if [ "$1" == "install-build" ]
then
    echo "Compiling with __INSTALL_BUILD__ macro"
    flags="$flags -D__INSTALL_BUILD__"
fi

time g++ $flags src/main.cpp -o $output `wx-config  --version=3.0  --toolkit=$gtkv  --cxxflags --libs`