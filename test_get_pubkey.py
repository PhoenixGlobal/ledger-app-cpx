#!/usr/bin/env python

from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException

bipp44_path = (
    "8000002C"
    + "80000378"
    + "80000000"
    + "00000000"
    + "00000000")



dongle = getDongle(True)
publicKey = dongle.exchange(
    bytes(bytearray.fromhex("80040000FF" + bipp44_path)))


print("publicKey       [" + str(len(publicKey)) +
      "] " + publicKey.hex().upper())
