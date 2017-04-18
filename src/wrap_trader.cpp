#include <node.h>
#include "wrap_trader.h"

Persistent<Function> WrapTrader::constructor;
int WrapTrader::s_uuid;
std::map<const char*, int,ptrCmp> WrapTrader::event_map;
std::map<int, Persistent<Function> > WrapTrader::callback_map;
std::map<int, Persistent<Function> > WrapTrader::fun_rtncb_map;

WrapTrader::WrapTrader() {	
	logger_cout("wrap_trader------>object start init");
	uvTrader = new uv_trader();	
	logger_cout("wrap_trader------>object init successed");
}

WrapTrader::~WrapTrader(void) {
    if(uvTrader){
	    delete uvTrader;
    }
	logger_cout("wrap_trader------>object destroyed");
}

void WrapTrader::Init(Isolate *isolate) {
	// Prepare constructor template
	Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
	tpl->SetClassName(String::NewFromUtf8(isolate, "WrapTrader"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Prototype
	NODE_SET_PROTOTYPE_METHOD(tpl, "on", On);
	NODE_SET_PROTOTYPE_METHOD(tpl, "connect", Connect);
	NODE_SET_PROTOTYPE_METHOD(tpl, "reqUserLogin", ReqUserLogin);
	NODE_SET_PROTOTYPE_METHOD(tpl, "reqUserLogout", ReqUserLogout);
	NODE_SET_PROTOTYPE_METHOD(tpl, "reqSettlementInfoConfirm", ReqSettlementInfoConfirm);
	NODE_SET_PROTOTYPE_METHOD(tpl, "reqQryInstrument", ReqQryInstrument);
	NODE_SET_PROTOTYPE_METHOD(tpl, "reqQryTradingAccount", ReqQryTradingAccount);
	NODE_SET_PROTOTYPE_METHOD(tpl, "reqQryInvestorPosition", ReqQryInvestorPosition);
	NODE_SET_PROTOTYPE_METHOD(tpl, "reqQryInvestorPositionDetail", ReqQryInvestorPositionDetail);
	NODE_SET_PROTOTYPE_METHOD(tpl, "reqOrderInsert", ReqOrderInsert);
	NODE_SET_PROTOTYPE_METHOD(tpl, "reqOrderAction", ReqOrderAction);
	NODE_SET_PROTOTYPE_METHOD(tpl, "reqQryInstrumentMarginRate", ReqQryInstrumentMarginRate);
	NODE_SET_PROTOTYPE_METHOD(tpl, "reqQryDepthMarketData", ReqQryDepthMarketData);
	NODE_SET_PROTOTYPE_METHOD(tpl, "reqQrySettlementInfo", ReqQrySettlementInfo);
	NODE_SET_PROTOTYPE_METHOD(tpl, "disconnect", Disposed);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTradingDay", GetTradingDay);


	// constructor = Persistent<Function>::New(tpl->GetFunction());
	constructor.Reset(isolate, tpl->GetFunction());
}

void WrapTrader::initEventMap() {
	event_map["connect"] = T_ON_CONNECT;
	event_map["disconnected"] = T_ON_DISCONNECTED;
	event_map["rspUserLogin"] = T_ON_RSPUSERLOGIN;
	event_map["rspUserLogout"] = T_ON_RSPUSERLOGOUT;
	event_map["rspInfoconfirm"] = T_ON_RSPINFOCONFIRM;
	event_map["rspInsert"] = T_ON_RSPINSERT;
	event_map["errInsert"] = T_ON_ERRINSERT;
	event_map["rspAction"] = T_ON_RSPACTION;
	event_map["errAction"] = T_ON_ERRACTION;
	event_map["rqOrder"] = T_ON_RQORDER;
	event_map["rtnOrder"] = T_ON_RTNORDER;
	event_map["rqTrade"] = T_ON_RQTRADE;
	event_map["rtnTrade"] = T_ON_RTNTRADE;
	event_map["rqInvestorPosition"] = T_ON_RQINVESTORPOSITION;
	event_map["rqInvestorPositionDetail"] = T_ON_RQINVESTORPOSITIONDETAIL;
	event_map["rqTradingAccount"] = T_ON_RQTRADINGACCOUNT;
	event_map["rqInstrument"] = T_ON_RQINSTRUMENT;
	event_map["rqDdpthmarketData"] = T_ON_RQDEPTHMARKETDATA;
	event_map["rqSettlementInfo"] = T_ON_RQSETTLEMENTINFO;
	event_map["rspError"] = T_ON_RSPERROR;
}

Handle<Value> WrapTrader::New(const FunctionCallbackInfo<Value>& args) {
	Isolate *isolate = args.GetIsolate();

  if (event_map.size() == 0)
      initEventMap();

  if (args.IsConstructCall()) {
      // Invoked as constructor: `new MyObject(...)`
      WrapTrader *wTrader = new WrapTrader();
      wTrader->Wrap(args.This());
      args.GetReturnValue().Set(args.This());
  } else {
      // Invoked as plain function `MyObject(...)`, turn into construct call.
      const int argc = 1;
      Local <Value> argv[argc] = {Number::New(isolate, 0)};
      Local <Function> cons = Local<Function>::New(isolate, constructor);
      Local <Context> context = isolate->GetCurrentContext();
      Local <Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
      args.GetReturnValue().Set(instance);
  }
}

Handle<Value> WrapTrader::NewInstance(const FunctionCallbackInfo<Value>& args) {
	Isolate *isolate = args.GetIsolate();
  const unsigned argc = 1;
  Local <Value> argv[argc] = {Number::New(isolate, 0)};
  Local <Function> cons = Local<Function>::New(isolate, constructor);
  Local <Context> context = isolate->GetCurrentContext();
  Local <Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
  args.GetReturnValue().Set(instance);
}

Handle<Value> WrapTrader::On(const FunctionCallbackInfo<Value>& args) {
	Isolate *isolate = args.GetIsolate();

  if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
      logger_cout("Wrong arguments->event name or function");
      isolate->ThrowException(
              Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments->event name or function")));
      return;
  }

  WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.Holder());

  Local <String> eventName = args[0]->ToString();
  Local <Function> cb = Local<Function>::Cast(args[1]);

  String::Utf8Value eNameAscii(eventName);

  std::map<std::string, int>::iterator eIt = event_map.find((std::string) * eNameAscii);
  if (eIt == event_map.end()) {
      logger_cout("System has not register this event");
      isolate->ThrowException(
              Exception::TypeError(String::NewFromUtf8(isolate, "System has no register this event")));
      return;
  }

  std::map < int, Persistent < Function > > ::iterator
  cIt = callback_map.find(eIt->second);
  if (cIt != callback_map.end()) {
      logger_cout("Callback is defined before");
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Callback is defined before")));
      return;
  }

  callback_map[eIt->second].Reset(isolate, cb);
  obj->uvTrader->On(*eNameAscii, eIt->second, FunCallback);
  return args.GetReturnValue().Set(String::NewFromUtf8(isolate, "finish exec on"));
}

Handle<Value> WrapTrader::Connect(const FunctionCallbackInfo<Value>& args) {
	Isolate *isolate = args.GetIsolate();

  if (args[0]->IsUndefined()) {
      logger_cout("Wrong arguments->front addr");
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments->front addr")));
      return;
  }
  if (!args[2]->IsNumber() || !args[3]->IsNumber()) {
      logger_cout("Wrong arguments->public or private topic type");
      isolate->ThrowException(
              Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments->public or private topic type")));
      return;
  }
  int uuid = -1;
  WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.Holder());
  if (!args[4]->IsUndefined() && args[4]->IsFunction()) {
      uuid = ++s_uuid;
      fun_rtncb_map[uuid].Reset(isolate, Local<Function>::Cast(args[4]));
      logger_cout(to_string(uuid).append("|uuid").c_str());
  }

  Local <String> frontAddr = args[0]->ToString();
  Local <String> szPath = args[1]->IsUndefined() ? String::NewFromUtf8(isolate, "t") : args[0]->ToString();
  String::Utf8Value addrUtf8(frontAddr);
  String::Utf8Value pathUtf8(szPath);
  int publicTopicType = args[2]->Int32Value();
  int privateTopicType = args[3]->Int32Value();

  UVConnectField pConnectField;
  memset(&pConnectField, 0, sizeof(pConnectField));
  strcpy(pConnectField.front_addr, ((std::string) * addrUtf8).c_str());
  strcpy(pConnectField.szPath, ((std::string) * pathUtf8).c_str());
  pConnectField.public_topic_type = publicTopicType;
  pConnectField.private_topic_type = privateTopicType;
  logger_cout(((std::string) * addrUtf8).append("|addrUtf8").c_str());
  logger_cout(((std::string) * pathUtf8).append("|pathUtf8").c_str());
  logger_cout(to_string(publicTopicType).append("|publicTopicType").c_str());
  logger_cout(to_string(privateTopicType).append("|privateTopicType").c_str());

  obj->uvTrader->Connect(&pConnectField, FunRtnCallback, uuid);
  return args.GetReturnValue().Set(String::NewFromUtf8(isolate, "finish exec connect"));
}

Handle<Value> WrapTrader::ReqUserLogin(const FunctionCallbackInfo<Value>& args) {
	Isolate *isolate = args.GetIsolate();

  std::string log = "wrap_trader ReqUserLogin------>";
  if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
      std::string _head = std::string(log);
      logger_cout(_head.append(" Wrong arguments").c_str());
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments")));
      return;
  }

  int uuid = -1;
  WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.Holder());
  if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
      uuid = ++s_uuid;
      fun_rtncb_map[uuid].Reset(isolate, Local<Function>::Cast(args[3]));
      std::string _head = std::string(log);
      logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
  }

  Local <String> broker = args[0]->ToString();
  Local <String> userId = args[1]->ToString();
  Local <String> pwd = args[2]->ToString();
  String::Utf8Value brokerUtf8(broker);
  String::Utf8Value userIdUtf8(userId);
  String::Utf8Value pwdUtf8(pwd);

  CThostFtdcReqUserLoginField req;
  memset(&req, 0, sizeof(req));
  strcpy(req.BrokerID, ((std::string) * brokerUtf8).c_str());
  strcpy(req.UserID, ((std::string) * userIdUtf8).c_str());
  strcpy(req.Password, ((std::string) * pwdUtf8).c_str());
  logger_cout(
          log.append(" ").append((std::string) * brokerUtf8).append("|").append((std::string) * userIdUtf8).append(
                  "|").append((std::string) * pwdUtf8).c_str());
  obj->uvTrader->ReqUserLogin(&req, FunRtnCallback, uuid);
  return args.GetReturnValue().Set(String::NewFromUtf8(isolate, "finish exec reqUserlogin"));
}

