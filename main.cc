#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "version.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef WITH_MPI
#include <mpi.h>
#endif

#ifdef ENABLE_SDL
#include <SDL.h>
#include "main_sdl.h"
#endif

#if defined(__APPLE__)
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>     // usleep
#endif

#include <fstream>

#include "parson.h" // deps/parson
#include "render.h"
#include "mmm_io.h"
#include "timerutil.h"
#include "main_console.h"
#include "scene.h"

namespace {

static int GetNumCPUs()
{
    int cpus = 0;

#ifdef _WIN32
    SYSTEM_INFO info;

    GetSystemInfo(&info);

    if (info.dwNumberOfProcessors > 1) {
        cpus = info.dwNumberOfProcessors;
    }
#elif defined(__APPLE__) /* OS X */
    int    mib[2], rc;
    size_t len;

    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;
    len    = sizeof(cpus);
    rc     = sysctl(mib, 2, &cpus, &len, NULL, 0);

#elif defined(__linux__)  /* linux */
    FILE* fp;
    char  buf[1024];

    fp = fopen("/proc/cpuinfo", "r");
    if (!fp)
        return 0;

    while (!feof(fp)) {
        fgets(buf, 1023, fp);
        if (strncasecmp("ht", buf, strlen("ht")) == 0) {
            /* Hyper Thread */
            cpus++;
        } else if (strncasecmp("processor", buf, strlen("processor")) == 0) {
            /* Pysical CPU processor */
            cpus++;
        }
    }
   
    fclose(fp);

#endif // ifdef _WIN32

    if (cpus < 1) {
        cpus = 1;
    }

    return cpus;
} 

bool
InitScene(
  mallie::Scene& scene,
  mallie::RenderConfig& config)
{
  return scene.Init(config.obj_filename, config.material_filename, config.scene_scale);
}

bool
LoadJSONConfig(
  mallie::RenderConfig& config, // [out]
  const std::string& filename)
{
#if 1
  { // file check
    std::ifstream is(filename.c_str());

    if (!is) {
      std::cerr << "File not found: " << filename << std::endl;
      return false;
    }
  }

  JSON_Value* root = json_parse_file(filename.c_str());
  if (json_value_get_type(root) != JSONObject) {
    return false;
  }

  JSON_Object* object = json_value_get_object(root);
  if (json_value_get_type(json_object_dotget_value(object, "obj_filename")) == JSONString) {
    config.obj_filename = json_object_dotget_string(object, "obj_filename");
  }

  if (json_value_get_type(json_object_dotget_value(object, "material_filename")) == JSONString) {
    config.material_filename = json_object_dotget_string(object, "material_filename");
  }

  if (json_value_get_type(json_object_dotget_value(object, "scene_scale")) == JSONNumber) {
    config.scene_scale = json_object_dotget_number(object, "scene_scale");
  }

  if (json_value_get_type(json_object_dotget_value(object, "eye")) == JSONArray) {
    JSON_Array* array = json_object_dotget_array(object, "eye");
    if (json_array_get_count(array) == 3) {
      config.eye[0] = json_array_get_number(array, 0);
      config.eye[1] = json_array_get_number(array, 1);
      config.eye[2] = json_array_get_number(array, 2);
    }
  }

  if (json_value_get_type(json_object_dotget_value(object, "up")) == JSONArray) {
    JSON_Array* array = json_object_dotget_array(object, "up");
    if (json_array_get_count(array) == 3) {
      config.up[0] = json_array_get_number(array, 0);
      config.up[1] = json_array_get_number(array, 1);
      config.up[2] = json_array_get_number(array, 2);
    }
  }

  if (json_value_get_type(json_object_dotget_value(object, "lookat")) == JSONArray) {
    JSON_Array* array = json_object_dotget_array(object, "lookat");
    if (json_array_get_count(array) == 3) {
      config.lookat[0] = json_array_get_number(array, 0);
      config.lookat[1] = json_array_get_number(array, 1);
      config.lookat[2] = json_array_get_number(array, 2);
    }
  }

  if (json_value_get_type(json_object_dotget_value(object, "resolution")) == JSONArray) {
    JSON_Array* array = json_object_dotget_array(object, "resolution");
    if (json_array_get_count(array) == 2) {
      config.width= json_array_get_number(array, 0);
      config.height = json_array_get_number(array, 1);
    }
  }

  if (json_value_get_type(json_object_dotget_value(object, "num_passes")) == JSONNumber) {
    config.num_passes = json_object_dotget_number(object, "num_passes");
  }

  if (json_value_get_type(json_object_dotget_value(object, "num_photons")) == JSONNumber) {
    config.num_passes = json_object_dotget_number(object, "num_photons");
  }

  if (json_value_get_type(json_object_dotget_value(object, "plane")) == JSONBoolean) {
    config.plane = json_object_dotget_boolean(object, "plane");
  }

  json_value_free(root);

  return true;

#else
  std::ifstream is(filename.c_str());

  if (!is) {
    std::cerr << "File not found: " << filename << std::endl;
    return false;
  }

  picojson::value v;
  std::string err = picojson::parse(v, is);
  if (!err.empty()) {
    std::cout << err << std::endl;
    return false;
  }

  if (v.get("obj_filename").is<std::string>()) {
    config.obj_filename = v.get("obj_filename").get<std::string>();
  }

  if (v.get("material_filename").is<std::string>()) {
    config.material_filename = v.get("material_filename").get<std::string>();
  }

  if (v.get("scene_scale").is<double>()) {
    config.scene_scale = v.get("scene_scale").get<double>();
  }

  if (v.get("plane").is<bool>()) {
    config.plane = v.get("plane").get<bool>();
  }

  if (v.get("num_passes").is<double>()) {
    config.num_passes = (int)v.get("num_passes").get<double>();
  }

  if (v.get("num_photons").is<double>()) {
    config.num_photons = (int)v.get("num_photons").get<double>();
  }

  if (v.get("eye").is<picojson::array>()) {
    assert(v.get("eye").get<picojson::array>().size() == 3);
    config.eye[0] = v.get("eye").get(0).get<double>();
    config.eye[1] = v.get("eye").get(1).get<double>();
    config.eye[2] = v.get("eye").get(2).get<double>();
  }

  if (v.get("up").is<picojson::array>()) {
    assert(v.get("up").get<picojson::array>().size() == 3);
    config.up[0] = v.get("up").get(0).get<double>();
    config.up[1] = v.get("up").get(1).get<double>();
    config.up[2] = v.get("up").get(2).get<double>();
  }

  if (v.get("lookat").is<picojson::array>()) {
    assert(v.get("lookat").get<picojson::array>().size() == 3);
    config.lookat[0] = v.get("lookat").get(0).get<double>();
    config.lookat[1] = v.get("lookat").get(1).get<double>();
    config.lookat[2] = v.get("lookat").get(2).get<double>();
  }

  if (v.get("fov").is<double>()) {
    config.fov = v.get("fov").get<double>();
  }

  if (v.get("resolution").is<picojson::array>()) {
    assert(v.get("resolution").get<picojson::array>().size() == 2);
    config.width = v.get("resolution").get(0).get<double>();
    config.height = v.get("resolution").get(1).get<double>();
  }

  if (v.get("num_passes").is<double>()) {
    config.num_passes = (int)v.get("num_passes").get<double>();
  }

  return true;
#endif
}

}

