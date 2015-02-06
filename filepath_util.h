#ifndef __MALLIE_FILEPATH_UTIL_H__
#define __MALLIE_FILEPATH_UTIL_H__

#include <vector>
#include <string>

namespace mallie {

// Expand filepath. e.g. '~/myfile' -> '/home/syoyo/myfile'
std::string ExpandFilePath(const std::string& filepath);
std::string GetFilePathExtension(const std::string& FileName);
std::string GetBaseFilename(const std::string& FileName);
std::string FindFile(const std::vector<std::string>& paths, const std::string& FileName);
std::vector<std::string> SplitPath(const std::string& s, char c = ':');

bool FileExists(const std::string& abs_filepath);

// 'aa/' + bb -> 'aa/bb'
std::string JoinPath(const std::string& path0, const std::string& path1);

};

#endif  // __MALLIE_FILEPATH_UTIL_H__
