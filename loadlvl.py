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


#TODO
# - reduce/remove sleeps
# - keep listening!

import sys
import os
import serial
import time
import calendar
import math
import signal

DEBUG = 1

# Working directory

cwd = os.getcwd()

levelsFolder = cwd + os.sep + os.sep

# Receive commands from PSX
# ~ Run = True

Listen    = 1 

uniDebugMode = 0

Command = ""

memAddr = ""

flagAddr = ""

loadFile = ""

levelId  = ""

# One byte

uno = int(1).to_bytes(1, byteorder='little', signed=False)

data = 0

# ~ dataSize = 0

# Serial connection setup

ser = serial.Serial('/dev/ttyUSB0')

# Unirom can do 115200 and 510000 ( https://github.com/JonathanDotCel/NOTPSXSerial/blob/bce29e87cb858769fe60eb34d8eb123f9f36c8db/NOTPSXSERIAL.CS#L842 )

ser.baudrate = '115200'

# checkSum is the checkSum for the full data

checkSum = 0

# If set, it means the data transfer has been initiated 

Transfer = 0

# Delay between write operations. These seem to be needed for the connection not to hang.

sleepTime = 0.08    # Seems like safe minimum

def sig_interrupt_handler(signal, frame):
    
    global Run
    
    Run = False

def setDEBG():
    
    global sleepTime, ser, uniDebugMode
    
    if DEBUG:
        
        print("Sending DEBG command...")
    
    ser.write( bytes( 'DEBG' , 'ascii' ) )
    
    time.sleep(sleepTime)
                
    # Empty in waiting buffer
    
    ser.reset_input_buffer()
    
    time.sleep(sleepTime)
    
    uniDebugMode = 1

def WaitForResponse( expectedAnswer ):

    # Get incoming data from the serial port in a rolling buffer 
    # when the content of the buffer corresponds to 'expectedAnswer', returns True
    
    global DEBUG
    
    responseBuffer = ""
    
    success = False
    
    while True:
        
        if DEBUG > 1:
            
            print("Waiting for data in serial input buffer.")
        
        # If data in serial's incoming buffer
        
        if ser.in_waiting:
        
            if DEBUG > 1:
            
                print("Brace yourself, data is coming...")
            
            # Read 1 byte
        
            byteValue = ser.read(1)
            
            # Make sure byte value is < 128 so that it can be decoded to ascii
            
            if byteValue[0] < 128:
            
                responseBuffer += byteValue.decode('ascii')  
            
            else:
                
                responseBuffer += '.'
            
            # Always keep response buffer 4 chars long
            
            if len( responseBuffer ) > 4:
                
                # Remove first char in buffer
                
                responseBuffer = responseBuffer[1:]
            
            
            # If response is ERR!, checksum check does not check, so check it again
            
            if responseBuffer == "ERR!":
                
                if DEBUG > 1:
            
                    print("Checksum error !")
                
                success = False
                
                break
                
            # When expected response shows up, break from the while loop
            
            if responseBuffer == expectedAnswer:
                
                success = True
                
                break
    if DEBUG  > 1:
    
        print( "Got : " + responseBuffer )
    
    responseBuffer = ""
    
    return success
    
def CalculateChecksum( inBytes, skipFirstSector = False):
    
    returnVal = 0;
    
    i = 0
    
    if skipFirstSector:
        
         i = 2048
    
    while i < len( inBytes ):

        returnVal += inBytes[i];
        
        i += 1
    
    return returnVal;

