## ===== constructor function implementation template
bool ${signature_name}(JSContext *cx, uint32_t argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    bool ok = true;
#if len($arguments) >= $min_args
    #set arg_count = len($arguments)
    #set arg_idx = $min_args
    #set $count = 0
    #while $count < $arg_idx
        #set $arg = $arguments[$count]
        #if $arg.is_numeric
    ${arg.to_string($generator)} arg${count} = 0;
        #elif $arg.is_pointer
    ${arg.to_string($generator)} arg${count} = nullptr;
        #else
    ${arg.to_string($generator)} arg${count};
        #end if
        #set $count = $count + 1
    #end while
    #set $count = 0
    #set arg_list = ""
    #set arg_array = []
    #while $count < $arg_idx
        #set $arg = $arguments[$count]
    ${arg.to_native({"generator": $generator,
                         "in_value": "args.get(" + str(count) + ")",
                         "out_value": "arg" + str(count),
                         "class_name": $class_name,
                         "level": 2,
                         "ntype": str($arg)})};
        #set $arg_array += ["arg"+str(count)]
        #set $count = $count + 1
    #end while
    #if $arg_idx > 0
    JSB_PRECONDITION2(ok, cx, false, "${signature_name} : Error processing arguments");
    #end if
    #set $arg_list = ", ".join($arg_array)
    ${namespaced_class_name}* cobj = new (std::nothrow) ${namespaced_class_name}($arg_list);
    TypeTest<${namespaced_class_name}> t;
    js_type_class_t *typeClass = nullptr;
    std::string typeName = t.s_name();
    auto typeMapIter = _js_global_type_map.find(typeName);
    CCASSERT(typeMapIter != _js_global_type_map.end(), "Can't find the class type!");
    typeClass = typeMapIter->second;
    CCASSERT(typeClass, "The value is null.");
    JS::RootedObject proto(cx, typeClass->proto.ref());
    JS::RootedObject parent(cx, typeClass->parentProto.ref());
    JS::RootedObject obj(cx, JS_NewObject(cx, typeClass->jsclass, proto, parent));
    jsval objVal = OBJECT_TO_JSVAL(obj);
    #if $is_ref_class
    cobj->autorelease();
        #if $generator.script_control_cpp
    cobj->retain();
    ScriptingCore::retainCount++;
    CCLOG("++++++RETAINED++++++ %d ${namespaced_class_name} ref count: %d", ScriptingCore::retainCount, cobj->getReferenceCount());
    JS::RootedObject hook(cx, JS_NewObject(cx, jsb_FinalizeHook_class, JS::RootedObject(cx, jsb_FinalizeHook_prototype), JS::NullPtr()));
    JS_SetProperty(cx, hook, "owner", JS::RootedValue(cx, objVal));
    JS_SetProperty(cx, obj, "__hook", JS::RootedValue(cx, OBJECT_TO_JSVAL(hook)));
        #end if
    #end if
    args.rval().set(objVal);
    // link the native object with the javascript object
#if not $generator.script_control_cpp
    js_proxy_t* p = jsb_new_proxy(cobj, obj);
    AddNamedObjectRoot(cx, &p->obj, "${namespaced_class_name}");
#else
    jsb_new_proxy(cobj, obj);
#end if
    if (JS_HasProperty(cx, obj, "_ctor", &ok) && ok)
        ScriptingCore::getInstance()->executeFunctionWithOwner(OBJECT_TO_JSVAL(obj), "_ctor", args);
    return true;
#end if
}