#ifndef NEPOMUK_BINDINGS_NODE_HPP_
#define NEPOMUK_BINDINGS_NODE_HPP_

#include <memory>
#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <nan.h>
#pragma GCC diagnostic pop

#include "service/interface.hpp"
#include "service/master.hpp"

namespace nepomuk
{
namespace binding
{

// the poor worker that has to carry all the load
class Worker final : public Nan::AsyncWorker
{
    using Base = Nan::AsyncWorker;

  public:
    Worker(std::shared_ptr<service::Master> master_service,
           std::shared_ptr<service::Interface> service,
           service::ServiceParameters service_parameters,
           Nan::Callback *callback);

    void Execute() override;
    void HandleOKCallback() override;

  private:
    // to ensure the datasets stay valid
    std::shared_ptr<service::Master> master_service;
    std::shared_ptr<service::Interface> service;
    service::ServiceParameters service_parameters;
};

// This struct provides a node wrapper for the transit service. This allows for querying a server
// for transit routes in their various forms and the debug map. Every service provided by the engine
// needs to be set-up here.
// 
// The class itself is deriving from a wrapped object using the NAN library.
class Engine final : public Nan::ObjectWrap
{
    using Base = Nan::ObjectWrap;

  public:
    // basic set-up of the engine
    // static NAN_MODULE_INIT(init);
    static void init(v8::Local<v8::Object>, v8::Local<v8::Object> module);

    // plugin a new engine element
    // static NAN_METHOD(plug);
    static void plug(const Nan::FunctionCallbackInfo<v8::Value> &);

    // the registered services
    // static NAN_METHOD(request);
    static void request(const Nan::FunctionCallbackInfo<v8::Value> &);

  private:
    // static NAN_METHOD(create);
    static void create(const Nan::FunctionCallbackInfo<v8::Value> &);

    Engine(std::string const &path);
    Engine(Engine &&) = delete;
    Engine &operator=(Engine &&) = delete;

    // Since the engine for the node bindings is constructed from java script land, we need to
    // ensure the consistency / construction calling ourselves.
    // The following function guarantees singleton access into the class.
    static Nan::Persistent<v8::Function> &construct();

  private:
    std::shared_ptr<service::Master> master_service;
};

} // namespace binding
} // namespace nepomuk

// export the module into the global node namespace
NODE_MODULE(nepomuk, nepomuk::binding::Engine::init)

#endif // NEPOMUK_BINDINGS_NODE_HPP_
