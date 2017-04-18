#include <node.h>
#include <v8.h>
#include "wrap_trader.h"
#include "wrap_mduser.h"

using namespace v8;

bool islog;//log?

Handle<Value> CreateTrader(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  args.GetReturnValue().Set(WrapTrader::NewInstance(args));
  // WrapTrader::NewInstance(args);
}

Handle<Value> CreateMdUser(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();
	args.GetReturnValue().Set(WrapMdUser::NewInstance(args));
	// WrapMdUser::NewInstance(args);
}

Handle<Value> Settings(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (!args[0]->IsUndefined() && args[0]->IsObject()) {
		Local<Object> setting = args[0]->ToObject();
		Local<Value> log = setting->Get(String::NewFromUtf8(isolate, "log"));
		if (!log->IsUndefined()) {
			islog = log->BooleanValue();
		}		
	}

	// return args.GetReturnValue().Set(Undefined(isolate));
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
