#!/usr/bin/python
# VLBI Server program
# Written by Uwe Bach 2010

import sys, string, fileinput, os, serial, time
import tty
from select import select
from socket import *


########################
#    Preliminaries     #
########################

#
# Procedure to find right bit for SDC to switch in phcal
#
class NotTTYException(Exception): pass

class TerminalFile:
    def __init__(self,infile):
        if not infile.isatty():
            raise NotTTYException()
        self.file=infile

        #prepare for getch
        self.save_attr=tty.tcgetattr(self.file)
        newattr=self.save_attr[:]
        newattr[3] &= ~tty.ECHO & ~tty.ICANON
        tty.tcsetattr(self.file, tty.TCSANOW, newattr)

    def __del__(self):
        #restoring stdin
        import tty  #required this import here
        tty.tcsetattr(self.file, tty.TCSADRAIN, self.save_attr)

    def getch(self):
        if select([self.file],[],[],0)[0]:
            c=self.file.read(1)
        else:
            c=''
        return c

def str2hms(inp, flag):

    ins = inp.split(":")
    out = ""
    if len(ins) == 1:  
       return inp
    for i in range(len(ins)):
        if i == 0:
           if flag: out = ins[i] + "h"
           else: out = ins[i] + "d"
        elif i == 1:
           if flag: out += ins[i] + "m" 
           else: out += ins[i] + "'" 
        elif i == 2:
           if flag: out += ins[i] + 's'
           else: out += ins[i] + '"'

    return out

def pcalon(sumlo):
        #Find pcal-code for current RX
        #
	# 18/21cm PFK
        if sumlo > 900 and sumlo < 1750:
                pcalcode="1"
		rx="P200mm"
	# 5cm PFK
        elif sumlo > 5010 and sumlo < 6000:
                pcalcode="0"
		rx="P50mm"
	# 3mm PFK
        elif sumlo > 84000 and sumlo < 96000:
                pcalcode="0"
		rx="P3mm"
	# 6cm SFK
        elif sumlo > 4000 and sumlo < 5000:
                pcalcode="i"
		rx="S60mm"
	# 3.6cm SFK
        elif sumlo > 7100 and sumlo < 9000:
                pcalcode="m"
		rx="S36mm"
	# 2cm bis 7mm SFK
        elif sumlo > 12000 and sumlo < 15500:
                pcalcode="k"
		rx="S20mm"
	# 1.3cm SFK
        elif sumlo > 17000 and sumlo < 25000:
                pcalcode="k"
		rx="S13mm"
	# 7mm SFK
        elif sumlo > 40000 and sumlo < 45000:
                pcalcode="k"
		rx="S7mm"
        else:
	        pcalcode="@"
		rx="RX unknown"
	return pcalcode,rx


########################
#    Program starts    #
########################

#be4Sock.close()
#mk4fsSock.close()
#ser.close()
#time.sleep(1)
#
# Set the socket parameters for receiving
# VLBI Tests
#
rhost = ""
#host = "localhost"
# VLBI Tests
rport = 51381
buf = 1024
raddr = (rhost,rport)

# Create socket and bind to address
fsSock = socket(AF_INET,SOCK_DGRAM)
fsSock.bind(raddr)
fsSock.settimeout(10)
#
# Set the socket parameters for sending to be4
# VLBI Tests
#
#shost = "134.104.65.78"
shost = "134.104.64.134"
# VLBI Tests
sport = 23456
saddr = (shost,sport)

# Create socket
be4Sock = socket(AF_INET,SOCK_DGRAM)

#
# Set the socket parameters for sending pcal to S197
#
#shost = "134.104.65.78"
S197host = "134.104.64.17"
# VLBI Tests
S197port = 10001
S197addr = (S197host,S197port)

# Create socket

# for getch() 
s=TerminalFile(sys.stdin)

#
# Setting for restart
#
oldstring=">>SV NONE NONE 00h00m00.0s 00d00'00\" -1.0 1 0 1950FSX"
#oldstring="NEW"
osource="NONE"
oRA="00h00m00.0s"
oDEC="00d00'00\""
osumlo=0
#ophascal="0"
otcal=0
ophascal=0
#
# Endless loop to wait for incoming massages
# and send them to be4 or process locally (pcal,tcal)
#
while 1:
        outfile=open('VLBISend.log','a')
        try:
                data,addr = fsSock.recvfrom(buf)
                if not data:
                        print "Client has exited!"
                        break
                t=time.localtime()
                print "\n%4d.%02d.%02d %02d:%02d:%02d, received: %s" % (t[0],t[1],t[2],t[3],t[4],t[5],data)
                outfile.write("\n%4d.%02d.%02d %02d:%02d:%02d, received: %s" % (t[0],t[1],t[2],t[3],t[4],t[5],data))
        #        print "\nThis came from mk4fs: %s" % data
                if (string.find(data[0:2],">>"))!=-1:
                        newline=string.split(data)
                        ostype=data[2:4]
                        source=newline[2]
                        RA=str2hms(newline[3],1)
                        DEC=str2hms(newline[4],0)
                        sumlo=float(newline[5])
                        tcal=newline[6]
                        phascal=newline[7]
