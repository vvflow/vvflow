#!/dev/null

mkdir -p build

OS="$1"
[ -n "$OS" ] || OS="ubuntu-xenial"
echo Running $OS

sudo docker build -t vvflow-build:$OS $PWD -f Dockerfile.$OS && \
sudo docker run -it --rm \
    -v $PWD:/vvflow:ro \
    -v $PWD/build:/root \
    -p 1207:1207 \
    -p 1208:1208 \
    vvflow-build:$OS

# Ronn cheatsheet:
# http://ricostacruz.com/cheatsheets/ronn.html

# python -m SimpleHTTPServer 1207 &
# grip /vvflow/README.md 0.0.0.0:1207 &
# http://127.0.0.1:1207/utils/vvcompose2/vvcompose.1.html#BODY-GENERATORS
# man -l utils/vvcompose2/vvcompose.1