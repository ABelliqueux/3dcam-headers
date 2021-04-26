#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This working version corrected with the help of sickle : 
# https://discord.com/channels/642647820683444236/663664210525290507/836029253593858060
# Corrected script by Sickle : http://psx.arthus.net/code/rawdog.py

# Sickle - 26/04/2021 :
# " Ooh, you were like frustratingly close dude! Few tiny issues:
# - first of your 3 rolling buffers was bugged (other 2 were spot on)
# - waiting too long between commands at points, unirom timed out
# - var i was missing the i += chunkSize so we were stuck in a loop there (e.g. tried to send a second chonk)
# - exit was gummed up with the main logic being in a while True: "

# As suggested: 
# - Removed while True: loop
# - moved rolling buffer loops to WaitForResponse()
# - reduced sleeps
# - inc var i with chunkSize

import sys
import serial
import time
import math

ser = serial.Serial('/dev/ttyUSB0')

ser.baudrate = '115200'

chunkSize = 2048

responseBuffer = ''

address = '800b1470' # We're receiving this info as a string from the psx

# test data

data = int(111111111111111111111111111111111).to_bytes(14, byteorder='little', signed=False)

size = len( data )

checkSum = 0 # should be 1701

Ssbin = 0

Saddr = 0

Slen = 0

def WaitForResponse( expectedAnswer ):
    
    responseBuffer = ""
    
    while True:
                
        if ser.in_waiting:
        
            print( "Input buffer : " + str(ser.in_waiting))
            
            chkValue = ser.read(1)
            
            # Make sure byte value is < 128 so that it can be decoded to ascii :: always getting '\xc7' 'q' '\x1c' '\xc7'  '\xab' '\xed' '1' '\x05'
            
            if chkValue[0] < 128:
            
                responseBuffer += chkValue.decode('ascii')  
            
            else:
                
                responseBuffer += '.'
            
            if len( responseBuffer ) > 4:
                
                # remove first char in buffer
                
                responseBuffer = responseBuffer[1:]
            
                print( "Response buffer : " + responseBuffer )
            
            
            if responseBuffer == expectedAnswer:
                
                break
                
    print( "Got response : " + responseBuffer + " - " + expectedAnswer )