Handle<Value> WrapTrader::ReqUserLogout(const FunctionCallbackInfo<Value>& args) {
	Isolate *isolate = args.GetIsolate();
  std::string log = "wrap_trader ReqUserLogout------>";

  if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
      std::string _head = std::string(log);
      logger_cout(_head.append(" Wrong arguments").c_str());
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong arguments")));
      return ;
  }
  int uuid = -1;
  WrapTrader *obj = ObjectWrap::Unwrap<WrapTrader>(args.Holder());
  if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
      uuid = ++s_uuid;
      fun_rtncb_map[uuid].Reset(isolate, Local<Function>::Cast(args[2]));
      std::string _head = std::string(log);
      logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
  }

  Local <String> broker = args[0]->ToString();
  Local <String> userId = args[1]->ToString();
  String::Utf8Value brokerAscii(broker);
  String::Utf8Value userIdAscii(userId);

  CThostFtdcUserLogoutField req;
  memset(&req, 0, sizeof(req));
  strcpy(req.BrokerID, ((std::string) * brokerAscii).c_str());
  strcpy(req.UserID, ((std::string) * userIdAscii).c_str());
  logger_cout(log.append(" ").append((std::string) * brokerAscii).append("|").append(
          (std::string) * userIdAscii).c_str());
  obj->uvTrader->ReqUserLogout(&req, FunRtnCallback, uuid);
  return ;
}

