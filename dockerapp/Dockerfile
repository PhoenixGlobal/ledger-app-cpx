FROM ubuntu:16.04

ENV	BOLOS_ENV=/docker-shared/ledgerbuild/ledger-app-cpx/dev-env/CC
ENV	BOLOS_SDK=/docker-shared/ledgerbuild/ledger-app-cpx/dev-env/SDK/nanos-secure-sdk

RUN	apt-get update -y &&\
    apt-get install lib32z1 -y &&\
    apt-get install gcc -y &&\
    apt-get install libstdc++6-4.7-dev -y &&\
    apt-get install libc6-dev-i386 -y &&\
    apt-get install python3 -y &&\
    apt-get install python3-pip -y &&\
    apt-get install libusb-1.0-0-dev -y &&\
    apt-get install libudev-dev -y &&\
    apt-get install curl -y &&\
    apt-get install vim -y &&\
    apt-get install git -y

RUN apt-get install libssl-dev -y &&\
    cd /usr/include/openssl/ &&\
    ln -s /usr/include/gnutls/openssl.h . &&\
    ln -s ../x86_64-linux-gnu/openssl/opensslconf.h .

RUN cd /root &&\
    pip3 install git+https://github.com/LedgerHQ/blue-loader-python.git --upgrade


CMD ["/bin/bash"]

