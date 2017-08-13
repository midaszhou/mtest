#!/usr/bin/env python
# -*- coding: utf-8 -*-
# --- with reference to:  http://blog.csdn.net/dmcpxy/article/details/39495871

import urllib,urllib2


#debug=True
debug=False

class Utility:
    @staticmethod
    def ToGB(str_origin):
        if(debug):
		print(str_origin)
        return str_origin.decode('GBK').encode('utf-8')

class StockInfo():
    """
     0: 未知
     1: 名称
     2: 代码
     3: 当前价格
     4: 涨跌
     5: 涨跌%
     6: 成交量（手）
     7: 成交额（万）
     8:
     9: 总市值"""

    @staticmethod
    def GetStockStrByNum(num):
        req= urllib2.Request('http://qt.gtimg.cn/q=s_'+ str(num))
        f=urllib2.urlopen(req)
        if(debug):
		print 'type(f):%s'% str(type(f))
        if(debug):
		print '----- f.geturl():'
	 	print(f.geturl())
        if(debug):
		 print '----- f.info():'
		 print(f.info())
        #return like: v_s_sz000858="51~五 粮 液~000858~18.10~0.01~0.06~94583~17065~~687.07";
	str_ret=f.readline().decode('gbk')
 	f.close()
        if(debug):
		print 'str_ret: %s'%str_ret
        return str_ret

#    @staticmethod
    def ParseResultStr(self,resultstr):
        if(debug):
		 print "resultstr: %s" % resultstr
        slist=resultstr[14:]
        if(debug):
		print "---slist: %s" % slist
        if(debug):
		 print(slist)
        slist=slist.split(u'~')

        if(debug):
	   print ('slist: ', slist)

        print('*******************************')
        print'  股票名称:  %s' % slist[1].encode('utf-8')
        print'  股票代码:  %s' % slist[2].encode('utf-8')
        print'  当前价格:  %s' % slist[3].encode('utf-8')
        print'  涨    跌:  %s' % slist[4].encode('utf-8')
        print'  涨  跌%%:  %s%%' % slist[5].encode('utf-8')
        print'成交量(手):  %s' % slist[6].encode('utf-8')
        print'成交额(万):  %s' % slist[7].encode('utf-8')
        #print('date and time is :', dateandtime)
        print('*******************************')

#    @staticmethod
    def GetStockInfo(self,num):
#        str_ret=StockInfo.GetStockStrByNum(num)
        str_ret=self.GetStockStrByNum(num)
        #str_decoded=Utility.ToGB(str_ret)
	#print 'str_decode: %s'% str_decoded
#        StockInfo.ParseResultStr(str_ret)
        self.ParseResultStr(str_ret)

if __name__ == '__main__':
    stocks = ['sh600888','sz300104','sz300027','sz000919']
    my=StockInfo()
#    my.GetStockInfo('sh600888')
    for stock in stocks:
        my.GetStockInfo(stock)