Handle<Value> WrapTrader::ReqSettlementInfoConfirm(const FunctionCallbackInfo<Value>& args) {
	HandleScope scope;
	std::string log = "wrap_trader ReqSettlementInfoConfirm------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;	
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[2]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}	 

	Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);

	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
	logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*investorIdAscii).c_str());
	obj->uvTrader->ReqSettlementInfoConfirm(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqQryInstrument(const FunctionCallbackInfo<Value>& args) {
	HandleScope scope;
	std::string log = "wrap_trader ReqQryInstrument------>";

	if (args[0]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> instrumentId = args[0]->ToString();
	String::AsciiValue instrumentIdAscii(instrumentId);

	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());
	logger_cout(log.append(" ").append((std::string)*instrumentIdAscii).c_str());
	obj->uvTrader->ReqQryInstrument(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqQryTradingAccount(const FunctionCallbackInfo<Value>& args) {
	HandleScope scope;
	std::string log = "wrap_trader ReqQryTradingAccount------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[2]->IsUndefined() && args[2]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[2]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);

	CThostFtdcQryTradingAccountField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
	logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*investorIdAscii).c_str());
	obj->uvTrader->ReqQryTradingAccount(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqQryInvestorPosition(const FunctionCallbackInfo<Value>& args) {
	HandleScope scope;
	std::string log = "wrap_trader ReqQryInvestorPosition------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[3]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	Local<String> instrumentId = args[2]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);
	String::AsciiValue instrumentIdAscii(instrumentId);

	CThostFtdcQryInvestorPositionField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
	strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());

	logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*investorIdAscii).append("|").append((std::string)*instrumentIdAscii).c_str());
	obj->uvTrader->ReqQryInvestorPosition(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqQryInvestorPositionDetail(const FunctionCallbackInfo<Value>& args) {
	HandleScope scope;
	std::string log = "wrap_trader ReqQryInvestorPositionDetail------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[3]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	Local<String> instrumentId = args[2]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);
	String::AsciiValue instrumentIdAscii(instrumentId);

	CThostFtdcQryInvestorPositionDetailField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
	strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());

	logger_cout(log.append(" ").append((std::string)*brokerAscii).append("|").append((std::string)*investorIdAscii).append("|").append((std::string)*instrumentIdAscii).c_str());
	obj->uvTrader->ReqQryInvestorPositionDetail(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqOrderInsert(const FunctionCallbackInfo<Value>& args) {
	HandleScope scope;
	std::string log = "wrap_trader ReqOrderInsert------>";

	if (args[0]->IsUndefined() || !args[0]->IsObject()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}
	Local<Object> jsonObj = args[0]->ToObject();
	Local<Value> brokerId = jsonObj->Get(v8::String::New("brokerId"));
	if (brokerId->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->brokerId")));
		return scope.Close(Undefined());
	}
	String::AsciiValue brokerId_(brokerId->ToString());
	Local<Value> investorId = jsonObj->Get(v8::String::New("investorId"));
	if (investorId->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->investorId")));
		return scope.Close(Undefined());
	}
	String::AsciiValue investorId_(investorId->ToString());
	Local<Value> instrumentId = jsonObj->Get(v8::String::New("instrumentId"));
	if (instrumentId->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->instrumentId")));
		return scope.Close(Undefined());
	}
	String::AsciiValue instrumentId_(instrumentId->ToString());
	Local<Value> priceType = jsonObj->Get(v8::String::New("priceType"));
	if (priceType->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->priceType")));
		return scope.Close(Undefined());
	}
	String::AsciiValue priceType_(priceType->ToString());
	Local<Value> direction = jsonObj->Get(v8::String::New("direction"));
	if (direction->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->direction")));
		return scope.Close(Undefined());
	}
	String::AsciiValue direction_(direction->ToString());
	Local<Value> combOffsetFlag = jsonObj->Get(v8::String::New("combOffsetFlag"));
	if (combOffsetFlag->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->combOffsetFlag")));
		return scope.Close(Undefined());
	}
	String::AsciiValue combOffsetFlag_(combOffsetFlag->ToString());
	Local<Value> combHedgeFlag = jsonObj->Get(v8::String::New("combHedgeFlag"));
	if (combHedgeFlag->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->combHedgeFlag")));
		return scope.Close(Undefined());
	}
	String::AsciiValue combHedgeFlag_(combHedgeFlag->ToString());
	Local<Value> vlimitPrice = jsonObj->Get(v8::String::New("limitPrice"));
	if (vlimitPrice->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->limitPrice")));
		return scope.Close(Undefined());
	}
	double limitPrice = vlimitPrice->NumberValue();
	Local<Value> vvolumeTotalOriginal = jsonObj->Get(v8::String::New("volumeTotalOriginal"));
	if (vvolumeTotalOriginal->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->volumeTotalOriginal")));
		return scope.Close(Undefined());
	}
	int32_t volumeTotalOriginal = vvolumeTotalOriginal->Int32Value();
	Local<Value> timeCondition = jsonObj->Get(v8::String::New("timeCondition"));
	if (timeCondition->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->timeCondition")));
		return scope.Close(Undefined());
	}
	String::AsciiValue timeCondition_(timeCondition->ToString());
	Local<Value> volumeCondition = jsonObj->Get(v8::String::New("volumeCondition"));
	if (volumeCondition->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->volumeCondition")));
		return scope.Close(Undefined());
	}
	String::AsciiValue volumeCondition_(volumeCondition->ToString());
	Local<Value> vminVolume = jsonObj->Get(v8::String::New("minVolume"));
	if (vminVolume->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->minVolume")));
		return scope.Close(Undefined());
	}
	int32_t minVolume = vminVolume->Int32Value();
	Local<Value> forceCloseReason = jsonObj->Get(v8::String::New("forceCloseReason"));
	if (forceCloseReason->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->forceCloseReason")));
		return scope.Close(Undefined());
	}
	String::AsciiValue forceCloseReason_(forceCloseReason->ToString());
	Local<Value> visAutoSuspend = jsonObj->Get(v8::String::New("isAutoSuspend"));
	if (visAutoSuspend->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->isAutoSuspend")));
		return scope.Close(Undefined());
	}
	int32_t isAutoSuspend = visAutoSuspend->Int32Value();
	Local<Value> vuserForceClose = jsonObj->Get(v8::String::New("userForceClose"));
	if (vuserForceClose->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->userForceClose")));
		return scope.Close(Undefined());
	}
	int32_t userForceClose = vuserForceClose->Int32Value();

	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));
    log.append(" ");

    Local<Value> orderRef = jsonObj->Get(v8::String::New("orderRef"));
    if (!orderRef->IsUndefined()) {
        String::AsciiValue orderRef_(orderRef->ToString());
        strcpy(req.OrderRef, ((std::string)*orderRef_).c_str());  
        log.append("orderRef:").append((std::string)*orderRef_).append("|");
    }

    Local<Value> vstopPrice = jsonObj->Get(v8::String::New("stopPrice"));
    if(!vstopPrice->IsUndefined()){
        double stopPrice = vstopPrice->NumberValue();
        req.StopPrice = stopPrice;
        log.append("stopPrice:").append(to_string(stopPrice)).append("|");
    }
    Local<Value> contingentCondition = jsonObj->Get(v8::String::New("contingentCondition"));
    if (!contingentCondition->IsUndefined()) {
        String::AsciiValue contingentCondition_(contingentCondition->ToString());
	    req.ContingentCondition = ((std::string)*contingentCondition_)[0];
		log.append("contingentCondition:").append((std::string)*contingentCondition_).append("|");
    }

	strcpy(req.BrokerID, ((std::string)*brokerId_).c_str());
	strcpy(req.InvestorID, ((std::string)*investorId_).c_str());
	strcpy(req.InstrumentID, ((std::string)*instrumentId_).c_str());
	req.OrderPriceType = ((std::string)*priceType_)[0];
	req.Direction = ((std::string)*direction_)[0];
	req.CombOffsetFlag[0] = ((std::string)*combOffsetFlag_)[0];
	req.CombHedgeFlag[0] = ((std::string)*combHedgeFlag_)[0];
	req.LimitPrice = limitPrice;
	req.VolumeTotalOriginal = volumeTotalOriginal;
	req.TimeCondition = ((std::string)*timeCondition_)[0];
	req.VolumeCondition = ((std::string)*volumeCondition_)[0];
	req.MinVolume = minVolume;
	req.ForceCloseReason = ((std::string)*forceCloseReason_)[0];
	req.IsAutoSuspend = isAutoSuspend;
	req.UserForceClose = userForceClose;
	logger_cout(log.
        append("brokerID:").append((std::string)*brokerId_).append("|").
		append("investorID:").append((std::string)*investorId_).append("|").
		append("instrumentID:").append((std::string)*instrumentId_).append("|").
		append("priceType:").append((std::string)*priceType_).append("|").
		append("direction:").append((std::string)*direction_).append("|").
		append("comboffsetFlag:").append((std::string)*combOffsetFlag_).append("|").
		append("combHedgeFlag:").append((std::string)*combHedgeFlag_).append("|").
		append("limitPrice:").append(to_string(limitPrice)).append("|").
		append("volumnTotalOriginal:").append(to_string(volumeTotalOriginal)).append("|").
		append("timeCondition:").append((std::string)*timeCondition_).append("|").
		append("volumeCondition:").append((std::string)*volumeCondition_).append("|").
		append("minVolume:").append(to_string(minVolume)).append("|").
		append("forceCloseReason:").append((std::string)*forceCloseReason_).append("|").
		append("isAutoSuspend:").append(to_string(isAutoSuspend)).append("|").
		append("useForceClose:").append(to_string(userForceClose)).append("|").c_str());
	obj->uvTrader->ReqOrderInsert(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqOrderAction(const FunctionCallbackInfo<Value>& args) {
	HandleScope scope;
	std::string log = "wrap_trader ReqOrderAction------>";

	if (args[0]->IsUndefined() || !args[0]->IsObject()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());

	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<Object> jsonObj = args[0]->ToObject();
	Local<Value> vbrokerId = jsonObj->Get(v8::String::New("brokerId"));
	if (vbrokerId->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->brokerId")));
		return scope.Close(Undefined());
	}
	String::AsciiValue brokerId_(vbrokerId->ToString());
	Local<Value> vinvestorId = jsonObj->Get(v8::String::New("investorId"));
	if (vinvestorId->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->investorId")));
		return scope.Close(Undefined());
	}
	String::AsciiValue investorId_(vinvestorId->ToString());
	Local<Value> vinstrumentId = jsonObj->Get(v8::String::New("instrumentId"));
	if (vinstrumentId->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->instrumentId")));
		return scope.Close(Undefined());
	}
	String::AsciiValue instrumentId_(vinstrumentId->ToString());
	Local<Value> vactionFlag = jsonObj->Get(v8::String::New("actionFlag"));
	if (vactionFlag->IsUndefined()) {
		ThrowException(Exception::TypeError(String::New("Wrong arguments->actionFlag")));
		return scope.Close(Undefined());
	}
	int32_t actionFlag = vactionFlag->Int32Value();

	CThostFtdcInputOrderActionField req;
	memset(&req, 0, sizeof(req));

    log.append(" ");
	Local<Value> vorderRef = jsonObj->Get(v8::String::New("orderRef"));
	if (!vorderRef->IsUndefined()) {
	    String::AsciiValue orderRef_(vorderRef->ToString());
	    strcpy(req.OrderRef, ((std::string)*orderRef_).c_str());
		log.append((std::string)*orderRef_).append("|");
	}
	Local<Value> vfrontId = jsonObj->Get(v8::String::New("frontId"));
	if (!vfrontId->IsUndefined()) {
	    int32_t frontId = vfrontId->Int32Value();
	    req.FrontID = frontId;
		log.append(to_string(frontId)).append("|");
	}
	Local<Value> vsessionId = jsonObj->Get(v8::String::New("sessionId"));
	if (!vsessionId->IsUndefined()) {
	    int32_t sessionId = vsessionId->Int32Value();
	    req.SessionID = sessionId;
		log.append(to_string(sessionId)).append("|");
	}
	Local<Value> vexchangeID = jsonObj->Get(v8::String::New("exchangeID"));
	if (!vexchangeID->IsUndefined()) {
	    String::AsciiValue exchangeID_(vexchangeID->ToString());
	    strcpy(req.ExchangeID, ((std::string)*exchangeID_).c_str());
		log.append((std::string)*exchangeID_).append("|");
	}
	Local<Value> vorderSysID = jsonObj->Get(v8::String::New("orderSysID"));
	if (vorderSysID->IsUndefined()) {
	    String::AsciiValue orderSysID_(vorderSysID->ToString());
	    strcpy(req.OrderSysID, ((std::string)*orderSysID_).c_str());
		log.append((std::string)*orderSysID_).append("|");
	}

	strcpy(req.BrokerID, ((std::string)*brokerId_).c_str());
	strcpy(req.InvestorID, ((std::string)*investorId_).c_str());
	req.ActionFlag = actionFlag;
	strcpy(req.InstrumentID, ((std::string)*instrumentId_).c_str());
	logger_cout(log.
		append((std::string)*brokerId_).append("|").
		append((std::string)*investorId_).append("|").
		append((std::string)*instrumentId_).append("|").
		append(to_string(actionFlag)).append("|").c_str());

	obj->uvTrader->ReqOrderAction(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqQryInstrumentMarginRate(const FunctionCallbackInfo<Value>& args) {
	HandleScope scope;
	std::string log = "wrap_trader ReqQryInstrumentMarginRate------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined() || args[3]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());

	if (!args[4]->IsUndefined() && args[4]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[4]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	Local<String> instrumentId = args[2]->ToString();
	int32_t hedgeFlag = args[3]->Int32Value();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);
	String::AsciiValue instrumentIdAscii(instrumentId);

	CThostFtdcQryInstrumentMarginRateField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
	strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());
	req.HedgeFlag = hedgeFlag;
	logger_cout(log.append(" ").
		append((std::string)*brokerAscii).append("|").
		append((std::string)*investorIdAscii).append("|").
		append((std::string)*instrumentIdAscii).append("|").
		append(to_string(hedgeFlag)).append("|").c_str());	 

	obj->uvTrader->ReqQryInstrumentMarginRate(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqQryDepthMarketData(const FunctionCallbackInfo<Value>& args) {
	HandleScope scope;
	std::string log = "wrap_trader ReqQryDepthMarketData------>";

	if (args[0]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[1]->IsUndefined() && args[1]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[1]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> instrumentId = args[0]->ToString();
	String::AsciiValue instrumentIdAscii(instrumentId);

	CThostFtdcQryDepthMarketDataField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.InstrumentID, ((std::string)*instrumentIdAscii).c_str());
	logger_cout(log.append(" ").
		append((std::string)*instrumentIdAscii).append("|").c_str());
	obj->uvTrader->ReqQryDepthMarketData(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::ReqQrySettlementInfo(const FunctionCallbackInfo<Value>& args) {
	HandleScope scope;
	std::string log = "wrap_trader ReqQrySettlementInfo------>";

	if (args[0]->IsUndefined() || args[1]->IsUndefined() || args[2]->IsUndefined()) {
		std::string _head = std::string(log);
		logger_cout(_head.append(" Wrong arguments").c_str());
		ThrowException(Exception::TypeError(String::New("Wrong arguments")));
		return scope.Close(Undefined());
	}
	int uuid = -1;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	if (!args[3]->IsUndefined() && args[3]->IsFunction()) {
		uuid = ++s_uuid;
		fun_rtncb_map[uuid] = Persistent<Function>::New(Local<Function>::Cast(args[3]));
		std::string _head = std::string(log);
		logger_cout(_head.append(" uuid is ").append(to_string(uuid)).c_str());
	}

	Local<String> broker = args[0]->ToString();
	Local<String> investorId = args[1]->ToString();
	Local<String> tradingDay = args[2]->ToString();
	String::AsciiValue brokerAscii(broker);
	String::AsciiValue investorIdAscii(investorId);
	String::AsciiValue tradingDayAscii(tradingDay);

	CThostFtdcQrySettlementInfoField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, ((std::string)*brokerAscii).c_str());
	strcpy(req.InvestorID, ((std::string)*investorIdAscii).c_str());
	strcpy(req.TradingDay, ((std::string)*tradingDayAscii).c_str());
	logger_cout(log.append(" ").
		append((std::string)*brokerAscii).append("|").
		append((std::string)*investorIdAscii).append("|").
		append((std::string)*tradingDayAscii).append("|").c_str());

	obj->uvTrader->ReqQrySettlementInfo(&req, FunRtnCallback, uuid);
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::Disposed(const FunctionCallbackInfo<Value>& args) {
	HandleScope scope;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	obj->uvTrader->Disconnect();	
	std::map<int, Persistent<Function> >::iterator callback_it = callback_map.begin();
	while (callback_it != callback_map.end()) {
		callback_it->second.Dispose();
		callback_it++;
	}
	event_map.clear();
	callback_map.clear();
	fun_rtncb_map.clear();
	delete obj->uvTrader;
    obj->uvTrader = NULL;
	logger_cout("wrap_trader Disposed------>wrap disposed");
	return scope.Close(Undefined());
}

Handle<Value> WrapTrader::GetTradingDay(const FunctionCallbackInfo<Value>& args){
    HandleScope scope;
	WrapTrader* obj = ObjectWrap::Unwrap<WrapTrader>(args.This());
	const char* tradingDay = obj->uvTrader->GetTradingDay();
	return scope.Close(String::New(tradingDay));
}

void WrapTrader::FunCallback(CbRtnField *data) {
	std::map<int, Persistent<Function> >::iterator cIt = callback_map.find(data->eFlag);
	if (cIt == callback_map.end())
		return;

	switch (data->eFlag) {
	case T_ON_CONNECT:
	{
						 Local<Value> argv[1] = { Local<Value>::New(Undefined()) };
						 cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);
						 break;
	}
	case T_ON_DISCONNECTED:
	{
							  Local<Value> argv[1] = { Int32::New(data->nReason) };
							  cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);
							  break;
	}
	case T_ON_RSPUSERLOGIN:
	{
							  Local<Value> argv[4];
							  pkg_cb_userlogin(data, argv);
							  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
							  break;
	}
	case T_ON_RSPUSERLOGOUT:
	{
							   Local<Value> argv[4];
							   pkg_cb_userlogout(data, argv);
							   cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
							   break;
	}
	case T_ON_RSPINFOCONFIRM:
	{
								Local<Value> argv[4];
								pkg_cb_confirm(data, argv);
								cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
								break;
	}
	case T_ON_RSPINSERT:
	{
						   Local<Value> argv[4];
						   pkg_cb_orderinsert(data, argv);
						   cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
						   break;
	}
	case T_ON_ERRINSERT:
	{
						   Local<Value> argv[2];
						   pkg_cb_errorderinsert(data, argv);
						   cIt->second->Call(Context::GetCurrent()->Global(), 2, argv);
						   break;
	}
	case T_ON_RSPACTION:
	{
						   Local<Value> argv[4];
						   pkg_cb_orderaction(data, argv);
						   cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
						   break;
	}
	case T_ON_ERRACTION:
	{
						   Local<Value> argv[2];
						   pkg_cb_errorderaction(data, argv);
						   cIt->second->Call(Context::GetCurrent()->Global(), 2, argv);

						   break;
	}
	case T_ON_RQORDER:
	{
						 Local<Value> argv[4];
						 pkg_cb_rspqryorder(data, argv);
						 cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
						 break;
	}
	case T_ON_RTNORDER:
	{
						  Local<Value> argv[1];
						  pkg_cb_rtnorder(data, argv);
						  cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);

						  break;
	}
	case T_ON_RQTRADE:
	{
						 Local<Value> argv[4];
						 pkg_cb_rqtrade(data, argv);
						 cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

						 break;
	}
	case T_ON_RTNTRADE:
	{
						  Local<Value> argv[1];
						  pkg_cb_rtntrade(data, argv);
						  cIt->second->Call(Context::GetCurrent()->Global(), 1, argv);

						  break;
	}
	case T_ON_RQINVESTORPOSITION:
	{
									Local<Value> argv[4];
									pkg_cb_rqinvestorposition(data, argv);
									cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

									break;
	}
	case T_ON_RQINVESTORPOSITIONDETAIL:
	{
										  Local<Value> argv[4];
										  pkg_cb_rqinvestorpositiondetail(data, argv);
										  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

										  break;
	}
	case T_ON_RQTRADINGACCOUNT:
	{
								  Local<Value> argv[4];
								  pkg_cb_rqtradingaccount(data, argv);
								  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

								  break;
	}
	case T_ON_RQINSTRUMENT:
	{
							  Local<Value> argv[4];
							  pkg_cb_rqinstrument(data, argv);
							  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

							  break;
	}
	case T_ON_RQDEPTHMARKETDATA:
	{
								   Local<Value> argv[4];
								   pkg_cb_rqdepthmarketdata(data, argv);
								   cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);

								   break;
	}
	case T_ON_RQSETTLEMENTINFO:
	{
								  Local<Value> argv[4];
								  pkg_cb_rqsettlementinfo(data, argv);
								  cIt->second->Call(Context::GetCurrent()->Global(), 4, argv);
								  break;
	}
	case T_ON_RSPERROR:
	{
						   Local<Value> argv[3];
						   pkg_cb_rsperror(data, argv);
						   cIt->second->Call(Context::GetCurrent()->Global(), 3, argv);

						   break;
	}
	}
}

void WrapTrader::FunRtnCallback(int result, void* baton) {
	HandleScope scope;
	LookupCtpApiBaton* tmp = static_cast<LookupCtpApiBaton*>(baton);	 
	if (tmp->uuid != -1) {
		std::map<int, Persistent<Function> >::iterator it = fun_rtncb_map.find(tmp->uuid);
		Local<Value> argv[2] = { Local<Value>::New(Int32::New(tmp->nResult)),Local<Value>::New(Int32::New(tmp->iRequestID)) };
		it->second->Call(Context::GetCurrent()->Global(), 2, argv);
		it->second.Dispose();
		fun_rtncb_map.erase(tmp->uuid);	  		
	}
	scope.Close(Undefined());
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void WrapTrader::pkg_cb_userlogin(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
	if (data->rtnField){  
	    CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pRspUserLogin->TradingDay));
		jsonRtn->Set(String::NewSymbol("LoginTime"), String::New(pRspUserLogin->LoginTime));
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pRspUserLogin->BrokerID));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pRspUserLogin->UserID));
		jsonRtn->Set(String::NewSymbol("SystemName"), String::New(pRspUserLogin->SystemName));
		jsonRtn->Set(String::NewSymbol("FrontID"), Int32::New(pRspUserLogin->FrontID));
		jsonRtn->Set(String::NewSymbol("SessionID"), Int32::New(pRspUserLogin->SessionID));
		jsonRtn->Set(String::NewSymbol("MaxOrderRef"), String::New(pRspUserLogin->MaxOrderRef));
		jsonRtn->Set(String::NewSymbol("SHFETime"), String::New(pRspUserLogin->SHFETime));
		jsonRtn->Set(String::NewSymbol("DCETime"), String::New(pRspUserLogin->DCETime));
		jsonRtn->Set(String::NewSymbol("CZCETime"), String::New(pRspUserLogin->CZCETime));
		jsonRtn->Set(String::NewSymbol("FFEXTime"), String::New(pRspUserLogin->FFEXTime));
		jsonRtn->Set(String::NewSymbol("INETime"), String::New(pRspUserLogin->INETime));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_userlogout(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcRspUserLoginField* pRspUserLogin = static_cast<CThostFtdcRspUserLoginField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pRspUserLogin->BrokerID));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pRspUserLogin->UserID));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_confirm(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm = static_cast<CThostFtdcSettlementInfoConfirmField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pSettlementInfoConfirm->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pSettlementInfoConfirm->InvestorID));
		jsonRtn->Set(String::NewSymbol("ConfirmDate"), String::New(pSettlementInfoConfirm->ConfirmDate));
		jsonRtn->Set(String::NewSymbol("ConfirmTime"), String::New(pSettlementInfoConfirm->ConfirmTime));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_orderinsert(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcInputOrderField* pInputOrder = static_cast<CThostFtdcInputOrderField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pInputOrder->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pInputOrder->InvestorID));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pInputOrder->InstrumentID));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pInputOrder->OrderRef));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pInputOrder->UserID));
		jsonRtn->Set(String::NewSymbol("OrderPriceType"), String::New(charto_string(pInputOrder->OrderPriceType).c_str()));
		jsonRtn->Set(String::NewSymbol("Direction"), String::New(charto_string(pInputOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(String::NewSymbol("CombOffsetFlag"), String::New(pInputOrder->CombOffsetFlag));
		jsonRtn->Set(String::NewSymbol("CombHedgeFlag"), String::New(pInputOrder->CombHedgeFlag));
		jsonRtn->Set(String::NewSymbol("LimitPrice"), Number::New(pInputOrder->LimitPrice));
		jsonRtn->Set(String::NewSymbol("VolumeTotalOriginal"), Int32::New(pInputOrder->VolumeTotalOriginal));
		jsonRtn->Set(String::NewSymbol("TimeCondition"), String::New(charto_string(pInputOrder->TimeCondition).c_str()));
		jsonRtn->Set(String::NewSymbol("GTDDate"), String::New(pInputOrder->GTDDate));
		jsonRtn->Set(String::NewSymbol("VolumeCondition"), String::New(charto_string(pInputOrder->VolumeCondition).c_str()));
		jsonRtn->Set(String::NewSymbol("MinVolume"), Int32::New(pInputOrder->MinVolume));
		jsonRtn->Set(String::NewSymbol("ContingentCondition"), String::New(charto_string(pInputOrder->ContingentCondition).c_str()));
		jsonRtn->Set(String::NewSymbol("StopPrice"), Number::New(pInputOrder->StopPrice));
		jsonRtn->Set(String::NewSymbol("ForceCloseReason"), String::New(charto_string(pInputOrder->ForceCloseReason).c_str()));
		jsonRtn->Set(String::NewSymbol("IsAutoSuspend"), Int32::New(pInputOrder->IsAutoSuspend));
		jsonRtn->Set(String::NewSymbol("BusinessUnit"), String::New(pInputOrder->BusinessUnit));
		jsonRtn->Set(String::NewSymbol("RequestID"), Int32::New(pInputOrder->RequestID));
		jsonRtn->Set(String::NewSymbol("UserForceClose"), Int32::New(pInputOrder->UserForceClose));
		jsonRtn->Set(String::NewSymbol("IsSwapOrder"), Int32::New(pInputOrder->IsSwapOrder));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_errorderinsert(CbRtnField* data, Local<Value>*cbArray) {
    if (data->rtnField){ 
	    CThostFtdcInputOrderField* pInputOrder = static_cast<CThostFtdcInputOrderField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pInputOrder->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pInputOrder->InvestorID));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pInputOrder->InstrumentID));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pInputOrder->OrderRef));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pInputOrder->UserID));
		jsonRtn->Set(String::NewSymbol("OrderPriceType"), String::New(charto_string(pInputOrder->OrderPriceType).c_str()));
		jsonRtn->Set(String::NewSymbol("Direction"), String::New(charto_string(pInputOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(String::NewSymbol("CombOffsetFlag"), String::New(pInputOrder->CombOffsetFlag));
		jsonRtn->Set(String::NewSymbol("CombHedgeFlag"), String::New(pInputOrder->CombHedgeFlag));
		jsonRtn->Set(String::NewSymbol("LimitPrice"), Number::New(pInputOrder->LimitPrice));
		jsonRtn->Set(String::NewSymbol("VolumeTotalOriginal"), Int32::New(pInputOrder->VolumeTotalOriginal));
		jsonRtn->Set(String::NewSymbol("TimeCondition"), String::New(charto_string(pInputOrder->TimeCondition).c_str()));
		jsonRtn->Set(String::NewSymbol("GTDDate"), String::New(pInputOrder->GTDDate));
		jsonRtn->Set(String::NewSymbol("VolumeCondition"), String::New(charto_string(pInputOrder->VolumeCondition).c_str()));
		jsonRtn->Set(String::NewSymbol("MinVolume"), Int32::New(pInputOrder->MinVolume));
		jsonRtn->Set(String::NewSymbol("ContingentCondition"), String::New(charto_string(pInputOrder->ContingentCondition).c_str()));
		jsonRtn->Set(String::NewSymbol("StopPrice"), Number::New(pInputOrder->StopPrice));
		jsonRtn->Set(String::NewSymbol("ForceCloseReason"), String::New(charto_string(pInputOrder->ForceCloseReason).c_str()));
		jsonRtn->Set(String::NewSymbol("IsAutoSuspend"), Int32::New(pInputOrder->IsAutoSuspend));
		jsonRtn->Set(String::NewSymbol("BusinessUnit"), String::New(pInputOrder->BusinessUnit));
		jsonRtn->Set(String::NewSymbol("RequestID"), Int32::New(pInputOrder->RequestID));
		jsonRtn->Set(String::NewSymbol("UserForceClose"), Int32::New(pInputOrder->UserForceClose));
		jsonRtn->Set(String::NewSymbol("IsSwapOrder"), Int32::New(pInputOrder->IsSwapOrder));
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Local<Value>::New(Undefined());
	}
	*(cbArray + 1) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_orderaction(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcInputOrderActionField* pInputOrderAction = static_cast<CThostFtdcInputOrderActionField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pInputOrderAction->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pInputOrderAction->InvestorID));
		jsonRtn->Set(String::NewSymbol("OrderActionRef"), Int32::New(pInputOrderAction->OrderActionRef));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pInputOrderAction->OrderRef));
		jsonRtn->Set(String::NewSymbol("RequestID"), Int32::New(pInputOrderAction->RequestID));
		jsonRtn->Set(String::NewSymbol("FrontID"), Int32::New(pInputOrderAction->FrontID));
		jsonRtn->Set(String::NewSymbol("SessionID"), Int32::New(pInputOrderAction->SessionID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pInputOrderAction->ExchangeID));
		jsonRtn->Set(String::NewSymbol("OrderSysID"), String::New(pInputOrderAction->OrderSysID));
		jsonRtn->Set(String::NewSymbol("ActionFlag"), String::New(charto_string(pInputOrderAction->ActionFlag).c_str()));
		jsonRtn->Set(String::NewSymbol("LimitPrice"), Number::New(pInputOrderAction->LimitPrice));
		jsonRtn->Set(String::NewSymbol("VolumeChange"), Int32::New(pInputOrderAction->VolumeChange));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pInputOrderAction->UserID));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pInputOrderAction->InstrumentID));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_errorderaction(CbRtnField* data, Local<Value>*cbArray) {
    if (data->rtnField){ 
	    CThostFtdcOrderActionField* pOrderAction = static_cast<CThostFtdcOrderActionField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pOrderAction->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pOrderAction->InvestorID));
		jsonRtn->Set(String::NewSymbol("OrderActionRef"), Int32::New(pOrderAction->OrderActionRef));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pOrderAction->OrderRef));
		jsonRtn->Set(String::NewSymbol("RequestID"), Int32::New(pOrderAction->RequestID));
		jsonRtn->Set(String::NewSymbol("FrontID"), Int32::New(pOrderAction->FrontID));
		jsonRtn->Set(String::NewSymbol("SessionID"), Int32::New(pOrderAction->SessionID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pOrderAction->ExchangeID));
		jsonRtn->Set(String::NewSymbol("OrderSysID"), String::New(pOrderAction->OrderSysID));
		jsonRtn->Set(String::NewSymbol("ActionFlag"), String::New(charto_string(pOrderAction->ActionFlag).c_str()));
		jsonRtn->Set(String::NewSymbol("LimitPrice"), Number::New(pOrderAction->LimitPrice));
		jsonRtn->Set(String::NewSymbol("VolumeChange"), Int32::New(pOrderAction->VolumeChange));
		jsonRtn->Set(String::NewSymbol("ActionDate"), String::New(pOrderAction->ActionDate));
		jsonRtn->Set(String::NewSymbol("TraderID"), String::New(pOrderAction->TraderID));
		jsonRtn->Set(String::NewSymbol("InstallID"), Int32::New(pOrderAction->InstallID));
		jsonRtn->Set(String::NewSymbol("OrderLocalID"), String::New(pOrderAction->OrderLocalID));
		jsonRtn->Set(String::NewSymbol("ActionLocalID"), String::New(pOrderAction->ActionLocalID));
		jsonRtn->Set(String::NewSymbol("ParticipantID"), String::New(pOrderAction->ParticipantID));
		jsonRtn->Set(String::NewSymbol("ClientID"), String::New(pOrderAction->ClientID));
		jsonRtn->Set(String::NewSymbol("BusinessUnit"), String::New(pOrderAction->BusinessUnit));
		jsonRtn->Set(String::NewSymbol("OrderActionStatus"), String::New(charto_string(pOrderAction->OrderActionStatus).c_str()));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pOrderAction->UserID));
		jsonRtn->Set(String::NewSymbol("StatusMsg"), String::New(pOrderAction->StatusMsg));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pOrderAction->InstrumentID));
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Local<Value>::New(Undefined());
	}
	*(cbArray + 1) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rspqryorder(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcOrderField* pOrder = static_cast<CThostFtdcOrderField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pOrder->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pOrder->InvestorID));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pOrder->InstrumentID));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pOrder->OrderRef));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pOrder->UserID));
		jsonRtn->Set(String::NewSymbol("OrderPriceType"), String::New(charto_string(pOrder->OrderPriceType).c_str()));
		jsonRtn->Set(String::NewSymbol("Direction"), String::New(charto_string(pOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(String::NewSymbol("CombOffsetFlag"), String::New(pOrder->CombOffsetFlag));
		jsonRtn->Set(String::NewSymbol("CombHedgeFlag"), String::New(pOrder->CombHedgeFlag));
		jsonRtn->Set(String::NewSymbol("LimitPrice"), Number::New(pOrder->LimitPrice));
		jsonRtn->Set(String::NewSymbol("VolumeTotalOriginal"), Int32::New(pOrder->VolumeTotalOriginal));
		jsonRtn->Set(String::NewSymbol("TimeCondition"), String::New(charto_string(pOrder->TimeCondition).c_str()));
		jsonRtn->Set(String::NewSymbol("GTDDate"), String::New(pOrder->GTDDate));
		jsonRtn->Set(String::NewSymbol("VolumeCondition"), String::New(charto_string(pOrder->VolumeCondition).c_str()));
		jsonRtn->Set(String::NewSymbol("MinVolume"), Int32::New(pOrder->MinVolume));
		jsonRtn->Set(String::NewSymbol("ContingentCondition"), String::New(charto_string(pOrder->ContingentCondition).c_str()));
		jsonRtn->Set(String::NewSymbol("StopPrice"), Number::New(pOrder->StopPrice));
		jsonRtn->Set(String::NewSymbol("ForceCloseReason"), String::New(charto_string(pOrder->ForceCloseReason).c_str()));
		jsonRtn->Set(String::NewSymbol("IsAutoSuspend"), Int32::New(pOrder->IsAutoSuspend));
		jsonRtn->Set(String::NewSymbol("BusinessUnit"), String::New(pOrder->BusinessUnit));
		jsonRtn->Set(String::NewSymbol("RequestID"), Int32::New(pOrder->RequestID));
		jsonRtn->Set(String::NewSymbol("OrderLocalID"), String::New(pOrder->OrderLocalID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pOrder->ExchangeID));
		jsonRtn->Set(String::NewSymbol("ParticipantID"), String::New(pOrder->ParticipantID));
		jsonRtn->Set(String::NewSymbol("ClientID"), String::New(pOrder->ClientID));
		jsonRtn->Set(String::NewSymbol("ExchangeInstID"), String::New(pOrder->ExchangeInstID));
		jsonRtn->Set(String::NewSymbol("TraderID"), String::New(pOrder->TraderID));
		jsonRtn->Set(String::NewSymbol("InstallID"), Int32::New(pOrder->InstallID));
		jsonRtn->Set(String::NewSymbol("OrderSubmitStatus"), String::New(charto_string(pOrder->OrderSubmitStatus).c_str()));
		jsonRtn->Set(String::NewSymbol("NotifySequence"), Int32::New(pOrder->NotifySequence));
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pOrder->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(pOrder->SettlementID));
		jsonRtn->Set(String::NewSymbol("OrderSysID"), String::New(pOrder->OrderSysID));
		jsonRtn->Set(String::NewSymbol("OrderSource"), Int32::New(pOrder->OrderSource));
		jsonRtn->Set(String::NewSymbol("OrderStatus"), String::New(charto_string(pOrder->OrderStatus).c_str()));
		jsonRtn->Set(String::NewSymbol("OrderType"), String::New(charto_string(pOrder->OrderType).c_str()));
		jsonRtn->Set(String::NewSymbol("VolumeTraded"), Int32::New(pOrder->VolumeTraded));
		jsonRtn->Set(String::NewSymbol("VolumeTotal"), Int32::New(pOrder->VolumeTotal));
		jsonRtn->Set(String::NewSymbol("InsertDate"), String::New(pOrder->InsertDate));
		jsonRtn->Set(String::NewSymbol("InsertTime"), String::New(pOrder->InsertTime));
		jsonRtn->Set(String::NewSymbol("ActiveTime"), String::New(pOrder->ActiveTime));
		jsonRtn->Set(String::NewSymbol("SuspendTime"), String::New(pOrder->SuspendTime));
		jsonRtn->Set(String::NewSymbol("UpdateTime"), String::New(pOrder->UpdateTime));
		jsonRtn->Set(String::NewSymbol("CancelTime"), String::New(pOrder->CancelTime));
		jsonRtn->Set(String::NewSymbol("ActiveTraderID"), String::New(pOrder->ActiveTraderID));
		jsonRtn->Set(String::NewSymbol("ClearingPartID"), String::New(pOrder->ClearingPartID));
		jsonRtn->Set(String::NewSymbol("SequenceNo"), Int32::New(pOrder->SequenceNo));
		jsonRtn->Set(String::NewSymbol("FrontID"), Int32::New(pOrder->FrontID));
		jsonRtn->Set(String::NewSymbol("SessionID"), Int32::New(pOrder->SessionID));
		jsonRtn->Set(String::NewSymbol("UserProductInfo"), String::New(pOrder->UserProductInfo));
		jsonRtn->Set(String::NewSymbol("StatusMsg"), String::New(pOrder->StatusMsg));
		jsonRtn->Set(String::NewSymbol("UserForceClose"), Int32::New(pOrder->UserForceClose));
		jsonRtn->Set(String::NewSymbol("ActiveUserID"), String::New(pOrder->ActiveUserID));
		jsonRtn->Set(String::NewSymbol("BrokerOrderSeq"), Int32::New(pOrder->BrokerOrderSeq));
		jsonRtn->Set(String::NewSymbol("RelativeOrderSysID"), String::New(pOrder->RelativeOrderSysID));
		jsonRtn->Set(String::NewSymbol("ZCETotalTradedVolume"), Int32::New(pOrder->ZCETotalTradedVolume));
		jsonRtn->Set(String::NewSymbol("IsSwapOrder"), Int32::New(pOrder->IsSwapOrder));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rtnorder(CbRtnField* data, Local<Value>*cbArray) {
    if (data->rtnField){ 
	    CThostFtdcOrderField* pOrder = static_cast<CThostFtdcOrderField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pOrder->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pOrder->InvestorID));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pOrder->InstrumentID));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pOrder->OrderRef));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pOrder->UserID));
		jsonRtn->Set(String::NewSymbol("OrderPriceType"), String::New(charto_string(pOrder->OrderPriceType).c_str()));
		jsonRtn->Set(String::NewSymbol("Direction"), String::New(charto_string(pOrder->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(String::NewSymbol("CombOffsetFlag"), String::New(pOrder->CombOffsetFlag));
		jsonRtn->Set(String::NewSymbol("CombHedgeFlag"), String::New(pOrder->CombHedgeFlag));
		jsonRtn->Set(String::NewSymbol("LimitPrice"), Number::New(pOrder->LimitPrice));
		jsonRtn->Set(String::NewSymbol("VolumeTotalOriginal"), Int32::New(pOrder->VolumeTotalOriginal));
		jsonRtn->Set(String::NewSymbol("TimeCondition"), String::New(charto_string(pOrder->TimeCondition).c_str()));
		jsonRtn->Set(String::NewSymbol("GTDDate"), String::New(pOrder->GTDDate));
		jsonRtn->Set(String::NewSymbol("VolumeCondition"), String::New(charto_string(pOrder->VolumeCondition).c_str()));
		jsonRtn->Set(String::NewSymbol("MinVolume"), Int32::New(pOrder->MinVolume));
		jsonRtn->Set(String::NewSymbol("ContingentCondition"), String::New(charto_string(pOrder->ContingentCondition).c_str()));
		jsonRtn->Set(String::NewSymbol("StopPrice"), Number::New(pOrder->StopPrice));
		jsonRtn->Set(String::NewSymbol("ForceCloseReason"), String::New(charto_string(pOrder->ForceCloseReason).c_str()));
		jsonRtn->Set(String::NewSymbol("IsAutoSuspend"), Int32::New(pOrder->IsAutoSuspend));
		jsonRtn->Set(String::NewSymbol("BusinessUnit"), String::New(pOrder->BusinessUnit));
		jsonRtn->Set(String::NewSymbol("RequestID"), Int32::New(pOrder->RequestID));
		jsonRtn->Set(String::NewSymbol("OrderLocalID"), String::New(pOrder->OrderLocalID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pOrder->ExchangeID));
		jsonRtn->Set(String::NewSymbol("ParticipantID"), String::New(pOrder->ParticipantID));
		jsonRtn->Set(String::NewSymbol("ClientID"), String::New(pOrder->ClientID));
		jsonRtn->Set(String::NewSymbol("ExchangeInstID"), String::New(pOrder->ExchangeInstID));
		jsonRtn->Set(String::NewSymbol("TraderID"), String::New(pOrder->TraderID));
		jsonRtn->Set(String::NewSymbol("InstallID"), Int32::New(pOrder->InstallID));
		jsonRtn->Set(String::NewSymbol("OrderSubmitStatus"), String::New(charto_string(pOrder->OrderSubmitStatus).c_str()));
		jsonRtn->Set(String::NewSymbol("NotifySequence"), Int32::New(pOrder->NotifySequence));
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pOrder->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(pOrder->SettlementID));
		jsonRtn->Set(String::NewSymbol("OrderSysID"), String::New(pOrder->OrderSysID));
		jsonRtn->Set(String::NewSymbol("OrderSource"), Int32::New(pOrder->OrderSource));
		jsonRtn->Set(String::NewSymbol("OrderStatus"), String::New(charto_string(pOrder->OrderStatus).c_str()));
		jsonRtn->Set(String::NewSymbol("OrderType"), String::New(charto_string(pOrder->OrderType).c_str()));
		jsonRtn->Set(String::NewSymbol("VolumeTraded"), Int32::New(pOrder->VolumeTraded));
		jsonRtn->Set(String::NewSymbol("VolumeTotal"), Int32::New(pOrder->VolumeTotal));
		jsonRtn->Set(String::NewSymbol("InsertDate"), String::New(pOrder->InsertDate));
		jsonRtn->Set(String::NewSymbol("InsertTime"), String::New(pOrder->InsertTime));
		jsonRtn->Set(String::NewSymbol("ActiveTime"), String::New(pOrder->ActiveTime));
		jsonRtn->Set(String::NewSymbol("SuspendTime"), String::New(pOrder->SuspendTime));
		jsonRtn->Set(String::NewSymbol("UpdateTime"), String::New(pOrder->UpdateTime));
		jsonRtn->Set(String::NewSymbol("CancelTime"), String::New(pOrder->CancelTime));
		jsonRtn->Set(String::NewSymbol("ActiveTraderID"), String::New(pOrder->ActiveTraderID));
		jsonRtn->Set(String::NewSymbol("ClearingPartID"), String::New(pOrder->ClearingPartID));
		jsonRtn->Set(String::NewSymbol("SequenceNo"), Int32::New(pOrder->SequenceNo));
		jsonRtn->Set(String::NewSymbol("FrontID"), Int32::New(pOrder->FrontID));
		jsonRtn->Set(String::NewSymbol("SessionID"), Int32::New(pOrder->SessionID));
		jsonRtn->Set(String::NewSymbol("UserProductInfo"), String::New(pOrder->UserProductInfo));
		jsonRtn->Set(String::NewSymbol("StatusMsg"), String::New(pOrder->StatusMsg));
		jsonRtn->Set(String::NewSymbol("UserForceClose"), Int32::New(pOrder->UserForceClose));
		jsonRtn->Set(String::NewSymbol("ActiveUserID"), String::New(pOrder->ActiveUserID));
		jsonRtn->Set(String::NewSymbol("BrokerOrderSeq"), Int32::New(pOrder->BrokerOrderSeq));
		jsonRtn->Set(String::NewSymbol("RelativeOrderSysID"), String::New(pOrder->RelativeOrderSysID));
		jsonRtn->Set(String::NewSymbol("ZCETotalTradedVolume"), Int32::New(pOrder->ZCETotalTradedVolume));
		jsonRtn->Set(String::NewSymbol("IsSwapOrder"), Int32::New(pOrder->IsSwapOrder));
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Local<Value>::New(Undefined());
	}
	return;
}
void WrapTrader::pkg_cb_rqtrade(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcTradeField* pTrade = static_cast<CThostFtdcTradeField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pTrade->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pTrade->InvestorID));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pTrade->InstrumentID));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pTrade->OrderRef));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pTrade->UserID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pTrade->ExchangeID));
		jsonRtn->Set(String::NewSymbol("TradeID"), String::New(pTrade->TradeID));
		jsonRtn->Set(String::NewSymbol("Direction"), String::New(charto_string(pTrade->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(String::NewSymbol("OrderSysID"), String::New(pTrade->OrderSysID));
		jsonRtn->Set(String::NewSymbol("ParticipantID"), String::New(pTrade->ParticipantID));
		jsonRtn->Set(String::NewSymbol("ClientID"), String::New(pTrade->ClientID));
		jsonRtn->Set(String::NewSymbol("TradingRole"), String::New(charto_string(pTrade->TradingRole).c_str()));
		jsonRtn->Set(String::NewSymbol("ExchangeInstID"), String::New(pTrade->ExchangeInstID));
		jsonRtn->Set(String::NewSymbol("OffsetFlag"), String::New(charto_string(pTrade->OffsetFlag).c_str()));
		jsonRtn->Set(String::NewSymbol("HedgeFlag"), String::New(charto_string(pTrade->HedgeFlag).c_str()));
		jsonRtn->Set(String::NewSymbol("Price"), Number::New(pTrade->Price));
		jsonRtn->Set(String::NewSymbol("Volume"), Int32::New(pTrade->Volume));
		jsonRtn->Set(String::NewSymbol("TradeDate"), String::New(pTrade->TradeDate));
		jsonRtn->Set(String::NewSymbol("TradeTime"), String::New(pTrade->TradeTime));
		jsonRtn->Set(String::NewSymbol("TradeType"), String::New(charto_string(pTrade->TradeType).c_str()));
		jsonRtn->Set(String::NewSymbol("PriceSource"), String::New(charto_string(pTrade->PriceSource).c_str()));
		jsonRtn->Set(String::NewSymbol("TraderID"), String::New(pTrade->TraderID));
		jsonRtn->Set(String::NewSymbol("OrderLocalID"), String::New(pTrade->OrderLocalID));
		jsonRtn->Set(String::NewSymbol("ClearingPartID"), String::New(pTrade->ClearingPartID));
		jsonRtn->Set(String::NewSymbol("BusinessUnit"), String::New(pTrade->BusinessUnit));
		jsonRtn->Set(String::NewSymbol("SequenceNo"), Int32::New(pTrade->SequenceNo));
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pTrade->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(pTrade->SettlementID));
		jsonRtn->Set(String::NewSymbol("BrokerOrderSeq"), Int32::New(pTrade->BrokerOrderSeq));
		jsonRtn->Set(String::NewSymbol("TradeSource"), String::New(charto_string(pTrade->TradeSource).c_str()));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rtntrade(CbRtnField* data, Local<Value>*cbArray) {
    if (data->rtnField){ 
	    CThostFtdcTradeField* pTrade = static_cast<CThostFtdcTradeField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pTrade->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pTrade->InvestorID));
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pTrade->InstrumentID));
		jsonRtn->Set(String::NewSymbol("OrderRef"), String::New(pTrade->OrderRef));
		jsonRtn->Set(String::NewSymbol("UserID"), String::New(pTrade->UserID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pTrade->ExchangeID));
		jsonRtn->Set(String::NewSymbol("TradeID"), String::New(pTrade->TradeID));
		jsonRtn->Set(String::NewSymbol("Direction"), String::New(charto_string(pTrade->Direction).c_str()));  //var charval = String.fromCharCode(asciival);
		jsonRtn->Set(String::NewSymbol("OrderSysID"), String::New(pTrade->OrderSysID));
		jsonRtn->Set(String::NewSymbol("ParticipantID"), String::New(pTrade->ParticipantID));
		jsonRtn->Set(String::NewSymbol("ClientID"), String::New(pTrade->ClientID));
		jsonRtn->Set(String::NewSymbol("TradingRole"), Int32::New(pTrade->TradingRole));
		jsonRtn->Set(String::NewSymbol("ExchangeInstID"), String::New(pTrade->ExchangeInstID));
		jsonRtn->Set(String::NewSymbol("OffsetFlag"), Int32::New(pTrade->OffsetFlag));
		jsonRtn->Set(String::NewSymbol("HedgeFlag"), Int32::New(pTrade->HedgeFlag));
		jsonRtn->Set(String::NewSymbol("Price"), Number::New(pTrade->Price));
		jsonRtn->Set(String::NewSymbol("Volume"), Int32::New(pTrade->Volume));
		jsonRtn->Set(String::NewSymbol("TradeDate"), String::New(pTrade->TradeDate));
		jsonRtn->Set(String::NewSymbol("TradeTime"), String::New(pTrade->TradeTime));
		jsonRtn->Set(String::NewSymbol("TradeType"), Int32::New(pTrade->TradeType));
		jsonRtn->Set(String::NewSymbol("PriceSource"), String::New(charto_string(pTrade->PriceSource).c_str()));
		jsonRtn->Set(String::NewSymbol("TraderID"), String::New(pTrade->TraderID));
		jsonRtn->Set(String::NewSymbol("OrderLocalID"), String::New(pTrade->OrderLocalID));
		jsonRtn->Set(String::NewSymbol("ClearingPartID"), String::New(pTrade->ClearingPartID));
		jsonRtn->Set(String::NewSymbol("BusinessUnit"), String::New(pTrade->BusinessUnit));
		jsonRtn->Set(String::NewSymbol("SequenceNo"), Int32::New(pTrade->SequenceNo));
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pTrade->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(pTrade->SettlementID));
		jsonRtn->Set(String::NewSymbol("BrokerOrderSeq"), Int32::New(pTrade->BrokerOrderSeq));
		jsonRtn->Set(String::NewSymbol("TradeSource"), String::New(charto_string(pTrade->TradeSource).c_str()));
		*cbArray = jsonRtn;
	}
	else {
		*cbArray = Local<Value>::New(Undefined());
	}
	 
	return;
}
void WrapTrader::pkg_cb_rqinvestorposition(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcInvestorPositionField* _pInvestorPosition = static_cast<CThostFtdcInvestorPositionField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(_pInvestorPosition->InstrumentID));
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(_pInvestorPosition->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(_pInvestorPosition->InvestorID)); 
		jsonRtn->Set(String::NewSymbol("PosiDirection"), String::New(charto_string(_pInvestorPosition->PosiDirection).c_str()));
		jsonRtn->Set(String::NewSymbol("HedgeFlag"), String::New(charto_string(_pInvestorPosition->HedgeFlag).c_str()));
		jsonRtn->Set(String::NewSymbol("PositionDate"), Int32::New(_pInvestorPosition->PositionDate));
		jsonRtn->Set(String::NewSymbol("YdPosition"), Int32::New(_pInvestorPosition->YdPosition));
		jsonRtn->Set(String::NewSymbol("Position"), Int32::New(_pInvestorPosition->Position));
		jsonRtn->Set(String::NewSymbol("LongFrozen"), Int32::New(_pInvestorPosition->LongFrozen));
		jsonRtn->Set(String::NewSymbol("ShortFrozen"), Int32::New(_pInvestorPosition->ShortFrozen));
		jsonRtn->Set(String::NewSymbol("LongFrozenAmount"), Number::New(_pInvestorPosition->LongFrozenAmount));
		jsonRtn->Set(String::NewSymbol("ShortFrozenAmount"), Number::New(_pInvestorPosition->ShortFrozenAmount));
		jsonRtn->Set(String::NewSymbol("OpenVolume"), Int32::New(_pInvestorPosition->OpenVolume));
		jsonRtn->Set(String::NewSymbol("CloseVolume"), Int32::New(_pInvestorPosition->CloseVolume));
		jsonRtn->Set(String::NewSymbol("OpenAmount"), Number::New(_pInvestorPosition->OpenAmount));
		jsonRtn->Set(String::NewSymbol("CloseAmount"), Number::New(_pInvestorPosition->CloseAmount));
		jsonRtn->Set(String::NewSymbol("PositionCost"), Number::New(_pInvestorPosition->PositionCost));
		jsonRtn->Set(String::NewSymbol("PreMargin"), Number::New(_pInvestorPosition->PreMargin));
		jsonRtn->Set(String::NewSymbol("UseMargin"), Number::New(_pInvestorPosition->UseMargin));
		jsonRtn->Set(String::NewSymbol("FrozenMargin"), Number::New(_pInvestorPosition->FrozenMargin));
		jsonRtn->Set(String::NewSymbol("FrozenCash"), Number::New(_pInvestorPosition->FrozenCash));
		jsonRtn->Set(String::NewSymbol("FrozenCommission"), Number::New(_pInvestorPosition->FrozenCommission));
		jsonRtn->Set(String::NewSymbol("CashIn"), Number::New(_pInvestorPosition->CashIn));
		jsonRtn->Set(String::NewSymbol("Commission"), Number::New(_pInvestorPosition->Commission));
		jsonRtn->Set(String::NewSymbol("CloseProfit"), Number::New(_pInvestorPosition->CloseProfit));
		jsonRtn->Set(String::NewSymbol("PositionProfit"), Number::New(_pInvestorPosition->PositionProfit));
		jsonRtn->Set(String::NewSymbol("PreSettlementPrice"), Number::New(_pInvestorPosition->PreSettlementPrice));
		jsonRtn->Set(String::NewSymbol("SettlementPrice"), Number::New(_pInvestorPosition->SettlementPrice));
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(_pInvestorPosition->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(_pInvestorPosition->SettlementID));
		jsonRtn->Set(String::NewSymbol("OpenCost"), Number::New(_pInvestorPosition->OpenCost));
		jsonRtn->Set(String::NewSymbol("ExchangeMargin"), Number::New(_pInvestorPosition->ExchangeMargin));
		jsonRtn->Set(String::NewSymbol("CombPosition"), Int32::New(_pInvestorPosition->CombPosition));
		jsonRtn->Set(String::NewSymbol("CombLongFrozen"), Int32::New(_pInvestorPosition->CombLongFrozen));
		jsonRtn->Set(String::NewSymbol("CombShortFrozen"), Int32::New(_pInvestorPosition->CombShortFrozen));
		jsonRtn->Set(String::NewSymbol("CloseProfitByDate"), Number::New(_pInvestorPosition->CloseProfitByDate));
		jsonRtn->Set(String::NewSymbol("CloseProfitByTrade"), Number::New(_pInvestorPosition->CloseProfitByTrade));
		jsonRtn->Set(String::NewSymbol("TodayPosition"), Int32::New(_pInvestorPosition->TodayPosition));
		jsonRtn->Set(String::NewSymbol("MarginRateByMoney"), Number::New(_pInvestorPosition->MarginRateByMoney));
		jsonRtn->Set(String::NewSymbol("MarginRateByVolume"), Number::New(_pInvestorPosition->MarginRateByVolume));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rqinvestorpositiondetail(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail = static_cast<CThostFtdcInvestorPositionDetailField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pInvestorPositionDetail->InstrumentID));
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pInvestorPositionDetail->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pInvestorPositionDetail->InvestorID));
		jsonRtn->Set(String::NewSymbol("HedgeFlag"), String::New(charto_string(pInvestorPositionDetail->HedgeFlag).c_str()));
		jsonRtn->Set(String::NewSymbol("Direction"), String::New(charto_string(pInvestorPositionDetail->Direction).c_str()));
		jsonRtn->Set(String::NewSymbol("OpenDate"), String::New(pInvestorPositionDetail->OpenDate));
		jsonRtn->Set(String::NewSymbol("TradeID"), String::New(pInvestorPositionDetail->TradeID));
		jsonRtn->Set(String::NewSymbol("Volume"), Int32::New(pInvestorPositionDetail->Volume));
		jsonRtn->Set(String::NewSymbol("OpenPrice"), Number::New(pInvestorPositionDetail->OpenPrice));
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pInvestorPositionDetail->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(pInvestorPositionDetail->SettlementID));
		jsonRtn->Set(String::NewSymbol("TradeType"), String::New(charto_string(pInvestorPositionDetail->TradeType).c_str()));
		jsonRtn->Set(String::NewSymbol("CombInstrumentID"), String::New(pInvestorPositionDetail->CombInstrumentID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pInvestorPositionDetail->ExchangeID));
		jsonRtn->Set(String::NewSymbol("CloseProfitByDate"), Number::New(pInvestorPositionDetail->CloseProfitByDate));
		jsonRtn->Set(String::NewSymbol("CloseProfitByTrade"), Number::New(pInvestorPositionDetail->CloseProfitByTrade));
		jsonRtn->Set(String::NewSymbol("PositionProfitByDate"), Number::New(pInvestorPositionDetail->PositionProfitByDate));
		jsonRtn->Set(String::NewSymbol("PositionProfitByTrade"), Number::New(pInvestorPositionDetail->PositionProfitByTrade));
		jsonRtn->Set(String::NewSymbol("Margin"), Number::New(pInvestorPositionDetail->Margin));
		jsonRtn->Set(String::NewSymbol("ExchMargin"), Number::New(pInvestorPositionDetail->ExchMargin));
		jsonRtn->Set(String::NewSymbol("MarginRateByMoney"), Number::New(pInvestorPositionDetail->MarginRateByMoney));
		jsonRtn->Set(String::NewSymbol("MarginRateByVolume"), Number::New(pInvestorPositionDetail->MarginRateByVolume));
		jsonRtn->Set(String::NewSymbol("LastSettlementPrice"), Number::New(pInvestorPositionDetail->LastSettlementPrice));
		jsonRtn->Set(String::NewSymbol("SettlementPrice"), Number::New(pInvestorPositionDetail->SettlementPrice));
		jsonRtn->Set(String::NewSymbol("CloseVolume"), Int32::New(pInvestorPositionDetail->CloseVolume));
		jsonRtn->Set(String::NewSymbol("CloseAmount"), Number::New(pInvestorPositionDetail->CloseAmount));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rqtradingaccount(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcTradingAccountField *pTradingAccount = static_cast<CThostFtdcTradingAccountField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pTradingAccount->BrokerID));
		jsonRtn->Set(String::NewSymbol("AccountID"), String::New(pTradingAccount->AccountID));
		jsonRtn->Set(String::NewSymbol("PreMortgage"), Number::New(pTradingAccount->PreMortgage));
		jsonRtn->Set(String::NewSymbol("PreCredit"), Number::New(pTradingAccount->PreCredit));
		jsonRtn->Set(String::NewSymbol("PreDeposit"), Number::New(pTradingAccount->PreDeposit));
		jsonRtn->Set(String::NewSymbol("PreBalance"), Number::New(pTradingAccount->PreBalance));
		jsonRtn->Set(String::NewSymbol("PreMargin"), Number::New(pTradingAccount->PreMargin));
		jsonRtn->Set(String::NewSymbol("InterestBase"), Number::New(pTradingAccount->InterestBase));
		jsonRtn->Set(String::NewSymbol("Interest"), Number::New(pTradingAccount->Interest));
		jsonRtn->Set(String::NewSymbol("Deposit"), Number::New(pTradingAccount->Deposit));
		jsonRtn->Set(String::NewSymbol("Withdraw"), Number::New(pTradingAccount->Withdraw));
		jsonRtn->Set(String::NewSymbol("FrozenMargin"), Number::New(pTradingAccount->FrozenMargin));
		jsonRtn->Set(String::NewSymbol("FrozenCash"), Number::New(pTradingAccount->FrozenCash));
		jsonRtn->Set(String::NewSymbol("FrozenCommission"), Number::New(pTradingAccount->FrozenCommission));
		jsonRtn->Set(String::NewSymbol("CurrMargin"), Number::New(pTradingAccount->CurrMargin));
		jsonRtn->Set(String::NewSymbol("CashIn"), Number::New(pTradingAccount->CashIn));
		jsonRtn->Set(String::NewSymbol("Commission"), Number::New(pTradingAccount->Commission));
		jsonRtn->Set(String::NewSymbol("CloseProfit"), Number::New(pTradingAccount->CloseProfit));
		jsonRtn->Set(String::NewSymbol("PositionProfit"), Number::New(pTradingAccount->PositionProfit));
		jsonRtn->Set(String::NewSymbol("Balance"), Number::New(pTradingAccount->Balance));
		jsonRtn->Set(String::NewSymbol("Available"), Number::New(pTradingAccount->Available));
		jsonRtn->Set(String::NewSymbol("WithdrawQuota"), Number::New(pTradingAccount->WithdrawQuota));
		jsonRtn->Set(String::NewSymbol("Reserve"), Number::New(pTradingAccount->Reserve));
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pTradingAccount->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(pTradingAccount->SettlementID));
		jsonRtn->Set(String::NewSymbol("Credit"), Number::New(pTradingAccount->Credit));
		jsonRtn->Set(String::NewSymbol("Mortgage"), Number::New(pTradingAccount->Mortgage));
		jsonRtn->Set(String::NewSymbol("ExchangeMargin"), Number::New(pTradingAccount->ExchangeMargin));
		jsonRtn->Set(String::NewSymbol("DeliveryMargin"), Number::New(pTradingAccount->DeliveryMargin));
		jsonRtn->Set(String::NewSymbol("ExchangeDeliveryMargin"), Number::New(pTradingAccount->ExchangeDeliveryMargin));
		jsonRtn->Set(String::NewSymbol("ReserveBalance"), Number::New(pTradingAccount->ReserveBalance));
		jsonRtn->Set(String::NewSymbol("CurrencyID"), String::New(pTradingAccount->CurrencyID));
		jsonRtn->Set(String::NewSymbol("PreFundMortgageIn"), Number::New(pTradingAccount->PreFundMortgageIn));
		jsonRtn->Set(String::NewSymbol("PreFundMortgageOut"), Number::New(pTradingAccount->PreFundMortgageOut));
		jsonRtn->Set(String::NewSymbol("FundMortgageIn"), Number::New(pTradingAccount->FundMortgageIn));
		jsonRtn->Set(String::NewSymbol("FundMortgageOut"), Number::New(pTradingAccount->FundMortgageOut));
		jsonRtn->Set(String::NewSymbol("FundMortgageAvailable"), Number::New(pTradingAccount->FundMortgageAvailable));
		jsonRtn->Set(String::NewSymbol("MortgageableFund"), Number::New(pTradingAccount->MortgageableFund));
		jsonRtn->Set(String::NewSymbol("SpecProductMargin"), Number::New(pTradingAccount->SpecProductMargin));
		jsonRtn->Set(String::NewSymbol("SpecProductFrozenMargin"), Number::New(pTradingAccount->SpecProductFrozenMargin));
		jsonRtn->Set(String::NewSymbol("SpecProductCommission"), Number::New(pTradingAccount->SpecProductCommission));
		jsonRtn->Set(String::NewSymbol("SpecProductFrozenCommission"), Number::New(pTradingAccount->SpecProductFrozenCommission));
		jsonRtn->Set(String::NewSymbol("SpecProductPositionProfit"), Number::New(pTradingAccount->SpecProductPositionProfit));
		jsonRtn->Set(String::NewSymbol("SpecProductCloseProfit"), Number::New(pTradingAccount->SpecProductCloseProfit));
		jsonRtn->Set(String::NewSymbol("SpecProductPositionProfitByAlg"), Number::New(pTradingAccount->SpecProductPositionProfitByAlg));
		jsonRtn->Set(String::NewSymbol("SpecProductExchangeMargin"), Number::New(pTradingAccount->SpecProductExchangeMargin));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rqinstrument(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if (data->rtnField){ 
	    CThostFtdcInstrumentField *pInstrument = static_cast<CThostFtdcInstrumentField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("InstrumentID"), String::New(pInstrument->InstrumentID));
		jsonRtn->Set(String::NewSymbol("ExchangeID"), String::New(pInstrument->ExchangeID));
		jsonRtn->Set(String::NewSymbol("InstrumentName"), String::New(pInstrument->InstrumentName));
		jsonRtn->Set(String::NewSymbol("ExchangeInstID"), String::New(pInstrument->ExchangeInstID));
		jsonRtn->Set(String::NewSymbol("ProductID"), String::New(pInstrument->ProductID));
		jsonRtn->Set(String::NewSymbol("ProductClass"), String::New(charto_string(pInstrument->ProductClass).c_str()));
		jsonRtn->Set(String::NewSymbol("DeliveryYear"), Int32::New(pInstrument->DeliveryYear));
		jsonRtn->Set(String::NewSymbol("DeliveryMonth"), Int32::New(pInstrument->DeliveryMonth));
		jsonRtn->Set(String::NewSymbol("MaxMarketOrderVolume"), Int32::New(pInstrument->MaxMarketOrderVolume));
		jsonRtn->Set(String::NewSymbol("MinMarketOrderVolume"), Int32::New(pInstrument->MinMarketOrderVolume));
		jsonRtn->Set(String::NewSymbol("MaxLimitOrderVolume"), Int32::New(pInstrument->MaxLimitOrderVolume));
		jsonRtn->Set(String::NewSymbol("MinLimitOrderVolume"), Int32::New(pInstrument->MinLimitOrderVolume));
		jsonRtn->Set(String::NewSymbol("VolumeMultiple"), Int32::New(pInstrument->VolumeMultiple));
		jsonRtn->Set(String::NewSymbol("PriceTick"), Number::New(pInstrument->PriceTick));
		jsonRtn->Set(String::NewSymbol("CreateDate"), String::New(pInstrument->CreateDate));
		jsonRtn->Set(String::NewSymbol("OpenDate"), String::New(pInstrument->OpenDate));
		jsonRtn->Set(String::NewSymbol("ExpireDate"), String::New(pInstrument->ExpireDate));
		jsonRtn->Set(String::NewSymbol("StartDelivDate"), String::New(pInstrument->StartDelivDate));
		jsonRtn->Set(String::NewSymbol("EndDelivDate"), String::New(pInstrument->EndDelivDate));
		jsonRtn->Set(String::NewSymbol("InstLifePhase"), Int32::New(pInstrument->InstLifePhase));
		jsonRtn->Set(String::NewSymbol("IsTrading"), Int32::New(pInstrument->IsTrading));
		jsonRtn->Set(String::NewSymbol("PositionType"), String::New(charto_string(pInstrument->PositionType).c_str()));
		jsonRtn->Set(String::NewSymbol("PositionDateType"), String::New(charto_string(pInstrument->PositionDateType).c_str()));
		jsonRtn->Set(String::NewSymbol("LongMarginRatio"), Number::New(pInstrument->LongMarginRatio));
		jsonRtn->Set(String::NewSymbol("ShortMarginRatio"), Number::New(pInstrument->ShortMarginRatio));
		jsonRtn->Set(String::NewSymbol("MaxMarginSideAlgorithm"), String::New(charto_string(pInstrument->MaxMarginSideAlgorithm).c_str()));
		*(cbArray + 2) = jsonRtn;
	}
	else {
		*(cbArray + 2) = Local<Value>::New(Undefined());
	}
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}

