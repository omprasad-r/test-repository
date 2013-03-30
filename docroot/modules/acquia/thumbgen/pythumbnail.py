#!/usr/bin/python

# Copyright (c) 2007, Tobia Conforto <tobia.conforto@gmail.com>
#
# Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is
# hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
# USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

# Versions: 0.1  2007-08-13  Initial release
#
# Based on previous work by Andrew McCall <andrew@textux.com>
# modified by Matt Biddulph <matt@hackdiary.com> - to take screenshots
# modified by Ross Burton <ross@burtonini.com> - to resize the thumbnail in memory

import os, sys, tempfile, pwd, re, fcntl, time, getopt
#from threading import *
import threading
import socket
import subprocess

sys.stderr = open('out.err','aw')

def execute(x):
  p = subprocess.Popen(x, shell=True,
    stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, close_fds=True)
  return p.stdout.read()+p.stderr.read()

class SockServ:
  def __init__(self, sock=10012):
    log('socking')
    self.sock = socket.socket()
    self.sock.bind(('',sock))
    self.sock.setblocking(False)
    self.sock.listen(5)
    log('socked on',sock)
    
  def cron(self, ongood, onbad):
    #log('cronning')
    while 1:
      try:
        conn,addr = self.sock.accept()
        conn.setblocking(0)
        buff = ''
        while 1:
          try:
            data = conn.recv(1012)
          except socket.error,e:
            break
          buff += data
        #conn.send('thanks')
        #conn.close()
        def ondone(message='thanks'):
          conn.send(str(message))
          conn.close()
        if buff=='[[quit]]':
          onbad()
        else:
          log('making shot')
          ongood(buff,ondone)
#        moz.take_screenshot(buff,ondone)
      except socket.error,e:
        break

def log(*x):
  print >> open('screenshot.log','aw'), x

import time

running = False

def run():
  sock = SockServ(10012)
  global running
  running = True
  def end():
    global running
    running = False
  def process(buff, ondone):
    url, outfile, width, height, window_width = buff.split('::')
    res = execute('./CutyCapt --url="'+url+'" --out="'+outfile+'"')
    if res:#error occurred
      print >>sys.stderr, res
      ondone()
    res = execute('/usr/bin/convert "%s" -scale %sx%s "%s"'%(outfile,width,height,outfile))
    if res:#error occurred
      print >>sys.stderr, res
      ondone()
    ondone()
  while running:
    sock.cron(process,end)
    time.sleep(.1)

## to start server: xvfb-run --server-args="-screen 0, 1600x1200x24" python pythumbnail.py &
## to kill python procs in current shell: kill -9 `ps|grep python|sed -e 's/ .*//g'`

if __name__ == "__main__":
  logtime = time.time()
  run()

