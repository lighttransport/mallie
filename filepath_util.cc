#include "filepath_util.h"

#include <cstdio>

#ifdef _WIN32
#include <Windows.h>
#else
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include <wordexp.h>
#endif

namespace mallie {

std::vector<std::string> SplitPath(const std::string& s, char c) {
  if (s.empty()) {
    return std::vector<std::string>();
  }
 
  std::vector<std::string> res;
  int begin = 0, end = 0;
  for (int i = 0; i < s.size(); ++i) {
    if (s[i] == c) {
      res.push_back(s.substr(begin, end - begin));
      begin = i + 1;
      end = i + 1;
    } else {
      ++end;
    }
  }
 
  res.push_back(s.substr(begin));
 
  return res;
}
  
std::string ExpandFilePath(const std::string& filepath)
{
#ifdef _WIN32
  // @todo {}
  DWORD len = ExpandEnvironmentStringsA(filepath.c_str(), NULL, 0 );
  char* str = new char [len];
  ExpandEnvironmentStringsA(filepath.c_str(), str, len );

  std::string s(str);

  delete [] str;

  return s;
#else
  
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
  // no expansion
  std::string s = filepath;
#else
  std::string s;
  wordexp_t p;

  if (filepath.empty()) {
    return "";
  }

  //char** w;
  int ret = wordexp( filepath.c_str(), &p, 0 );
  if (ret) {
    fprintf(stderr, "Filepath expansion err: %d\n", ret);
    // err
    s = filepath;
    return s;
  }

  // Use first element only.
  if (p.we_wordv) {
    s = std::string(p.we_wordv[0]);
    wordfree( &p );
  } else {
    s = filepath;
  }

#endif
  
  return s;
#endif
}

std::string GetFilePathExtension(const std::string& FileName)
{
    if(FileName.find_last_of(".") != std::string::npos)
        return FileName.substr(FileName.find_last_of(".")+1);
    return "";
}

std::string GetBaseFilename(const std::string& FileName)
{
    if(FileName.find_last_of(".") != std::string::npos)
        return FileName.substr(0, FileName.find_last_of("."));
    return FileName;
}

std::string JoinPath(const std::string& path0, const std::string& path1)
{
  if (path0.empty()) {
    return path1;
  } else {
    // check '/'
    char lastChar = *path0.rbegin();
    if (lastChar != '/') {
      return path0 + std::string("/") + path1;
    } else {
      return path0 + path1;
    }
  }
}

std::string FindFile(const std::vector<std::string>& paths, const std::string& filepath)
{
  for (size_t i = 0; i < paths.size(); i++) {
    std::string absPath = ExpandFilePath(JoinPath(paths[i], filepath));
    if (FileExists(absPath)) {
      return absPath;
    }
  }

  return std::string();
}

bool
FileExists(const std::string& abs_filename)
{
    bool ret;
    FILE *fp = fopen(abs_filename.c_str(), "rb");
    if (fp) {
        ret = true;
        fclose(fp);
    } else {
        ret = false;
    }

    return ret;
}

} // namespace
