/*
 * QuickJS C library
 *
 * Copyright (c) 2017-2019 Fabrice Bellard
 * Copyright (c) 2017-2019 Charlie Gordon
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "private.h"
#include "tjs.h"

#include <string.h>
#include <uv.h>


/* load and evaluate a file */
static JSValue js_loadScript(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    const char *filename;
    JSValue ret;

    filename = JS_ToCString(ctx, argv[0]);
    if (!filename)
        return JS_EXCEPTION;
    ret = TJS_EvalFile(ctx, filename, JS_EVAL_TYPE_GLOBAL, false);
    JS_FreeCString(ctx, filename);
    return ret;
}

static JSValue js_std_exit(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    int status;
    if (JS_ToInt32(ctx, &status, argv[0]))
        status = -1;
    // Reset TTY state  (if it had changed) before exiting.
    uv_tty_reset_mode();
    exit(status);
    return JS_UNDEFINED;
}

static JSValue js_std_gc(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    JS_RunGC(JS_GetRuntime(ctx));
    return JS_UNDEFINED;
}

static JSValue js_evalScript(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) {
    const char *str;
    size_t len;
    JSValue ret;
    str = JS_ToCStringLen(ctx, &len, argv[0]);
    if (!str)
        return JS_EXCEPTION;
    ret = JS_Eval(ctx, str, len, "<evalScript>", JS_EVAL_TYPE_GLOBAL | JS_EVAL_FLAG_BACKTRACE_BARRIER);
    JS_FreeCString(ctx, str);
    return ret;
}

static const JSCFunctionListEntry js_std_funcs[] = {
    TJS_CFUNC_DEF("exit", 1, js_std_exit),
    TJS_CFUNC_DEF("gc", 0, js_std_gc),
    TJS_CFUNC_DEF("evalScript", 1, js_evalScript),
    TJS_CFUNC_DEF("loadScript", 1, js_loadScript),
};

void tjs__mod_std_init(JSContext *ctx, JSValue ns) {
    JS_SetPropertyFunctionList(ctx, ns, js_std_funcs, countof(js_std_funcs));
}
