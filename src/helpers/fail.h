//
// Created by Muscalu Stefan-Gabriel on 15/09/2018.
//

#ifndef DARKNET_BINDINGS_FAIL_H
#define DARKNET_BINDINGS_FAIL_H

template<typename ... Args>
static std::string format(const std::string& format, Args ... args) {
	size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format.c_str(), args ...);
	auto str = std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	return str;
}

#define fail(...) { Napi::Error::New(env, format(__VA_ARGS__)).ThrowAsJavaScriptException(); return; }
#define failt(...) { Napi::TypeError::New(env, format(__VA_ARGS__)).ThrowAsJavaScriptException(); return; }
#define failv(...) { Napi::Error::New(env, format(__VA_ARGS__)).ThrowAsJavaScriptException(); return Napi::Value(); }


#endif //DARKNET_BINDINGS_FAIL_H
