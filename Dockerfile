from ubuntu:xenial

WORKDIR /root
CMD /bin/bash

RUN apt-get update && \
	apt-get upgrade -y && \
	apt-get install -y less make gnuplot python gnupg curl man apt-transport-https && \
	curl -L https://packagecloud.io/vvflow/stable/gpgkey | apt-key add - && \
	echo "deb https://packagecloud.io/vvflow/stable/ubuntu/ xenial main" | tee /etc/apt/sources.list.d/vvflow.list && \
	apt-get update && \
	apt-get install -y vvflow && \
	cp -R /usr/share/doc/vvflow/example/ ./ && \
	apt-get clean
