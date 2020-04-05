# shellcheck disable=SC1091,SC2155

# SOURCE THIS FILE
# . prepare-devenv blue|s|x

if [ $# -ne 1 ]; then
    echo "Possible options: s or x"
    exit
elif [[ $1 == "-h" ]]; then
    echo "Possible options: s or x"
    exit
elif [[ $1 != "s" ]] && [[ $1 != "x" ]]; then
    echo "Possible options: s or x"
    exit
fi

if [[ $(dpkg-query -s python3-venv 2>&1) == *'is not installed'* ]]; then
    printf "\nPackage python3-venv is missing.\nOn Debian-like distros, run:\n\napt install python3-venv\n\n"
    exit
fi

#if [[ $(cat /etc/udev/rules.d/20-hw1.rules) == *'ATTRS{idVendor}=="2c97", ATTRS{idProduct}=="0004"'* ]]; then
#    printf "\nMissing udev rules. Please refer to https://support.ledger.com/hc/en-us/articles/115005165269-Fix-connection-issues\n\n"
#    return
#fi


if [ ! -d dev-env ]; then
    mkdir dev-env
    mkdir dev-env/SDK
    mkdir dev-env/CC

    curl -L https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q1-update/+download/gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2 -o gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2
    tar xf gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2
    rm gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2
    cp -r gcc-arm-none-eabi-5_3-2016q1 dev-env/CC/nanox/gcc-arm-none-eabi-5_3-2016q1
    mv gcc-arm-none-eabi-5_3-2016q1 dev-env/CC/gcc-arm-none-eabi-5_3-2016q1

    curl -L http://releases.llvm.org/7.0.0/clang+llvm-7.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz -o clang+llvm.tar.xz
    tar xf clang+llvm.tar.xz
    rm clang+llvm.tar.xz
    mv clang+llvm* dev-env/CC/clang-arm-fropi

    curl -L https://github.com/LedgerHQ/nanos-secure-sdk/archive/nanos-160.tar.gz -o nanos-secure-sdk.tar.gz
    tar xf nanos-secure-sdk.tar.gz
    rm nanos-secure-sdk.tar.gz
    mv nanos-secure-sdk* dev-env/SDK/nanos-secure-sdk

    python3 -m venv dev-env/ledger_py3
    source dev-env/ledger_py3/bin/activate
    pip install wheel
    pip install ledgerblue
    pip install secp256k1
fi


source dev-env/ledger_py3/bin/activate

#if [[ $1 == "blue" ]]; then
#    export BOLOS_SDK=$(pwd)/dev-env/SDK/blue-secure-sdk
#    export BOLOS_ENV=$(pwd)/dev-env/CC/others
if [[ $1 == "s" ]]; then
    export BOLOS_SDK=$(pwd)/dev-env/SDK/nanos-secure-sdk
    export BOLOS_ENV=$(pwd)/dev-env/CC
elif [[ $1 == "x" ]]; then
    export BOLOS_SDK=$(pwd)/dev-env/SDK/nanox-secure-sdk
    export BOLOS_ENV=$(pwd)/dev-env/CC
fi


