#include <tsgen/tsgen.h>

int
tsgen_loadfile(const char *file_name, double **_data, size_t *_len)
{

  if (file_name == NULL)
  {
    STACK_ERROR("%s", "filename can not be (nil)");
    return 0;
  }

  FILE *fp = fopen(file_name, "rb");

  if (fp == NULL)
  {
    STACK_ERROR("unable to open file %s", file_name);
    return 0;
  }

  STACK_INFO("loading timeseries in file %s", file_name);

  uint8_t endiness_bit;
  fread(&endiness_bit, sizeof(uint8_t), 1, fp);

  STACK_INFO("endiness_bit = %x", endiness_bit);

  if (endiness_bit != 1)
  {
    STACK_ERROR("%s", "endiness flag is not equal to 1");
    return 0;
  }

  uint64_t data_points = 0;
  fread(&data_points, sizeof(uint64_t), 1, fp);

  if (data_points == 0)
  {
    STACK_ERROR("%s", "unknown error reading data_points variable from file");
    return 0;
  }

  STACK_INFO("data_points = %lu", data_points);

  *_data = calloc(data_points, sizeof(double));
  CHECK_ALLOC(_data, sizeof(double) * data_points);

  fread(*_data, sizeof(double), data_points, fp);
  *_len = data_points;

  return 1;
}
