# CPX App for the Ledger Nano S

2020-05-07

This is the community edition of the Ledger app for the CPX Cryptocoin.
(Note: currently only Nano S supported)

WARNING: Use only for development and test purpose, at your own risk!

### Introduction
This document describes how to build the ledger-app-cpx and deploy it locally
via USB connection on a Ledger Nano S.

The development has been done and tested on OSX Catalina, but should naturally
be executable on any Linux based system (e.g. Ubuntu or CentOS) with only minor adjustemts.

Building and managing the lifecycle (e.g. installation) of the app is performed in two separate environments:  
> Host (application lifecycle)    
> - Application source code
> - GCC crosscompiler for ARM (gcc-arm-none-eabi)
> - C language family frontend for LLVM (clang+llvm-7.0.0)
> - Ledger Nano S SDK (nanos-secure-sdk)
> - Virtual python3 environment with tools to communicate with the Ledger device  
> (install/delete apps and test execution)

> Docker (application build)   
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

+ Retrieve public key:  
`./test_get_pubkey.py`

> HID => 80040000ff8000002c80000378800000000000000000000000  
> HID <= 04991d28c31abfc7024e416008b61d291a65270e02ec460364a36692f6e9d795fa4d8ac73bf4523055f                  
51a7d1644d6875b5f8b61c3586ed9488c47a6675e484af79000  
> publicKey       [65] 04991D28C31ABFC7024E416008B61D291A65270E02EC460364A36692F6E9D795FA4D8AC73BF452  3055F51A7D1644D6875B5F8B61C3586ED9488C47A6675E484AF7


+ Sign TX:  
`./test_signature_cpx.py`

> HID => 80040000ff8000002c80000378800000000000000000000000  
> HID <= 04991d28c31abfc7024e416008b61d291a65270e02ec460364a36692f6e9d795fa4d8ac73bf4523055f  51a7d1644d6875b5f8b61c3586ed9488c47a6675e484af79000  
> publicKey       [65] 04991D28C31ABFC7024E416008B61D291A65270E02EC460364A36692F6E9D795FA4D8AC73BF452  3055F51A7D1644D6875B5F8B61C3586ED9488C47A6675E484AF7  
message       [170] 0000000101f753e908bde2dea0dc378cb39995f058d17682ce8dc34d5f4a634db23def2e6ba35df5537a  9a304f0bf8277d6b2e459e8db6f2400000000000000001000602ba7def3000030493e0000001706e7e434b  
HID => 80028000690000000101f753e908bde2dea0dc378cb39995f058d17682ce8dc34d5f4a634db23de   f2e6ba35df5537a9a304f0bf8277d6b2e459e8db6f2400000000000000001000602ba7def3000030493e0000001706e7e434b8000002c80000378800000000000000000000000  
HID <= 304402203895d041b890ddadb9f4873bf2036f65ec246d9e77b355bfceb43c6368a1a4a802201be80f   016fc7212140b22ab836c9f69d95b7aa3da50abaffa2aa1da474c6d84d9000  
signature       [70] 304402203895d041b890ddadb9f4873bf2036f65ec246d9e77b355bfceb43c6368a1a4a802201be80f  016fc7212140b22ab836c9f69d95b7aa3da50abaffa2aa1da474c6d84d

Tests have been performed on a Ledger Nano S with a public known test mnemonic setup (can be found [here](https://coranos.github.io/neo/ledger-nano-s/recovery/)):

> - Mnemonic:     online ramp onion faculty trap clerk near rabbit busy gravity prize employ exit horse found slogan effort dash siren buzz sport pig coconut element
> - ElementPath:  m/44'/888'/0'/0/0
> - Address:         AeKd54zJdgqXy41NgH1PicXTVcz3RdRFdh  
> - WIF:                 KxGtKtYTsxCW997F762Zn62C2e72gQ9XMPkkL2231Rc4GuvSCuba 

----
Useful resources:  

[Ledger's documentation Hub](http://ledger.readthedocs.io) for the complete Ledger documentation

[Ledger Documentation Hub (as PDF)](https://buildmedia.readthedocs.org/media/pdf/ledger/latest/ledger.pdf)

[ledgerblue](https://pypi.org/project/ledgerblue/) Python tools to communicate with Ledger Blue, Nano S and Nano X and manage applications life cycle
