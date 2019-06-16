# gtk version
# highly recommended gtk3
gtkv=gtk3
outdir=bin
# flags=-g

if [ "$1" == "install-build" ]
then
    echo "Compiling with __INSTALL_BUILD__ macro"
    flags="$flags -D__INSTALL_BUILD__"
fi
mkdir -p $outdir
time g++ $flags src/main.cpp -o $outdir/nvidia-system-monitor `wx-config  --version=3.0  --toolkit=$gtkv  --cxxflags --libs`