void WrapTrader::pkg_cb_rqdepthmarketdata(CbRtnField *data, Local <Value> *cbArray) {
    Isolate *isolate = Isolate::GetCurrent();

    *cbArray = Number::New(isolate, data->nRequestID);
    *(cbArray + 1) = Boolean::New(isolate, data->bIsLast);
    if (data->rtnField) {
        CThostFtdcDepthMarketDataField *pDepthMarketData = static_cast<CThostFtdcDepthMarketDataField *>(data->rtnField);
        Local <Object> jsonRtn = Object::New(isolate);
        jsonRtn->Set(String::NewFromUtf8(isolate, "TradingDay"), String::NewFromUtf8(isolate, pDepthMarketData->TradingDay));
        jsonRtn->Set(String::NewFromUtf8(isolate, "InstrumentID"), String::NewFromUtf8(isolate, pDepthMarketData->InstrumentID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeID"), String::NewFromUtf8(isolate, pDepthMarketData->ExchangeID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ExchangeInstID"), String::NewFromUtf8(isolate, pDepthMarketData->ExchangeInstID));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LastPrice"), Number::New(isolate, pDepthMarketData->LastPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreSettlementPrice"), Number::New(isolate, pDepthMarketData->PreSettlementPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreClosePrice"), Number::New(isolate, pDepthMarketData->PreClosePrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreOpenInterest"), Number::New(isolate, pDepthMarketData->PreOpenInterest));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OpenPrice"), Number::New(isolate, pDepthMarketData->OpenPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "HighestPrice"), Number::New(isolate, pDepthMarketData->HighestPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LowestPrice"), Number::New(isolate, pDepthMarketData->LowestPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Volume"), Number::New(isolate, pDepthMarketData->Volume));
        jsonRtn->Set(String::NewFromUtf8(isolate, "Turnover"), Number::New(isolate, pDepthMarketData->Turnover));
        jsonRtn->Set(String::NewFromUtf8(isolate, "OpenInterest"), Number::New(isolate, pDepthMarketData->OpenInterest));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ClosePrice"), Number::New(isolate, pDepthMarketData->ClosePrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "SettlementPrice"), Number::New(isolate, pDepthMarketData->SettlementPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UpperLimitPrice"), Number::New(isolate, pDepthMarketData->UpperLimitPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "LowerLimitPrice"), Number::New(isolate, pDepthMarketData->LowerLimitPrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "PreDelta"), Number::New(isolate, pDepthMarketData->PreDelta));
        jsonRtn->Set(String::NewFromUtf8(isolate, "CurrDelta"), Number::New(isolate, pDepthMarketData->CurrDelta));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UpdateTime"), String::NewFromUtf8(isolate, pDepthMarketData->UpdateTime));
        jsonRtn->Set(String::NewFromUtf8(isolate, "UpdateMillisec"), Number::New(isolate, pDepthMarketData->UpdateMillisec));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidPrice1"), Number::New(isolate, pDepthMarketData->BidPrice1));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidVolume1"), Number::New(isolate, pDepthMarketData->BidVolume1));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskPrice1"), Number::New(isolate, pDepthMarketData->AskPrice1));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskVolume1"), Number::New(isolate, pDepthMarketData->AskVolume1));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidPrice2"), Number::New(isolate, pDepthMarketData->BidPrice2));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidVolume2"), Number::New(isolate, pDepthMarketData->BidVolume2));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskPrice2"), Number::New(isolate, pDepthMarketData->AskPrice2));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskVolume2"), Number::New(isolate, pDepthMarketData->AskVolume2));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidPrice3"), Number::New(isolate, pDepthMarketData->BidPrice3));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidVolume3"), Number::New(isolate, pDepthMarketData->BidVolume3));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskPrice3"), Number::New(isolate, pDepthMarketData->AskPrice3));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskVolume3"), Number::New(isolate, pDepthMarketData->AskVolume3));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidPrice4"), Number::New(isolate, pDepthMarketData->BidPrice4));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidVolume4"), Number::New(isolate, pDepthMarketData->BidVolume4));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskPrice4"), Number::New(isolate, pDepthMarketData->AskPrice4));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskVolume4"), Number::New(isolate, pDepthMarketData->AskVolume4));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidPrice5"), Number::New(isolate, pDepthMarketData->BidPrice5));
        jsonRtn->Set(String::NewFromUtf8(isolate, "BidVolume5"), Number::New(isolate, pDepthMarketData->BidVolume5));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskPrice5"), Number::New(isolate, pDepthMarketData->AskPrice5));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AskVolume5"), Number::New(isolate, pDepthMarketData->AskVolume5));
        jsonRtn->Set(String::NewFromUtf8(isolate, "AveragePrice"), Number::New(isolate, pDepthMarketData->AveragePrice));
        jsonRtn->Set(String::NewFromUtf8(isolate, "ActionDay"), String::NewFromUtf8(isolate, pDepthMarketData->ActionDay));
        *(cbArray + 2) = jsonRtn;
    }
    else {
        *(cbArray + 2) = Local<Value>::New(isolate, Undefined(isolate));
    }
    *(cbArray + 3) = pkg_rspinfo(data->rspInfo);
    return;
}

