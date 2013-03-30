
import re
import os,sys

def replace(txt):
    groups = txt.groups()
    sels = []
    for sel in groups[0].split(','):
        if not '#themebuilder-main' in sel and not '#' in sel and not re.findall("[\W^]body[\W$]",sel):
            sel = re.findall('^\s*',sel)[0] + '#themebuilder-wrapper ' + sel.strip() + re.findall('\s*$',sel)[0]
        sels.append(sel)
    return ','.join(sels)+'{'+groups[1]+'}'

def fix(filename):
    text = open(filename).read()
    replaced = re.sub(r"([\w\.#][\w:\.#\-\s,]+){([^}]*)}", replace, text)
    open(filename+'.old','w').write(text)
    open(filename,'w').write(replaced)

def replace2(txt):
    groups = txt.groups()
    sels = []
    for sel in groups[0].split(','):
        if not '#themebuilder-wrapper #themebuilder-main' in sel and not re.findall("[\W\^]body[\W\$]",sel):
            sel = re.findall('^\s*',sel)[0] + '#themebuilder-wrapper #themebuilder-main ' + sel.strip().replace('#themebuilder-main','').replace('#themebuilder-wrapper','').strip() + re.findall('\s*$',sel)[0]
        sels.append(sel)
    return ','.join(sels)+'{'+groups[1]+'}'

def fix2(filename):
    text = open(filename).read()
    replaced = re.sub(r"([\w\.#][\w:\.#\-\s,]+){([^}]*)}", replace2, text)
    open(filename+'.old','w').write(text)
    open(filename,'w').write(replaced)

import glob

if __name__=='__main__':
  # fix module files
  themedir = "../http/cssreset/modules/acquia/themebuilder/"
  css = []
  for i in range(10):
      css += glob.glob(themedir + "*/"*i + "*.css")
  print css
  for file in css:
    print file
    fix2(file)
  # fix jueryui
  themedir = "../http/cssreset/modules/acquia/jquery_ui/"
  css = []
  for i in range(10):
      css += glob.glob(themedir + "*/"*i + "*.css")
  print css
  for file in css:
    print file
    fix(file)