int
main(
  int argc,
  char **argv)
{

  if (argc > 1) {
    if (strcmp(argv[1], "--help") == 0) {
      printf("Usage: mallie <config.json>\n");
      exit(1);
    }
  }

#ifdef _OPENMP
  printf("[Mallie] OpenMP Detected. Max # of threads = %d\n", omp_get_max_threads());
#endif

#ifdef WITH_MPI
  int rank;
  int nnodes;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nnodes);
#endif
  printf("[Mallie] Version  : %s\n", MALLIE_VERSION);

  if (sizeof(real) == 4) {
    printf("[Mallie] Precision: 32bit float\n");
  } else {
    printf("[Mallie] Precision: 64bit double\n");
  }

  printf("[Mallie] # of CPUs: %d\n", GetNumCPUs());

  // Load config
  std::string config_filename("config.json");
  if (argc > 1) {
    config_filename = std::string(argv[1]);
  }
  printf("[Mallie] Config file: %s\n", config_filename.c_str());
  mallie::RenderConfig config;
  bool ret = LoadJSONConfig(config, config_filename);
  assert(ret);

  mallie::Scene scene;
  ret = InitScene(scene, config);
  assert(ret);

  mallie::timerutil t;
  t.start();
  printf("[Mallie] Begin\n");

#ifdef ENABLE_SDL
  // SDL_Init() must be defined in main()
  SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
  DoMainSDL(scene, config);
#else

  DoMainConsole(scene, config);
#endif

  t.end();
  printf("[Mallie] End\n");
  printf("[Mallie] Elapsed: %d sec(s)\n", (int)t.sec());
  fflush(stdout);

#ifdef WITH_MPI
  MPI_Finalize();
#endif

#ifdef ENABLE_SDL
  SDL_Quit();
#endif

  return EXIT_SUCCESS;
}
 