#			print "Bis hier %s %s %s %s %s %.1f %s %s %s" % (newline[0],newline[1],source,RA,DEC,sumlo,tcal,phascal,newline[8])
                        data="%s %s %s %s %s %.1f %s %s %s" % (newline[0],newline[1],source,RA,DEC,sumlo,tcal,phascal,newline[8])
                        if oldstring!=data:
                                print "Something has changed:"
                                outfile.write("\nSomething has changed:")
#                                print "old: %s  new: %s" % (oldstring,data)
#                                outfile.write("\nold: %s  new: %s" % (oldstring,data))
                                if source!=osource or RA!=oRA or DEC!=oDEC:
                                        print "new source: send %s to be4" % data
                                        outfile.write("\nnew source: send %s to be4" % data)
                                        be4Sock.sendto(data,saddr)
                                elif sumlo!=osumlo:
                                        print "new sumlo: send %s to be4" % data
                                        outfile.write("\nnew sumlo: send %s to be4" % data)
                                        be4Sock.sendto(data,saddr)
                                if tcal!=otcal:
                                        print "Switch tcal to new state"
                                        outfile.write("\nSwitch tcal to new state")
                                        if tcal=="1":
                                                print "switch on"
                                                outfile.write("\nswitch on")
#                                                mk4Sock.sendto(data,mk4addr)
                                        if tcal=="0":
                                                print "switch off"
#                                                mk4Sock.sendto(data,mk4addr)
                                if phascal!=ophascal or sumlo!=osumlo or tcal!=otcal:
#                                        print tcal,ostype
                                        if tcal=="1" and ostype=="SM":
                                                print "DBBC caltsys"
                                        else:
					        try:
					          S197Sock = socket(AF_INET,SOCK_STREAM)
                                                  S197Sock.connect(S197addr)
                                                  print "Switch phascal to new state"
                                                  if phascal=="1":
						          pcal=pcalon(sumlo)
							  print sumlo,pcal
                                                          print "switch on pcal for %s" % (pcal[1])
                                                          S197Sock.send(pcal[0])
                                                          outfile.write("\nswitch on pcal for %s" % (pcal[1]))
                                                  if phascal=="0":
                                                          print "switch off"
							  pcal='@'
                                                          S197Sock.send(pcal)
                                                          outfile.write("\nswitch off")
						  S197Sock.close()
						except:
						  print "Could not cconnect to S197"
                                oldstring=data
                                newline=string.split(oldstring)
                                osource=newline[2]
                                oRA=newline[3]
                                oDEC=newline[4]
                                osumlo=float(newline[5])
                                otcal=newline[6]
                                ophascal=newline[7]
                        else:
                                print "no new string: do nothing"
#                                print "old: %s = new: %s" % (oldstring,data)
#                                       print "send anyway"
#                                     be4Sock.sendto(data,saddr)
                                outfile.write("\nno new string: do nothing")
#                                outfile.write("\nold: %s = new: %s" % (oldstring,data))
                                oldstring=data
                                newline=string.split(oldstring)
                                osource=newline[2]
                                oRA=newline[3]
                                oDEC=newline[4]
                                osumlo=float(newline[5])
                                otcal=newline[6]
                                ophascal=newline[7]
                elif (string.find(data[0:2],"SM"))!=-1:
                        newline=string.split(data)
                        type=newline[1]
                        offset=newline[2]
                        if type=="OFFAZM":
                                azm=type
                                azmoff=offset
                        if type=="OFFELV":
                                print "FS cal:SM FSCAL %s %7s  %s %7s" % (azm,azmoff,type,offset)
                                outfile.write("\nFS cal:SM FSCAL %s %7s  %s %7s" % (azm,azmoff,type,offset))
                                line="SM FSCAL %s %7s  %s %7s" % (azm,azmoff,type,offset)
                                be4Sock.sendto(line,saddr)
                        oldstring=data
                        osource="onoff"
                elif (string.find(data,"POINTING"))!=-1:
                        print "Send: '>>SV POINTING' to be4"
                        outfile.write("\nSend: '>>SV %s' to be4" % data)
                        line=">>SV %s" % data
                        be4Sock.sendto(line,saddr)
                elif (string.find(data,"REPEAT"))!=-1:
                        print "Resend %s to be4" % oldstring
                        outfile.write("\nResend %s to be4" % oldstring)
                        line="%s" % oldstring
                        be4Sock.sendto(line,saddr)
                else:
                        print "non of the known cases"
                        outfile.write("\nnon of the known cases")
        except:
                t=time.localtime()
                date="%d.%02d.%02d %02d:%02d:%02d" % (t[0],t[1],t[2],t[3],t[4],t[5])
                if s.getch()=="r":
                        print "%s: repeat last command: %s" % (date,oldstring)
                        outfile.write("\n%s: repeat last command: %s" % (date,oldstring))
                        be4Sock.sendto(oldstring,saddr)
                if s.getch()=="q":
                        print "quit server"
                        outfile.write("\nquit server")
                        break
                continue
        outfile.close()
# Close socket and ttyS4
be4Sock.close()
fsSock.close()
