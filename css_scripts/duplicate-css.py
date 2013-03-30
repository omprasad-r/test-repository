import re
import glob
import os,sys


all_selectors = {}

def get_selectors(txt):
  txt = re.sub('\/\*.+?\*\/','',txt)
  return re.findall("^([^{\n]+){[^}]*}",txt,re.M)

def process(file):
  if 'csstidy' in file or 'jcarousel' in file or 'ajaxfileupload' in file:return
  sels = get_selectors(open(file).read())
  #print file
  for sel in sels:
    subs = sel.strip().split(',')
    for sub in subs:
      sub=sub.strip()
      if not all_selectors.has_key(sub):
        all_selectors[sub] = []
      if file not in all_selectors[sub]:
        all_selectors[sub].append(file)

def finish():
  count = 0
  for sel,files in all_selectors.items():
    if len(files)>1:
      print 'duplicate: %s'%sel
      count+=1
      for file in files:
        print '\t',file[len('cssreset/modules/acquia/'):]
  print '%d duplicates found'%count

if __name__=='__main__':
  # fix module files
  themedir = "cssreset/modules/acquia/themebuilder/"
  css = []
  for i in range(10):
      css += glob.glob(themedir + "*/"*i + "*.css")
  for file in css:
    process(file)
  finish()

