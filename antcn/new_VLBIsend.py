#!/usr/bin/python
# VLBI Server program
# Written by Uwe Bach 2010

import sys, string, fileinput, os, time
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

def send_wait_reply(ss, str):
    if (_VERBOSE):
        print 'Request: sending %s' % (str)
    ss.send(str + '\n')
    if (_VERBOSE):
        print 'Response: ',
    response = ss.recv(1024)
    response = string.strip(response)
    if (_VERBOSE):
        print 'result = %s' % (response[-1:])
    return (response)

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
        elif sumlo > 12000 and sumlo < 16900:
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

def matchlo(sumlo):
        #Find VLBI Setup for current RX
        #
    # 3.6cm SFK
        if sumlo==8110:
          rxsetup="S36mm_13cm_geo_vlbi"
          IFfreq=150
    # 50cm PFK
        elif sumlo == 157:
          rxsetup="P500mm_cont_spec_50MHz"
          IFfreq=175
    # 50cm PFK
        elif sumlo == 177:
          rxsetup="P500mm_cont_spec_100MHz"
          IFfreq=150
    # 21cm PFK
        elif sumlo >= 1160 and sumlo <= 1280:
          #rxsetup="P210mm_vlbi"
          rxsetup="P217mm_vlbi"
          IFfreq=150
    # 18cm PFK
        elif sumlo >= 1460 and sumlo <= 1520:
          rxsetup="P180mm_vlbi"
          IFfreq=150
    # 18cm PFK VLBA
        elif sumlo >= 1050 and sumlo <= 1120:
          rxsetup="P180mm_vlbi"
          IFfreq=550
    # 6cm SFK
        elif sumlo >= 4000 and sumlo <= 5000:
          rxsetup="S60mm_vlbi"
          #rxsetup="S45mm_vlbi4950"
          IFfreq=750
    # 5cm PFK
        elif sumlo==5300:
          rxsetup="S45mm_vlbi6050"
          IFfreq=750
    # 5cm PFK
        elif sumlo>=5400 and sumlo<=6000:
          rxsetup="P50mm_vlbi"
          IFfreq=750
    # 3.6cm SFK
        elif sumlo >= 7500 and sumlo <= 7900:
          rxsetup="S36mm_vlbi"
          IFfreq=750
    # 5cm SFK
        elif sumlo>=7940 and sumlo <=8050:
          rxsetup="S45mm_vlbi6750"
          IFfreq=-1250
    # 2cm SFK
        elif sumlo >= 11300 and sumlo <= 14900:
          rxsetup="S20mm_vlbi-low"
          IFfreq=750
    # 2cm SFK
        elif sumlo >= 15000 and sumlo <= 17900:
          rxsetup="S20mm_vlbi-high"
          IFfreq=-750
    # 1.3cm SFK
        elif sumlo >= 18000 and sumlo <= 21600:
          rxsetup="S14mm_vlbi"
          IFfreq=750
    # 1.3cm SFK
        elif sumlo >= 21650 and sumlo <= 26000:
          rxsetup="S14mm_vlbi-high"
          IFfreq=-750
    # 7mm SFK
        elif sumlo >= 40000 and sumlo <= 44000:
          rxsetup="S7mm_vlbi"
          IFfreq=750
    # 3mm PFK
        elif sumlo >= 80000 and sumlo <= 95000:
          rxsetup="P3mm_vlbi"
          IFfreq=750
        else:
          rxsetup="NONE"
          IFfreq=750
        return rxsetup,IFfreq

def ask_fiba(cmd):
    #print "ask_fiba"
    #udpUserSock  = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    #udpUserSock.settimeout(TIMEO)
    tcpCliSock = socket(AF_INET, SOCK_STREAM)
    error = tcpCliSock.connect_ex(FIBAADDR)
    tcpCliSock.settimeout(TIMEO)
    if error != 0:
        print "error connecting FiBa ", FIBAADDR
        return None 
    #
    if verboseMode: print cmd
    tcpCliSock.send(cmd)
    Data = ""
    n = 0
    if cmd == 'X99ZZZZ': dlen = 16*28
    else: dlen = 5
    while len(Data) < dlen:
        try:
            Data+=tcpCliSock.recv(1)
            n += 1
        except:
            print "FiBa timeout received"
            tcpCliSock.close()
            return None 
    if verboseMode: print Data
    tcpCliSock.shutdown(SHUT_RDWR)
    tcpCliSock.close()
    return Data