def main(args):
    
    global checkSum, responseBuffer, Ssbin, Saddr, Slen, chunkSize
    
    
    # Just to be sure 
    
    ser.reset_input_buffer()
    
    ser.reset_output_buffer()
    
    i = 0
    
    while i < len( data ):

        checkSum += data[i];
        
        i += 1
        
    print("checkSum : " + str(checkSum) )
    
    # ~ while True:

    # ~ global responseBuffer, Ssbin, Saddr, Slen, chunkSize
    
    if not Ssbin:
        
        print("Sending DEBG command...")
        
        ser.write( bytes( 'DEBG' , 'ascii' ) )
        
        time.sleep(.3)
                    
        # Empty in waiting buffer to get rid of 'DEBGOKAY'
        
        ser.reset_input_buffer()
        
        time.sleep(.5)
        
        print("Sending SBIN command...")
        
        ser.write( bytes( 'SBIN' , 'ascii' ) )
        
        time.sleep(.1)
        
        ser.write( bytes( 'UPV2' , 'ascii' ) )
    
        Ssbin = 1
        
        time.sleep(.1)
        
        # ~ while True:
        
            #while ser.in_waiting:
            # ~ print(".")
            
            # ~ responseBuffer += ser.read(12).decode('ascii' )
                
            # ~ break
                        
        # ~ print( "Buffer : " + responseBuffer )
    
    # ~ if responseBuffer[-4:] == "OKAY":
        
        responseBuffer = ""
        
        print("Waiting for OKAY...")
        
        WaitForResponse("OKAY")
        
        
        # convert addr str > int > bytes
        
        bytesAddr = int( address, 16 ).to_bytes( 4, byteorder='little', signed=False )
        
        # same as ?
    
        # ~ hexAddr = bytearray()
        
        # ~ hexAddr.append( 0x80)
        
        # ~ hexAddr.append( 0x0b)
        
        # ~ hexAddr.append( 0x14)
        
        # ~ hexAddr.append( 0x70)
        
        # ~ hexAddr.reverse()
        
        # Convert and write address bytes to serial
        
        ser.write( bytesAddr )
        
        time.sleep(.1)
        
        # Convert and write size bytes to serial
        
        bytesSize = size.to_bytes( 4, byteorder='little', signed = False )
        
        ser.write( bytesSize )
        
        time.sleep(.1)
        
        # Convert and write chekSum bytes to serial
        
        bytesChk = checkSum.to_bytes( 4, byteorder='little', signed = False )
        
        ser.write( bytesChk )
        
        time.sleep(.1)
        
        # Convert and write data bytes to serial
        
        numChunk = math.ceil( len( data ) / chunkSize )
        
        wait  = 0
        
        i = 0
        
        while i < len( data ):
            
            chunkChk = 0
            
            if ( i + chunkSize ) > len( data ):
                
                chunkSize = len( data ) - i
                
            print("Writing chunk " + str( i + 1  ) + " of " + str( numChunk ) )
        
            # we know data length is < 2048, we'd need some code to cut the data in 2K chunks in real use case
        
            # ~ print( "Input buffer b : " + str(ser.out_waiting))
            # ~ print( "Output buffer b : " + str(ser.out_waiting))
        
            # ~ for byte in range( len( data ) ):
        
            ser.write( data )
                                
                # ~ time.sleep(.005)
                
            time.sleep(.1)
            
            # Put chunk checksum calculation here
            
            # Wait for output buffer to be empty
            
            while ser.out_waiting:
                
                print("\*")
                
                wait += 1
            
            time.sleep(.1)
            
            print("Wait : "+ str(wait))
            
            # reset input buffer
            
            # ~ print( "Input buffer : " + str(ser.in_waiting))
            # ~ print( "Output buffer : " + str(ser.out_waiting))
            
            # ~ ser.reset_input_buffer()
            
            # wait for unirom to request the checksum
            
            print( "Chunk" + str( i + 1 ) + " waiting for unirom to request checksum (CHEK)..." )
            
            WaitForResponse( "CHEK" )
            
            # Wait for "CHEK" - MOVED to WaitForResponse()
            
            # ~ while True:
                
                # ~ if ser.in_waiting:
                
                    # ~ print( "Input buffer : " + str(ser.in_waiting))
                    
                    #print(".")
                    
                    # ~ chkValue = ser.read()
                    
                    # ~ # Make sure byte value is < 128 so that it can be decoded to ascii :: always getting '\xc7' 'q' '\x1c' '\xc7'  '\xab' '\xed' '1' '\x05'
                    
                    # ~ print( "chkVal : " + str(chkValue) + " - " + str( int.from_bytes(chkValue, 'big') ) )
                    
                    # ~ if int.from_bytes(chkValue, 'big') < 128:
                    
                        # ~ responseBuffer += chkValue.decode('ascii')  
                    
                    # ~ if len( responseBuffer ) > 4:
                        
                        # ~ # remove first char in buffer
                        
                        # ~ responseBuffer = responseBuffer[1:]
                    
                    # ~ print( "Response buffer : " + responseBuffer )
                    
                    # ~ if responseBuffer == "CHEK":
                        
                        # ~ print( "Got response : " + responseBuffer )
                        
                        # ~ break
            
            print( "Sending checksum to unirom..." );
            
            ser.write( bytesChk )
            
            time.sleep( .1 )
            
            # Wait for "MORE" - replace with WaitForResponse()
            
            # ~ while True:
                
                # ~ if ser.in_waiting:
                    
                    # ~ print(".")
                    
                    # ~ responseBuffer += ser.read().decode('ascii')
                    
                    # ~ if len( responseBuffer ) > 4:
                        
                        # ~ # remove first char in buffer
                        
                        # ~ responseBuffer = responseBuffer[1:]
                
                    # ~ if responseBuffer == "MORE":
                        
                        # ~ print( "Got response : " + responseBuffer )
                        
                        # ~ break
        
            WaitForResponse("MORE")
        
            print( str(i+1) + "chunk sent with correct checksum.")    
    
            i += chunkSize
    
    print("DONE!")
    
    return 0

if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv))
