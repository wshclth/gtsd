#include <selfsimilarity/selfsimilarity.h>

static
int _write_ts(size_t len, const double *ts, const char *out)
{
  size_t out_path_len = strlen(out) + 4;
  char *out_path = malloc(out_path_len);
  CHECK_ALLOC(out_path, out_path_len);

  memcpy(out_path, out, out_path_len - 4);
  memcpy(&out_path[out_path_len - 4], ".ts\x0", 4);

  STACK_INFO("writing original time series to %s", out_path);

  FILE *ts_out = fopen(out_path, "wb");

  if (ts_out == NULL)
  {
    STACK_ERROR("unable to open %s for writing", out_path);
    free(out_path);
    out_path = NULL;
    return 0;
  }

  /* write an endiness bit */
  uint8_t endiness_bit = 1;
  fwrite(&endiness_bit, sizeof(uint8_t), 1, ts_out);

  /* write number of bits */
  fwrite(&len, sizeof(size_t), 1, ts_out);

  /* write data */
  size_t bytes_written = 0;
  clock_t start = clock();
  for (size_t i = 0; i < len; ++i)
  {
    fwrite(&ts[i], sizeof(double), 1, ts_out);
    bytes_written += sizeof(double);
  }
  clock_t end = clock();

  STACK_INFO("wrote %.3fmb at %.3f/mbs",
      (double) bytes_written * 0.0000009536743164062,
      ((double) bytes_written * 0.0000009536743164062) /
      ((double) (end - start) / (double) CLOCKS_PER_SEC));

  fclose(ts_out);
  free(out_path);
  out_path = NULL;
  return 1;
}

int
selfsimilarity_genmatrix(size_t len, double *ts, const char *out)
{
  STACK_INFO("recieved a timeseries of length %lu", len);

  if (!_write_ts(len, ts, out))
  {
    STACK_TRACE();
    return 0;
  }

  STACK_INFO("generating matrix with feature sizes starting at %lu and "
             "ending at 2", len / 2, 2);

  for (size_t i = len / 2; i >= 2; --i)
  {
    features_t feature_set;
    feature_set.feature_size = i;
    feature_set.features = NULL;
    feature_set.num_features = (len - i) + 1;

    clock_t start = clock();
    if (!selfsimilarity_genfeatures(ts, &feature_set))
    {
      STACK_TRACE();
      return 0;
    }
    clock_t end = clock();
    STACK_INFO("generated feature set matrix %lu x %lu in %.3f seconds",
        feature_set.num_features, feature_set.feature_size,
        ( (double) (end - start) / CLOCKS_PER_SEC));

    double **features = feature_set.features;
    for (size_t rows = 0; rows < feature_set.num_features; ++rows)
    {
      free(features[rows]);
    }
    free(features);

  }

  return 1;
}