def set_fiba(sumlo):
# new K-band
#  if sumlo > 18000 and sumlo < 26000:
  if sumlo==24048.0:
    filter_req="033"
    filter08="S08033Z"
    filter16="S16033Z"
#  elif sumlo==7700.0 or sumlo==43870.0:
  elif sumlo < 1600.0:
    filter_req="161"
    filter08="S08161Z"
    filter16="S16161Z"
  elif sumlo == 8110.0:
    filter_req="161"
    filter08="S08161Z"
    filter16="S16161Z"
  elif sumlo == 8000.0:
    filter_req="185"
    filter08="S08185Z"
    filter16="S16185Z"
  elif sumlo == 7950.0:
    filter_req="161"
    filter08="S08161Z"
    filter16="S16161Z"
  elif sumlo == 4088.0:
    filter_req="161"
    filter08="S08161Z"
    filter16="S16161Z"
  elif sumlo == 85500.0:
    filter_req="161"
    filter08="S08161Z"
    filter16="S16161Z"
  elif sumlo == 5300.0:
    filter_req="185"
    filter08="S08185Z"
    filter16="S16185Z"
# other receivers
  else:
    filter_req="185"
    filter08="S08185Z"
    filter16="S16185Z"
  tmpfiba=ask_fiba("X99ZZZZ")
  if tmpfiba is not None:
    attenFiba08=float(tmpfiba.split("\n")[7].split(";")[2].split("dB")[0])
    attenFiba16=float(tmpfiba.split("\n")[15].split(";")[2].split("dB")[0])
    modeFiba08=tmpfiba.split("\n")[7].split(";")[-1].replace("\r","")
    modeFiba16=tmpfiba.split("\n")[15].split(";")[-1].replace("\r","")
    if modeFiba08!=filter_req:
      time.sleep(2)
      tmpfiba=ask_fiba(filter08)
      time.sleep(2)
      tmpfiba=ask_fiba(filter16)
      time.sleep(2)
      tmpfiba=ask_fiba("X99ZZZZ")
      attenFiba08=float(tmpfiba.split("\n")[7].split(";")[2].split("dB")[0])
      attenFiba16=float(tmpfiba.split("\n")[15].split(";")[2].split("dB")[0])
      modeFiba08=tmpfiba.split("\n")[7].split(";")[-1].replace("\r","")
      modeFiba16=tmpfiba.split("\n")[15].split(";")[-1].replace("\r","")
#    print "\n VLBI MulitFiBa mode is set to: Ch08=%s Ch16=%s\n Required Filter=%s" %  (modeFiba08,modeFiba16, filter_req)
    return modeFiba08,modeFiba16, filter_req
  else:
    print "error connecting FiBa ", FIBAADDR
    return "error","error","error"

def instcfg(source):
    pulsarmode="NONE"
    try:
        f=open('/usr2/sched/pulsar_sources.txt')
        all=f.readlines()
        f.close()
    except:
        print "\n File \"%s\" not found\n" % schfile
        sys.exit()
    for i in range(len(all)):
        target,mode=all[i].split()
        if source.upper()==target.upper():
            pulsarmode=mode
    return pulsarmode

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
#shost = "134.104.65.79"
shost = "134.104.64.134"
# VLBI Tests
sport = 23456
saddr = (shost,sport)

# Create socket
#be4Sock = socket(AF_INET,SOCK_DGRAM)

#
# Set the socket parameters for sending pcal to S197
#
#shost = "134.104.65.78"
# Debug
_VERBOSE=True
verboseMode=None
#
S197host = "134.104.64.17"
# VLBI Tests
S197port = 10001
S197addr = (S197host,S197port)

S275host = "134.104.64.41"
# VLBI Tests
S275port = 10001
S275addr = (S275host,S275port)

