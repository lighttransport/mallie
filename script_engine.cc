#include <cstdio>
#include <cstdlib>

#include "duktape.h"

#include "script_engine.h"

namespace {

static duk_context *gJS;

static duk_ret_t native_prime_check(duk_context *ctx) {
    int val = duk_require_int(ctx, 0);
    int lim = duk_require_int(ctx, 1);
    int i;

    for (i = 2; i <= lim; i++) {
        if (val % i == 0) {
            duk_push_false(ctx);
            return 1;
        }
    }

    duk_push_true(ctx);
    return 1;
}

} // namespace

void ScriptEngine::Release() { duk_destroy_heap(gJS); }

void ScriptEngine::Create() {
  Release();

  gJS = duk_create_heap_default();

  duk_push_global_object(gJS);
  duk_push_c_function(gJS, native_prime_check, 2 /*nargs*/);
  duk_put_prop_string(gJS, -2, "primeCheckNative");

  Eval("primeCheckNative(1,3);\n");

  return;
}

bool ScriptEngine::Eval(const char *str) {

  duk_push_string(gJS, str);
  if (duk_peval(gJS) != 0) {
    printf("> eval failed: %s\n", duk_safe_to_string(gJS, -1));
    return false;
  } else {
    printf("> result: %s\n", duk_safe_to_string(gJS, -1));
  }

  duk_pop(gJS);

  return true;
}
