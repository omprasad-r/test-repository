#!/usr/bin/env python

import socket

import sys

'''
    send the "kill" signal to the screenshot server.
'''
def kill(port):
    s=socket.socket()
    s.connect(('',port))
    s.send('[[quit]]')
    res = s.recv(4069)
    if res != 'thanks':
        raise Exception,'invalid server response',res
    s.close()

docs = '''control.py for controlling the server. currently just kills it.
usage:
  ./control.py kill'''

if __name__=='__main__':
  if len(sys.argv)<2:
    print docs
    sys.exit()
  cmd = sys.argv[1]
  if cmd == 'kill':
    kill(10012)
  else:
    print docs

