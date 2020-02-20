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


#CPX test message
message = str("0000000101f753e908bde2dea0dc378cb39995f058d17682ce8dc34d5f4a634db23def2e6ba35df5537a9a304f080de0b6b3a76400000000000000000001000602ba7def3000030493e0000001704aaf7650")
#message = str("0000000101f753e908bde2dea0dc378cb39995f058d17682ce8dc34d5f4a634db23def2e6ba35df5537a9a304f0810A741A4627800000000000000000001000602ba7def3000030493e0000001704aaf7650")

textToSign_01 = bytes.fromhex(message + bipp44_path)

textToSignArray = [textToSign_01]

dongle = getDongle(True)
publicKey = dongle.exchange(
    bytes(bytearray.fromhex("80040000FF" + bipp44_path)))



print("publicKey       [" + str(len(publicKey)) +
      "] " + publicKey.hex().upper())

print("message       [" + str(len(message)) +
      "] " + message)


for textToSign in textToSignArray:
	try:
		offset = 0
		while offset != len(textToSign):
			if (len(textToSign) - offset) > 255:
				chunk = textToSign[offset : offset + 255] 
			else:
				chunk = textToSign[offset:]
			if (offset + len(chunk)) == len(textToSign):
				p1 = 0x80
			else:
				p1 = 0x00

		
			apdu = bytes.fromhex("8002") + bytes([p1]) + bytes([0x00]) + bytes([len(chunk)]) + bytes(chunk)

            
			signature = dongle.exchange(apdu)
			offset += len(chunk)  	
		print ("signature       [" + str(len(signature)) + "] " + signature.hex())
	except CommException as comm:
		if comm.sw == 0x6985:
			print ("Aborted by user")
		else:
			print ("Invalid status " + hex(comm.sw))

