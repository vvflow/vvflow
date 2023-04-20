docker run -it --rm -v $HOME/w/vvflow:/vvflow:ro centos:7
yum -y groupinstall "Development Tools"
yum -y install openssl openssl-devel
yum -y install atlas-devel lapack64-devel
mkdir ~/build
cd ~/build

curl -L https://github.com/Kitware/CMake/releases/download/v3.26.3/cmake-3.26.3.tar.gz -O
tar -xaf cmake-3.26.3.tar.gz
pushd cmake-3.26.3
./bootstrap
make
make install
popd

curl -L https://netlib.org/blas/blast-forum/cblas.tgz -O
tar -xaf cblas.tgz
pushd CBLAS
make dlib2 -B CFLAGS="-O3 -DADD_ -fPIC"
popd
