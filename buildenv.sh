#!/dev/null

sudo docker build -t dovvbuild:latest $PWD && \
sudo docker run -it --rm \
    -v $HOME/vvhd:/vvhd:ro \
    -v $PWD:/build \
    dovvbuild:latest /bin/bash
