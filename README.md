# blue-app-neo CE

This is the community edition of the Ledger Nano S app for the NEO Cryptocoin.

Documentation on how it works is here:
[sequence diagrams](https://coranos.github.io/blue-app-neo/docs/index.html)

Run `make load` to build and load the application onto the device.

After installing and running the application, you can run `demo.py` to test signing several transactions over USB.

Each transaction should display correctly in the UI.
Use the buttons individually to scroll up and down to view the transaction details.
Either Sign or Deny the transaction by clicking both top buttons on the 'Sign Tx Now', 'Sign Tx' and 'Deny Tx' screens.
The only difference between 'Sign Tx Now' and 'Sign Tx' is their placement order in the screen list, both sign the transaction.

Note that in order to run `demo-GAS-NEO.py`, you must install the `fastecdsa` Python package:

```
pip install ecdsa
```

also install GMP (https://gmplib.org/)

See [The Environment Setup Guide](https://coranos.github.io/neo/ledger-nano-s/development/environment.html) if you want to build the appyourself..
========

See [Ledger's documentation](http://ledger.readthedocs.io) to get started.
=======
# blue-app-neo

todo: key screen does not refresh, have to go in and out to see a refresh.