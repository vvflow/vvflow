#!/dev/null

mkdir -p build

OS="$1"
[ -n "$OS" ] || OS="ubuntu-xenial"
echo Running $OS

sudo docker build --target tester -t vvflow-tester:$OS $PWD -f Dockerfile.$OS && \
sudo docker run -it --rm \
    -v $PWD:/vvflow:ro \
    -v $PWD/build/deb:/root \
    vvflow-tester:$OS
    