#ifndef __MALLIE_SCRIPT_ENGINE_H__
#define __MALLIE_SCRIPT_ENGINE_H__

class ScriptEngine {
public:
  ScriptEngine();
  ~ScriptEngine();

  static void Create();
  static void Release();

  static bool Eval(const char *str);
};

#endif // __MALLIE_SCRIPT_ENGINE_H__