FIBAHOST = '134.104.64.123'
FIBAPORT = 10001
FIBAADDR = (FIBAHOST, FIBAPORT)
TIMEO = 3
BUFFER = 16384

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
osumlostr='NONE'
#ophascal="0"
otcal="0"
ophascal=0
pulsarmode="NONE"
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
#    print "\nThis came from mk4fs: %s" % data
    if (string.find(data[0:2],">>"))!=-1:
      be4Sock = socket(AF_INET,SOCK_DGRAM)
      newline=string.split(data)
      ostype=data[2:4]
      source=newline[2]
      RA=str2hms(newline[3],1)
      DEC=str2hms(newline[4],0)
      sumlo=float(newline[5])
      sumlostr,IFfreq=matchlo(sumlo)
      tcal=newline[6]
      phascal=newline[7]
      pulsarmode=instcfg(source)
#      data="%s %s %s %s %s %s %s %s %s %s" % (newline[0],newline[1],source,RA,DEC,sumlo,tcal,phascal,newline[8],sumlostr)
#      data="%s %s %s %s %s %s %s %s %s" % (newline[0],newline[1],source,RA,DEC,sumlostr,tcal,phascal,newline[8])
      data="%s %s %s %s %s %s@%s %s %s %s %s" % (newline[0],newline[1],source,RA,DEC,sumlo+IFfreq,sumlostr,tcal,phascal,newline[8],pulsarmode)
#      print sumlostr,oldstring,data,source,sumlo
      if oldstring!=data and source!="NONE" and sumlo!=-1.0:
        print "Something has changed:"
        outfile.write("\nSomething has changed:")
        print "old: %s  new: %s" % (oldstring,data)
#        outfile.write("\nold: %s  new: %s" % (oldstring,data))
#        print sumlo,osumlo
        if sumlo!=osumlo:
          print "new sumlo: send %s to be4" % data
          outfile.write("\nnew sumlo: send %s to be4" % data)
          be4Sock.sendto(data,saddr)
          try:
            S197Sock = socket(AF_INET,SOCK_STREAM)
            S197Sock.connect(S197addr)
            print "Switch phascal to new state"
            if phascal=="1":
              pcal=pcalon(sumlo)
#              print sumlo,pcal
              print "switch on pcal for %s" % (pcal[1])
              rstr = send_wait_reply(S197Sock,pcal[0])
              outfile.write("\nswitch on pcal for %s" % (pcal[1]))
            if phascal=="0":
              print "switch off"
              pcal='@'
              rstr = send_wait_reply(S197Sock,pcal)
              outfile.write("\nswitch off")
            S197Sock.close()
          except:
            print "Could not connect to S197"
          try:
            S275Sock = socket(AF_INET,SOCK_STREAM)
            S275Sock.connect(S275addr)
            print "Switch IF box"
            if sumlo == 24048.0:
  #               Switch to narrow band
              new="j"
  #                new="5"
  #               print sumlo,"Input 2"
              print sumlo,"Input 1"
              rstr = send_wait_reply(S275Sock,new)
              outfile.write("\nswitch to IF input 2")
            elif sumlo == 8110.0:
  #               Switch to narrow band
              new="i"
              print sumlo,"Input SX"
              rstr = send_wait_reply(S275Sock,new)
              outfile.write("\nswitch to IF input SX")
            else:
  #              Switch to VLBA IF
              new="5"
              print sumlo,"Input 1"
              rstr = send_wait_reply(S275Sock,new)
              outfile.write("\nswitch to IF input 1")
            S275Sock.close()
          except:
            print "Could not connect to S275"
          fiba=set_fiba(sumlo)
          print "\n VLBI MulitFiBa mode is set to: Ch08=%s Ch16=%s\n Required Filter=%s" %  (fiba[0],fiba[1],fiba[2])
          outfile.write("\n VLBI MulitFiBa mode is set to: Ch08=%s Ch16=%s\n Required Filter=%s" %  (fiba[0],fiba[1],fiba[2]))
        elif source!=osource or RA!=oRA or DEC!=oDEC:
          print "new source: send %s to be4" % data
          outfile.write("\nnew source: send %s to be4" % data)
          be4Sock.sendto(data,saddr)
          try:
            S275Sock = socket(AF_INET,SOCK_STREAM)
            S275Sock.connect(S275addr)
            print "Switch IF box"
            if sumlo == 24048.0:
  #               Switch to narrow band
              new="j"
  #                new="5"
  #               print sumlo,"Input 2"
              print sumlo,"Input 1"
              rstr = send_wait_reply(S275Sock,new)
              outfile.write("\nswitch to IF input 2")
            elif sumlo == 8110.0:
  #               Switch to narrow band
              new="i"
              print sumlo,"Input SX"
              rstr = send_wait_reply(S275Sock,new)
              outfile.write("\nswitch to IF input SX")
            else:
  #              Switch to VLBA IF
              new="5"
              print sumlo,"Input 1"
              rstr = send_wait_reply(S275Sock,new)
              outfile.write("\nswitch to IF input 1")
            S275Sock.close()
          except:
            print "Could not connect to S275"
        elif tcal!=otcal:
          print "Switch tcal to new state"
          outfile.write("\nSwitch tcal to new state")
          if tcal=="1":
            tcal="11"
            #data="%s %s %s %s %s %s@%s %s %s %s" % (newline[0],newline[1],source,RA,DEC,sumlo+IFfreq,sumlostr,tcal,phascal,newline[8])
            data="%s %s %s %s %s %s@%s %s %s %s %s" % (newline[0],newline[1],source,RA,DEC,sumlo+IFfreq,sumlostr,tcal,phascal,newline[8],pulsarmode)
            print "switch on"
            outfile.write("\nswitch on")
            be4Sock.sendto(data,saddr)