void WrapTrader::pkg_cb_rqsettlementinfo(CbRtnField* data, Local<Value>*cbArray) {
	*cbArray = Int32::New(data->nRequestID);
	*(cbArray + 1) = Boolean::New(data->bIsLast)->ToBoolean();
    if(data->rtnField!=NULL){
	    CThostFtdcSettlementInfoField *pSettlementInfo = static_cast<CThostFtdcSettlementInfoField*>(data->rtnField);
		Local<Object> jsonRtn = Object::New();
		jsonRtn->Set(String::NewSymbol("TradingDay"), String::New(pSettlementInfo->TradingDay));
		jsonRtn->Set(String::NewSymbol("SettlementID"), Int32::New(pSettlementInfo->SettlementID));
		jsonRtn->Set(String::NewSymbol("BrokerID"), String::New(pSettlementInfo->BrokerID));
		jsonRtn->Set(String::NewSymbol("InvestorID"), String::New(pSettlementInfo->InvestorID));
		jsonRtn->Set(String::NewSymbol("SequenceNo"), Int32::New(pSettlementInfo->SequenceNo));
	    jsonRtn->Set(String::NewSymbol("Content"), String::New(pSettlementInfo->Content));
	    *(cbArray + 2) = jsonRtn;
	}
	else {
	    *(cbArray + 2) = Local<Value>::New(Undefined());
    }
	*(cbArray + 3) = pkg_rspinfo(data->rspInfo);
	return;
}
void WrapTrader::pkg_cb_rsperror(CbRtnField* data, Local<Value>*cbArray) {
	Isolate *isolate = Isolate::GetCurrent();

	*cbArray = Number::New(isolate, data->nRequestID);
	*(cbArray + 1) = Boolean::New(isolate, data->bIsLast);
	*(cbArray + 2) = pkg_rspinfo(data->rspInfo);
	return;
}
Local<Value> WrapTrader::pkg_rspinfo(void *vpRspInfo) {
	Isolate *isolate = Isolate::GetCurrent();

  if (vpRspInfo) {
      CThostFtdcRspInfoField *pRspInfo = static_cast<CThostFtdcRspInfoField *>(vpRspInfo);
      Local <Object> jsonInfo = Object::New(isolate);
      jsonInfo->Set(String::NewFromUtf8(isolate, "ErrorID"), Number::New(isolate, pRspInfo->ErrorID));
      jsonInfo->Set(String::NewFromUtf8(isolate, "ErrorMsg"), String::NewFromUtf8(isolate, pRspInfo->ErrorMsg));
      return jsonInfo;
  }
  else {
      return Local<Value>::New(isolate, Undefined(isolate));
  }
}
