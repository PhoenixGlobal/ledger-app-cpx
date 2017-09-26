# blue-app-neo CE

This is the community edition of the Ledger Nano S app for the NEO Cryptocoin.

Run `make load` to build and load the application onto the device.

After installing and running the application, you can run `demo.py` to test signing several transactions over USB.

Each transaction should display correctly in the UI.
Use the buttons individually to scroll up and down to view the transaction details.
Either Sign or Deny the transaction by clicking both top buttons on the 'Sign Tx Now', 'Sign Tx' and 'Deny Tx' screens.
The only difference between 'Sign Tx Now' and 'Sign Tx' is their placement order in the screen list, both sign the transaction.

Note that in order to run `demo.py`, you must install the `secp256k1` Python package:

```
pip install secp256k1
```

See [Ledger's documentation](http://ledger.readthedocs.io) to get started.
=======
# blue-app-neo