#            mk4Sock.sendto(data,mk4addr)
          if tcal=="0":
            tcal="10"
            #data="%s %s %s %s %s %s@%s %s %s %s" % (newline[0],newline[1],source,RA,DEC,sumlo+IFfreq,sumlostr,tcal,phascal,newline[8])
            data="%s %s %s %s %s %s@%s %s %s %s %s" % (newline[0],newline[1],source,RA,DEC,sumlo+IFfreq,sumlostr,tcal,phascal,newline[8],pulsarmode)
            print "switch off"
            be4Sock.sendto(data,saddr)
            tcal="0"
            #3mm source fix!
            source="NONE"
        elif phascal!=ophascal:
          try:
            S197Sock = socket(AF_INET,SOCK_STREAM)
            S197Sock.connect(S197addr)
            print "Switch phascal to new state"
            if phascal=="1":
              pcal=pcalon(sumlo)
#              print sumlo,pcal
              print "switch on pcal for %s" % (pcal[1])
              rstr = send_wait_reply(S197Sock,pcal[0])
              outfile.write("\nswitch on pcal for %s" % (pcal[1]))
            if phascal=="0":
              print "switch off"
              pcal='@'
              rstr = send_wait_reply(S197Sock,pcal)
              outfile.write("\nswitch off")
            S197Sock.close()
          except:
            print "Could not connect to S197"
        oldstring=data
        newline=string.split(oldstring)
        newline2=string.split(newline[5],"@")
        osource=source
        oRA=newline[3]
        oDEC=newline[4]
        osumlo=float(newline2[0])-IFfreq
        osumlostr=sumlostr
        otcal=tcal
        ophascal=newline[7]
      else:
        print "no new string: do nothing"
        outfile.write("\nno new string: do nothing")
        oldstring=data
        newline=string.split(oldstring)
        newline2=string.split(newline[5],"@")
#        print newline2
        otcal=newline[6]
        osource=newline[2]
        if osource=="NONE":
          otcal="0"
        oRA=newline[3]
        oDEC=newline[4]
        osumlo=float(newline2[0])-IFfreq
        osumlostr=sumlostr
        if osumlo==-1.0:
          otcal="0"
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
    elif (string.find(data,"SETFIBA"))!=-1:
      fiba=set_fiba(sumlo)
      print "\n VLBI MulitFiBa mode is set to: Ch08=%s Ch16=%s\n Required Filter=%s" %  (fiba[0],fiba[1],fiba[2])
      outfile.write("\n VLBI MulitFiBa mode is set to: Ch08=%s Ch16=%s\n Required Filter=%s" %  (fiba[0],fiba[1],fiba[2]))
#                elif (string.find(data,"@"))!=-1:
#                        print "Tracking list %s send to be4" % data
#                        outfile.write("\nResend %s to be4" % data)
#                        line="%s" % data
#                        be4Sock.sendto(line,saddr)
    else:
      print "non of the known cases"
      outfile.write("\nnon of the known cases")
    be4Sock.shutdown()
    be4Sock.close()
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
