#include <cstdio>
#include <cstdlib>

#include "TinyJS.h"
#include "TinyJS_Functions.h"
#include "TinyJS_MathFunctions.h"

#include "script_engine.h"

namespace {

static CTinyJS *gJS;

void js_print(CScriptVar *v, void *userdata) {
  printf("Mallie > %s\n", v->getParameter("text")->getString().c_str());
}

void js_print_camera(CScriptVar *v, void *userdata) {
  printf("\nMallie:info\teye:%f,%f,%f\tlookat:%f,%f,%f\tup:%f,%f,%f\n",
         v->getParameter("e")->getArrayIndex(0)->getDouble(),
         v->getParameter("e")->getArrayIndex(1)->getDouble(),
         v->getParameter("e")->getArrayIndex(2)->getDouble(),
         v->getParameter("l")->getArrayIndex(0)->getDouble(),
         v->getParameter("l")->getArrayIndex(1)->getDouble(),
         v->getParameter("l")->getArrayIndex(2)->getDouble(),
         v->getParameter("u")->getArrayIndex(0)->getDouble(),
         v->getParameter("u")->getArrayIndex(1)->getDouble(),
         v->getParameter("u")->getArrayIndex(2)->getDouble());
}

void js_dump(CScriptVar *v, void *userdata) {
  CTinyJS *js = (CTinyJS *)userdata;
  js->root->trace(">  ");
}

} // namespace

void ScriptEngine::Release() { delete gJS; }

void ScriptEngine::Create() {
  Release();

  gJS = new CTinyJS();

  /* add the functions from TinyJS_Functions.cpp and TinyJS_MathFunctions.cpp */
  registerFunctions(gJS);
  registerMathFunctions(gJS);
  /* Add a native function */
  gJS->addNative("function print(text)", &js_print, 0);
  gJS->addNative("function dump()", &js_dump, gJS);
  gJS->addNative("function dumpCamera(e, l, u)", &js_print_camera, 0);

  gJS->execute("print(\"Script initialized.\n\");");

  try {
    gJS->execute(
        "var eye = [0,0,0]; var lookup = [0,0,0]; var up = [0,0,0];\n");
  } catch (CScriptException *e) {
    printf("ERROR: %s\n", e->text.c_str());
  }

  return;
}

bool ScriptEngine::Eval(const char *str) {
  try {
    gJS->execute(str);
  } catch (CScriptException *e) {
    printf("ERROR: %s\n", e->text.c_str());
  }

  return true;
}
