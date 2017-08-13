#!/usr/bin/env python
# -*- coding: utf-8 -*-
# --- with reference to:  http://blog.csdn.net/dmcpxy/article/details/39495871

import urllib,urllib2
# Python的str默认是ascii编码 和unicode编码冲突
# 下面使得str编码用实际编码而不是 ascii
import sys
reload(sys)
sys.setdefaultencoding('utf-8')

#debug=True
debug=False

class Utility:
    @staticmethod
    def ToUTF(str_origin):
        if(debug):
		print(str_origin)
        return str_origin.decode('gbk')

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
	str_ret=Utility.ToUTF(f.readline())
#---or	str_ret=f.readline().decode('gbk')
 	f.close()
        if(debug):
		print 'str_ret: %s'%str_ret
        return str_ret

#    @staticmethod
    def ParseResultStr(self,resultstr):
	#print "----type(resultstr):%s ---  resultstr: %s" % (type(resultstr),resultstr)
        str_stock=resultstr[14:]

#	print 'str_stock:%s, type(str_stock):%s'%(str_stock,type(str_stock))

        if(debug):
		print "---str_stock: %s" % str_stock

        slist=str_stock.split('~')
#	print 'slist: %s    type(slist): %s'% (slist,type(slist))
# ---- 列表的unicode字符串打印会乱码，打印结果如下:
#  slist: [u'51', u'\u534e\u8c0a\u5144\u5f1f', u'300027', u'9.02', u'-0.10', u'-1.10', u'372384', u'33856', u'', u'250.26";\n']    type(slist): <type 'list'>
#  unicode  == unsigned short == wchar_t 上面\u是转义字符	print 'slist[1]: %s    type(slist[1]): %s'% ( str(slist[1]),type(slist[1]))
        print('*******************************')
        print'  股票名称:  %s' % slist[1]
        print'  股票代码:  %s' % slist[2]
        print'  当前价格:  %s' % slist[3]
        print'  涨    跌:  %s' % slist[4]
        print'  涨  跌 %%:  %s%%' % slist[5]
        print'成交量(手):  %s' % slist[6]
        print'成交额(万):  %s' % slist[7]
        #print('date and time is :', dateandtime)
        #print('*******************************')

#    @staticmethod
    def GetStockInfo(self,num):
        str_ret=self.GetStockStrByNum(num)
#---or        str_ret=StockInfo().GetStockStrByNum(num)
        #str_decoded=Utility.ToGB(str_ret)
	#print 'str_decode: %s'% str_decoded
        StockInfo().ParseResultStr(str_ret)
#---or     self.ParseResultStr(str_ret)

if __name__ == '__main__':
    stocks = ['sh600888','sz300027','sz000919','sh601018']
    my=StockInfo()
    for stock in stocks:
        my.GetStockInfo(stock)

