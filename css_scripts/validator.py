## css validator....makes use of the online service.

import httplib, mimetypes

### multipart postage ###

def post_multipart(host, selector, fields, files):
    """
    Post fields and files to an http host as multipart/form-data.
    fields is a sequence of (name, value) elements for regular form fields.
    files is a sequence of (name, filename, value) elements for data to be uploaded as files
    Return the server's response page.
    """
    content_type, body = encode_multipart_formdata(fields, files)
    h = httplib.HTTP(host)
    h.putrequest('POST', selector)
    h.putheader('content-type', content_type)
    h.putheader('content-length', str(len(body)))
    h.endheaders()
    h.send(body)
    errcode, errmsg, headers = h.getreply()
    return h.file.read()

def encode_multipart_formdata(fields, files):
    """
    fields is a sequence of (name, value) elements for regular form fields.
    files is a sequence of (name, filename, value) elements for data to be uploaded as files
    Return (content_type, body) ready for httplib.HTTP instance
    """
    BOUNDARY = '----------ThIs_Is_tHe_bouNdaRY_$'
    CRLF = '\r\n'
    L = []
    for (key, value) in fields:
        L.append('--' + BOUNDARY)
        L.append('Content-Disposition: form-data; name="%s"' % key)
        L.append('')
        L.append(value)
    for (key, filename, value) in files:
        L.append('--' + BOUNDARY)
        L.append('Content-Disposition: form-data; name="%s"; filename="%s"' % (key, filename))
        L.append('Content-Type: %s' % get_content_type(filename))
        L.append('')
        L.append(value)
    L.append('--' + BOUNDARY + '--')
    L.append('')
    body = CRLF.join(L)
    content_type = 'multipart/form-data; boundary=%s' % BOUNDARY
    return content_type, body

def get_content_type(filename):
    return mimetypes.guess_type(filename)[0] or 'application/octet-stream'

def get_page(url,data=None):
  if data:data = urllib.urlencode(data)
  req = urllib2.Request(url, data, {'User-agent' : 'Mozilla/4.0 (compatible; MSIE 5.5; Windows NT)'})
  try:
      handle = urllib2.urlopen(req)
  except:
      try:handle = urllib2.urlopen(req)
      except:handle = urllib2.urlopen(req)
  text = handle.read()
  redirect_pattern = re.escape('window.location.replace("') + '([^"]+)' + re.escape('");')
  result = re.findall(redirect_pattern,text)
  return text





### actual validating code ###

def get_errors(text):
  errors = re.findall("<tr class='error'>.+?</tr>",text,re.S|re.M)
  for e in errors:
    line = re.findall("<td class='linenumber' title='Line \d+'>(\d+)</td>",e)
    context = re.findall("<td class='codeContext'>(.+?)</td>",e,re.S)
    error = re.findall("<td class='parse-error'>(.+?)</td>",e,re.S)
    yield line,context,error

def validate(text):
  data = {'text':text,
          'profile':'css3',
          'type':'css',
          'usermedium':'all',
          'warning':'0'}

  result = post_multipart('jigsaw.w3.org','/css-validator/validator',data.items(),[])
  if '<h3>Congratulations! No Error Found.</h3>' in result:
    return True
  else:
    return list(get_errors(result))








def process(file):
  res = validate(open(file).read())
  if res:
    print '.',
    sys.stdout.flush()
    return
  else:
    print file
    for e in errors:
      print 'Error at %d: %s %s'%(e[0],''.join(e[1]),''.join(e[2]))

def finish():
  print 'done processing. see above for errors'







import glob,sys
if __name__=='__main__':
  # fix module files
  themedir = "cssreset/modules/acquia/themebuilder/"
  css = []
  for i in range(10):
      css += glob.glob(themedir + "*/"*i + "*.css")
  print len(css),'files found'
  for file in css:
    process(file)
  finish()



