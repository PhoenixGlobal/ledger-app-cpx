# CPX App for the Ledger Nano S

2020-05-06

This is the community edition of the Ledger app for the CPX Cryptocoin.
(Note: currently only Nano S supported)

### Introduction
This document describes how to build the ledger-app-cpx and deploy it locally
via USB connection on a Ledger Nano S.

The development has been done and tested on OSX Catalina, but should naturally
be executable on any Linux based system (e.g. Ubuntu or CentOS) with only minor adjustemts.

Building and managing the lifecycle (e.g. installation) of the app is performed in two separate environments:  
>\# Host (application lifecycle)    
> - Application source code
> - GCC crosscompiler for ARM (gcc-arm-none-eabi)
> - C language family frontend for LLVM (clang+llvm-7.0.0)
> - Ledger Nano S SDK (nanos-secure-sdk)
> - Virtual python3 environment with tools to communicate with the Ledger device  
> (install/delete apps and test execution)

> \# Docker (application build)   
> - Access to the host environment via shared volume  
> - Additional libraries for gcc  
> - Build the application in a controlled and segregated environment

The two environments should be setup and operated in two parallel terminal sessions in order to speed up and optimize the development lifecycle.  

### Host Environment Setup 
Open the first terminal session (T1) and perform the following steps:

1. Create and change into the shared docker folder in your homedir:  
`mkdir -p ~/docker-shared/ledgerbuild`  
`cd ~/docker-shared/ledgerbuild`  
2. Clone the git repository and change into the downloaded folder:  
`git clone https://github.com/APEX-Network/ledger-app-cpx.git`  
`cd ledger-app-cpx`

3. Prepare the environment setup by calling the following script  
  (for now only parameter 's' (Ledger Nano S) is supported):  
  `./prepare-devenv.sh s`  

   This will create a subfolder 'dev-env', if not existing, in which all 
   neccessary tools and SDKs are downloaded and unpacked.  
   Also a virtual python3 environment will be created and required tools for
   communicating with the Ledger hardware will be installed.  

4. Activate the virtual python3 environment:  
`source dev-env/ledger_py3/bin/activate`  

5. Ledger Nano S custom CA installation: to be done only once!  
In Ledger app development, it is necessary to enter your PIN code each time you install an unsigned app.  
By installing a custom certificate once on your device you can avoid having to retype your PIN each time you adjust your app.  
Here are the steps for the Ledger Nano S:

   a.) Generate a public / private keypair using the following command:  
  `python3 -m ledgerblue.genCAPair`  
    Public key : pubkeyxxx  
    Private key: privkeyxxx

   b.) Enter recovery mode on your Ledger Nano S:  
   Do this by unplugging it then holding down the right button (near the hinge, away from USB port) while plugging it in again.  
   Recovery mode should then appear on the screen. Enter your pin and continue.
  
   c.) Load your public key onto the Ledger Nano S:  
   Paste the public key generated at step 5a after --public.  
   Paste the private key generated at step 5a after --rootPrivateKey.  
   Include a --name parameter containing the name of the custom certificate (any string will do):  
  `python3 -m ledgerblue.setupCustomCA --apdu --rootPrivateKey=privkeyxxx --public=pubkeyxxx --name=TestCA --targetId 0x31100004`  

6. Export following environment variables in your activated virtual python environment:  
`export BOLOS_SDK=$(pwd)/dev-env/SDK/nanos-secure-sdk`  
`export BOLOS_ENV=$(pwd)/dev-env/CC`  
`export SCP_PRIVKEY=privkeyxxx`

### Docker Image Setup  
Open the second terminal session (T2) and perform the following steps:  

1. Change into the shared docker folder in your homedir:  
`cd ~/docker-shared/ledgerbuild/ledger-app-cpx`  

2. Execute the following command in order to compose the docker image:  
`docker-compose build`

### Build the application  

Perform the following steps in T2:  

1. Start the docker container into an interactive bash session:  
`./start_docker.sh`

2. Change into the folder containing the ledger app source code:  
`cd /docker-shared/ledgerbuild/ledger-app-cpx/` 

3. Build the app (ignore the warnings from the SDK):  
`make`

### Load the application  
Perform the following steps in T1:

1. Connect your Ledger Nano S device via USB to the host and unlock with your PIN    
  
2. Install the app:  
`make load`

3. Delete the app:  
`make delete`

  (Allow trusted manager on the Ledger device, when running the following command for the first time)
  
### Test the application  
Open the installed CPX app on the Ledger device and use one of the following
test scripts in T1:

1. Retrieve public key:  
`./test_get_pubkey.py`

2. Sign TX:  
`./test_signature_cpx.py`

----
Useful resources:  

[Ledger's documentation Hub](http://ledger.readthedocs.io) for the complete Ledger documentation

[Ledger Documentation Hub (as PDF)](https://buildmedia.readthedocs.org/media/pdf/ledger/latest/ledger.pdf)
