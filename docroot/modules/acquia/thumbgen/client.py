#!/usr/bin/env python

import socket
import sys
import getopt

'''
    connect to the screenshot server to schedule a screenshot.
    currently sends the desired url and the output file -- will be
    modified in the future to include auth tokens, etc.
'''
def triggershot(*args):
  port = args[0]
  s=socket.socket()
  s.connect(('',port))
  s.send('::'.join(str(arg) for arg in args[1:]))
  res = s.recv(4069)
  if res != 'thanks':
    raise Exception,'invalid server response',res
  s.close()

docs = '''client.py for triggering screenshots with the server
usage:
  python client.py http://example.com outfile.png -p port=1012 -w width=200 -h height=150 -f full_width=1024
'''

if __name__=='__main__':
  optlist, args = getopt.gnu_getopt(sys.argv[1:], 'p:w:h:f:')
  if len(args)!=2:
    print docs
    sys.exit()
  url, outfile = args
  opts = dict(optlist)
  port = int(opts.get('-p',10012))
  width = int(opts.get('-w', 200))
  height = int(opts.get('-h', 150))
  window_width = int(opts.get('-f', 1024));
  
  triggershot(port, url, outfile, width, height, window_width)