def WriteBytes( inData ):
    
    if DEBUG:
        
        print("Preparing to write bytes...")
        
    # The data needs to be split in 2K chunks 
    
    chunkSize = 2048
    
    # BEGIN WHILE DATA
    
    i = 0
    
    while i < len( inData ):
        
        # BEGIN WHILE TRUE
        
        while True:
            
            # BEGIN TRY/EXCEPT
            
            try:
                    
                # Calculate number of 2K chunks we're about to send
                
                numChunk = math.ceil( len( inData ) / chunkSize )
                
                # Calculate current chunk
                
                currentChunk = math.ceil( (i + 1) / chunkSize)                
                
                if DEBUG:
                    
                    print( str ( numChunk + 1 - currentChunk ) + " chunks of " + str ( chunkSize) + "bytes to send " )
                
                # Avoid going out of range
                
                if ( i + chunkSize ) > len( inData ):
                    
                    chunkSize = len( inData ) - i
                    
                print("Writing chunk " + str( currentChunk ) + " of " + str( numChunk ) )
                                
                # ~ ser.write(inData)

                chunkChecksum = 0
                
                # Send inData in 2048B chunks
                
                for byte in range( chunkSize ):
                    
                    # Send byte
                    
                    ser.write( inData[ i + byte ].to_bytes(1, byteorder='little', signed=False) )
                    
                    # Calculate chunk checksum
                    
                    chunkChecksum += inData[ i + byte ]
                                            
                time.sleep(sleepTime)
                
                if DEBUG:                        
                
                    print( "Chunk cheksum : " + str( chunkChecksum ) ) 
                
                # Wait for output buffer to be empty
                # REMOVE ? Is this needed ?
                
                while ser.out_waiting:
                    
                    print("*")
                    
                    wait += 1
                
                time.sleep(sleepTime)
                
                # Wait for unirom to request the checksum
                
                if DEBUG > 1:
                
                    print( "Chunk " + str( currentChunk ) + " waiting for unirom to request checksum (CHEK)..." )
                
                WaitForResponse( "CHEK" )
                
                # Send checksum
                
                if DEBUG:
                
                    print( "Sending checksum to unirom..." );
                
                # ~ chunkChecksum = 170
                
                bytesChunkChecksum = chunkChecksum.to_bytes( 4, byteorder='little', signed = False )
                
                ser.write( bytesChunkChecksum )
                
                # ~ time.sleep( sleepTime )
            
                if DEBUG > 1:
                    
                    print( "Waiting for unirom to request more data (MORE)..." )
            
                # Wait for unirom to request MORE inData ( next chunk )
            
                if not WaitForResponse("MORE"):
                    
                    if DEBUG:
                        
                        print("ERROR ! Retrying...")

                    raise Exception()
            
                if DEBUG:
                    
                    print( str( currentChunk ) + " chunk sent with correct checksum.")
        
                # Increment i from chunkSize
        
                i += chunkSize
                
            except Exception:
                
                continue
            
            # END TRY/EXCEPT
            
            break
        
        # END WHILE TRUE
    
        numChunk = 0
    
    # END WHILE DATA

def SendBin( inData, memAddr ):
    
    global sleepTime 
    
    dataSize = len( inData )
    
    if DEBUG:
        
        print("Data size : " + str( dataSize ) )
    
    # Prepare unirom for data reception - sent "SBIN" - received : "OKV2"
    
    if DEBUG > 1:
        
        print("Sending SBIN command...")
    
    ser.write( bytes( 'SBIN' , 'ascii' ) )
    
    time.sleep(sleepTime)
    
    # We're using unirom in debug mode, which means protocol version 2 is available
    # Upgrade protocol  - sent "UPV2" - received : "OKAY"
    
    ser.write( bytes( 'UPV2' , 'ascii' ) )

    time.sleep(sleepTime)

    # Initialisation done, set flag
    
    # ~ Init = 1
    
    # From now on, we're using the rolling buffer
    if DEBUG  > 1:
        
        print("Waiting for OKAY...")
    
    WaitForResponse("OKAY")
    
    # Calculate data checkSum

    checkSum = CalculateChecksum( inData )

    if DEBUG :
        
        print("Data checkSum : " + str(checkSum) )
    
    # Send memory address to load data to, size of data and checkSum
    # Unirom expects unsigned longs ( 32bits ), byte endianness little
    
    # Convert address from string to integer, then to ulong 32b
    
    bytesAddr = int( memAddr, 16 ).to_bytes( 4, byteorder='little', signed=False )
    
    # Write address to serial
    
    ser.write( bytesAddr )
    
    time.sleep(sleepTime)
    
    # Convert and write int size to serial
    
    bytesSize = dataSize.to_bytes( 4, byteorder='little', signed = False )
    
    ser.write( bytesSize )
    
    time.sleep(sleepTime)
    
    # Convert and write int chekSum to serial
    
    bytesChk = checkSum.to_bytes( 4, byteorder='little', signed = False )
    
    ser.write( bytesChk )
    
    time.sleep(sleepTime)

    # Send dat data
    
    WriteBytes( inData )

