#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, urllib, urllib2, json

image_data = open('widora2.jpg','rb').read()

url ='https://aip.baidubce.com/rest/2.0/ocr/v1/general_basic?access_token=24.2e8d5c22d3ab78c0a3591ef7ef88d95b.2592000.1504785362.282335-9964286'
data = {}
data['language_type'] = "CHN_ENG"
#data['image'] = "/9j/4AAQSkZJRgABAQEAYABgAAD/2wBDABMNDxEPDBMREBEWFRMXHTAfHRsbHTsqLSMwRj5KSUU+RENNV29eTVJpU0NEYYRiaXN3fX59S12Jkoh5kW96fXj/2wBDARUWFh0ZHTkfHzl4UERQeHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHh4eHj/wAARCAAfACEDAREAAhEBAxEB/8QAGAABAQEBAQAAAAAAAAAAAAAAAAQDBQb/xAAjEAACAgICAgEFAAAAAAAAAAABAgADBBESIRMxBSIyQXGB/8QAFAEBAAAAAAAAAAAAAAAAAAAAAP/EABQRAQAAAAAAAAAAAAAAAAAAAAD/2gAMAwEAAhEDEQA/APawEBAQEBAgy8i8ZTVV3UY6V1eU2XoWDDZB19S646Gz39w9fkKsW1r8Wm2yo1PYis1be0JG9H9QNYCAgc35Cl3yuVuJZl0cB41rZQa32dt2y6OuOiOxo61vsLcVblxaVyXD3hFFjL6La7I/sDWAgICAgICB/9k="
data['image'] = image_data.encode('base64').replace('\n','')
decoded_data = urllib.urlencode(data)
req = urllib2.Request(url, data = decoded_data)
req.add_header("Content-Type","application/x-www-form-urlencoded")
#req.add_header("access_token","24.2e8d5c22d3ab78c0a3591ef7ef88d95b.2592000.1504785362.282335-9964286")

resp = urllib2.urlopen(req)
content = resp.read()
if(content):
     print(content)
