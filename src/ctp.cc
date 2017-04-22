﻿#include <node.h>
#include <uv.h>
#include "wrap_trader.h"
#include "wrap_mduser.h"

using namespace v8;

bool islog;//log?

void CreateTrader(const FunctionCallbackInfo<Value>& args) {
  WrapTrader::NewInstance(args);
}

void CreateMdUser(const FunctionCallbackInfo<Value>& args) {
	WrapMdUser::NewInstance(args);
}

void Settings(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (!args[0]->IsUndefined() && args[0]->IsObject()) {
		Local<Object> setting = args[0]->ToObject();
		Local<Value> log = setting->Get(String::NewFromUtf8(isolate, "log"));
		if (!log->IsUndefined()) {
			islog = log->BooleanValue();
		}		
	}

	args.GetReturnValue().Set(Undefined(isolate));
}

void Init(Local<Object> exports) {
	Isolate* isolate = exports->GetIsolate();
	WrapTrader::Init(isolate);
	WrapMdUser::Init(isolate);

	NODE_SET_METHOD(exports, "createTrader", CreateTrader);
  NODE_SET_METHOD(exports, "createMdUser", CreateMdUser);
  NODE_SET_METHOD(exports, "settings", Settings);
}

NODE_MODULE(ctp, Init)
