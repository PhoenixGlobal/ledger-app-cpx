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
#1 CPX:
message_1=str("0000000101f753e908bde2dea0dc378cb39995f058d17682ce8dc34d5f4a634db23def2e6ba35df5537a9a304f080de0b6b3a76400000000000000000001000602ba7def3000030493e0000001706e7b73c2")

#0.1 CPX:
message_2=str("0000000101f753e908bde2dea0dc378cb39995f058d17682ce8dc34d5f4a634db23def2e6ba35df5537a9a304f08016345785d8a00000000000000000001000602ba7def3000030493e0000001706e7bffa1")

#1.123456 CPX:
message_3=str("0000000101f753e908bde2dea0dc378cb39995f058d17682ce8dc34d5f4a634db23def2e6ba35df5537a9a304f080f97514798c800000000000000000001000602ba7def3000030493e0000001706e7cb1ca")

#100 CPX:
message_4=str("0000000101f753e908bde2dea0dc378cb39995f058d17682ce8dc34d5f4a634db23def2e6ba35df5537a9a304f09056bc75e2d631000000000000000000001000602ba7def3000030493e0000001706e7d2fab")

#100 Mio CPX:
message_5=str("0000000101f753e908bde2dea0dc378cb39995f058d17682ce8dc34d5f4a634db23def2e6ba35df5537a9a304f0b52b7d2dcc80cd2e40000000000000000000001000602ba7def3000030493e0000001706e7db845")

#1 P:
message_6=str("0000000101f753e908bde2dea0dc378cb39995f058d17682ce8dc34d5f4a634db23def2e6ba35df5537a9a304f01010000000000000001000602ba7def3000030493e0000001706e7e434b")

#999 P:
message_7=str("0000000101f753e908bde2dea0dc378cb39995f058d17682ce8dc34d5f4a634db23def2e6ba35df5537a9a304f0203e70000000000000001000602ba7def3000030493e0000001706e7e434b")

#0.123456123456123456 CPX:
message_8=str("0000000101f753e908bde2dea0dc378cb39995f058d17682ce8dc34d5f4a634db23def2e6ba35df5537a9a304f0801b69ab0aff2f2400000000000000001000602ba7def3000030493e0000001706e7e434b")

#300000089.123456123456123456 CPX:
message_9=str("0000000101f753e908bde2dea0dc378cb39995f058d17682ce8dc34d5f4a634db23def2e6ba35df5537a9a304f0bf8277d6b2e459e8db6f2400000000000000001000602ba7def3000030493e0000001706e7e434b")


message=message_9

#message = str("0000000101f753e908bde2dea0dc378cb39995f058d17682ce8dc34d5f4a634db23def2e6ba35df5537a9a304f080de0b6b3a76400000000000000000001000602ba7def3000030493e0000001704aaf7650")
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