def resetListener():
    
    global checkSum, data, Listen, Transfer, dataSize, memAddr, loadFile, flagAddr
    
    memAddr = ""

    flagAddr = ""
    
    loadFile = ""
    
    checkSum = 0
    
    data = 0
    
    dataSize = 0
    
    Transfer = 0
    
    Listen = 1

    ser.reset_input_buffer()
    
    ser.reset_output_buffer()
    
def main(args):
    
    while True:
    
        global checkSum, data, Listen, Transfer, dataSize, memAddr, loadFile, flagAddr, levelId
        
        # Flush serial buffers to avoid residual data
        
        ser.reset_input_buffer()
        
        ser.reset_output_buffer()
        
        inputBuffer = ""
            
        # Listen to incomming connections on serial
        
        if Listen:

            print("Listening for incoming data...")
            
            if DEBUG  > 1:
            
                print("memAddr : " + str(memAddr) + " - loadFile" + loadFile )
            
            while True:

                # If data on serial, fill buffer
                
                while ser.in_waiting:
                    
                    inputBuffer += ser.read().decode('ascii')
                
                if inputBuffer:
                    
                    if DEBUG:
                    
                        print( "Incoming data : " + inputBuffer )
                    
                    # parse command CMD:ARG1:ARG2(:ARGn)
                    
                    argList = []
                    
                    argList = inputBuffer.split(':')
                
                    # Send command
                
                    if argList[0] == "load" and len(argList) == 4:
                        
                        if len(argList[1]) < 8 or len(argList[2]) < 8:
                            
                            if DEBUG:
                                
                                print("Wrong data format, aborting...")
                            
                            break
                            
                        memAddr   = argList[1]
                    
                        flagAddr  = argList[2]
                    
                        loadFile  = argList[3]
                            
                        ser.reset_input_buffer()
                        
                        inputBuffer = ""
                        
                        if DEBUG > 1:
                        
                            print( memAddr + " - " + flagAddr + " - " + loadFile )
                        
                        Listen = 0
                        
                        break
        
                    else:
                        
                        ser.reset_input_buffer()
                        
                        inputBuffer = ""
                        
                        break
        
        if memAddr and loadFile:
        
            # Remove separator and ';1' at end of the string
        
            fileClean = loadFile.split(';')[0][1:]

            print("Received addresses and filename : " + memAddr + " - " + flagAddr + " - " + fileClean)
            
            # TODO : replace with a proper level naming scheme
            # right now, we're receiving currently loaded file
            # so we have to switch manually here.
            
            binFileName = ""
            
            if fileClean == "level0.bin":
            
                binFileName = "Overlay.lvl1" 
            
                levelId     = 1
            
            if fileClean == "level1.bin":
                
                binFileName = "Overlay.lvl0"
            
                levelId     = 0
            
            if DEBUG:

                print(
                
                    "Load Data to : " + memAddr + "\n" +
                    
                    "Reset flag at: " + flagAddr + "\n" +
                    
                    "File   : " + loadFile + "\n" +
                    
                    "Bin    : " + binFileName + "ID : " + str(levelId)
            
                     )
            
            # Open file as binary if bin filename is defined
            
            if binFileName:

                binFile = open( levelsFolder + binFileName, 'rb' )
            
                data = binFile.read()
            
                Transfer = 1
            
            else:
                
                print(" No filename provided, doing nothing ")
                
                resetListener()
        
        # If Init was set, initialize transfer and send data
        
        if Transfer:
            
            print("Initializing data transfer...")
            
            if not uniDebugMode:
            
                # Set unirom to debugmode - sent : "DEBG" - received : "DEBGOKAY"
            
                setDEBG()
            
            # Send level data
            
            SendBin( data, memAddr )

            # Set level changed flag 
            
            SendBin( levelId.to_bytes(1, byteorder='little', signed=False) , flagAddr)
            
            # Reset everything 
            
            resetListener()
            
            print("DONE!")
    
    return 0

if __name__ == '__main__':
    import sys
    sys.exit(main(sys.argv))
