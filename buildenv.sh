#!/dev/null

mkdir -p build
sudo docker build -t dovvbuild:latest $PWD && \
sudo docker run -it --rm \
    -v $PWD:/vvhd:ro \
    -v $PWD/build:/root \
    -p 1207:1207 \
    dovvbuild:latest /bin/bash

# Ronn cheatsheet:
# http://ricostacruz.com/cheatsheets/ronn.html