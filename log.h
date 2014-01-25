#ifndef __MALLIE_LOG_H__
#define __MALLIE_LOG_H__

#include <string>

namespace mallie {

class Log {
public:
  Log();
  ~Log();

private:
  std::string message_;
};

} // namespace

#endif // __MALLIE_LOG_H__
