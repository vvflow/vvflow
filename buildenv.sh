#!/dev/null

mkdir -p build

OS="$1"
[ -n "$OS" ] || OS="ubuntu-xenial"
echo Running $OS

sudo docker build --target builder -t vvflow-build:$OS $PWD -f Dockerfile.$OS && \
sudo docker run -it --rm \
    -v $PWD:/vvflow:ro \
    -v $PWD/build:/root \
    -p 1207:1207 \
    -p 1208:1208 \
    vvflow-build:$OS
