#ifndef N_TRADER_H_
#define N_TRADER_H_

#include "stdafx.h"
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <node.h>
#include <node_object_wrap.h>
#include "ThostFtdcTraderApi.h"
#include "ThostFtdcUserApiDataType.h"
#include <uv.h>
#include "uv_trader.h"

using namespace v8;

extern bool islog;
extern void logger_cout(const char* content);
extern std::string to_string(int val);
extern std::string charto_string(char val);

class WrapTrader : public node::ObjectWrap {
public:
	WrapTrader(void);
	~WrapTrader(void);

	///连接前置机
	static Handle<Value> Connect(const FunctionCallbackInfo<Value>& args);	
	///注册事件
	static Handle<Value> On(const FunctionCallbackInfo<Value>& args);
	///用户登录请求
	static Handle<Value> ReqUserLogin(const FunctionCallbackInfo<Value>& args);
	///登出请求 
	static Handle<Value> ReqUserLogout(const FunctionCallbackInfo<Value>& args);
	///投资者结算结果确认
	static Handle<Value> ReqSettlementInfoConfirm(const FunctionCallbackInfo<Value>& args);
	///请求查询合约
	static Handle<Value> ReqQryInstrument(const FunctionCallbackInfo<Value>& args);
	///请求查询资金账户
	static Handle<Value> ReqQryTradingAccount(const FunctionCallbackInfo<Value>& args);
	///请求查询投资者持仓
	static Handle<Value> ReqQryInvestorPosition(const FunctionCallbackInfo<Value>& args);
	///持仓明细
	static Handle<Value> ReqQryInvestorPositionDetail(const FunctionCallbackInfo<Value>& args);
	///报单录入请求
	static Handle<Value> ReqOrderInsert(const FunctionCallbackInfo<Value>& args);
	///报单操作请求
	static Handle<Value> ReqOrderAction(const FunctionCallbackInfo<Value>& args);
	///请求查询合约保证金率 
	static Handle<Value> ReqQryInstrumentMarginRate(const FunctionCallbackInfo<Value>& args);
	///请求查询行情 
	static Handle<Value> ReqQryDepthMarketData(const FunctionCallbackInfo<Value>& args);
	///请求查询投资者结算结果 
	static Handle<Value> ReqQrySettlementInfo(const FunctionCallbackInfo<Value>& args);
	///删除接口对象
	static Handle<Value> Disposed(const FunctionCallbackInfo<Value>& args);
	//对象初始化
	static void Init(int args);
	static Handle<Value> NewInstance(const FunctionCallbackInfo<Value>& args);
    static Handle<Value> GetTradingDay(const FunctionCallbackInfo<Value>& args);

private:
	static void initEventMap();	
	static Handle<Value> New(const FunctionCallbackInfo<Value>& args);
	static void pkg_cb_userlogin(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_userlogout(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_confirm(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_orderinsert(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_errorderinsert(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_orderaction(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_errorderaction(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rspqryorder(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rtnorder(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqtrade(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rtntrade(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqinvestorposition(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqinvestorpositiondetail(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqtradingaccount(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqinstrument(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqdepthmarketdata(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rqsettlementinfo(CbRtnField* data, Local<Value>*cbArray);
	static void pkg_cb_rsperror(CbRtnField* data, Local<Value>*cbArray);

	static Local<Value> pkg_rspinfo(void *vpRspInfo);
	uv_trader* uvTrader;
	static int s_uuid;
	static void FunCallback(CbRtnField *data);
	static void FunRtnCallback(int result, void* baton);
	static Persistent<Function> constructor;
	static std::map<const char*, int,ptrCmp> event_map;
	static std::map<int, Persistent<Function> > callback_map;
	static std::map<int, Persistent<Function> > fun_rtncb_map; 	
};



#endif
