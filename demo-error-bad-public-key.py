#!/usr/bin/env python
#*******************************************************************************
#*   Ledger Blue
#*   (c) 2016 Ledger
#*
#*  Licensed under the Apache License, Version 2.0 (the "License");
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*      http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an "AS IS" BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#********************************************************************************
from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException
from secp256k1 import PublicKey

bipp44_path = (
               "8000002C"
              +"80000378"
              +"80000000"
              +"00000000"
              +"00000000")

dongle = getDongle(True)
goodPublicKey = dongle.exchange(bytes(("80040000FF"+ bipp44_path).decode('hex')))
goodPublicKeyHex = str(goodPublicKey).encode('hex')
print "goodPublicKeyHex: " + goodPublicKeyHex

badPublicKey = dongle.exchange(bytes(("80040000FF"+ bipp44_path).decode('hex')))
badPublicKeyHex = str(badPublicKey).encode('hex')
print "badPublicKeyHex: " + badPublicKeyHex

print "bad equals good: " + str(badPublicKeyHex == goodPublicKeyHex)
