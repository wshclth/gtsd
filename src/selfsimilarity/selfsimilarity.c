#include <math.h>
#include <pthread.h>
#include <selfsimilarity/selfsimilarity.h>
#include <time.h>

typedef struct
{
  double **features;
  size_t feature_size;
  size_t row_index;
  double *result;
} kernel_args_t;

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

static
int _flush_row(FILE *fp, double *data, size_t len)
{
  /* write data */
  for (size_t i = 0; i < len; ++i)
  {
    fwrite(&data[i], sizeof(double), 1, fp);
  }
  return 1;
}

static
void *_row_kernel(void *args)
{
  kernel_args_t *kernel_data = (kernel_args_t*) args;

  /* allocate enough data to store the lower half of the symetric matrix */
  kernel_data->result = calloc(sizeof(double), kernel_data->row_index + 1);

  const double *row_feature = kernel_data->features[kernel_data->row_index];

  for (size_t colidx = 0; colidx < kernel_data->row_index; ++colidx)
  {
    const double *col_feature = kernel_data->features[colidx];

    double dot_product = 0;
    for (size_t i = 0; i < kernel_data->feature_size; ++i)
    {
      dot_product += (row_feature[i] * col_feature[i]);
    }
    kernel_data->result[colidx] = acos(dot_product);
  }

  pthread_exit(args);
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

  size_t processors = (size_t) sysconf(_SC_NPROCESSORS_ONLN) - 1;
  STACK_INFO("generating %lu threads for concurrency", processors);

  pthread_t *threads = calloc(processors, sizeof(pthread_t));

  STACK_INFO("generating matrix with feature sizes starting at %lu and "
             "ending at 2", len / 2, 2);


  double num_frames = log10((double) (UINT64_MAX - 1));
  size_t num_frames_digits = strlen(".frame") + (size_t) num_frames;

  size_t out_path_len = strlen(out) + num_frames_digits;
  char *out_path = malloc(out_path_len);
  CHECK_ALLOC(out_path, out_path_len);
  memcpy(out_path, out, out_path_len - num_frames_digits);

  for (size_t i = len / 2; i >= 2; --i)
  {

    /* generate the features */
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

    STACK_INFO("[%5lu / %-5lu] "
        "generated feature set matrix %lu x %lu in %.3f seconds",
        ((len / 2) - i), (len / 2 - 2),
        feature_set.num_features, feature_set.feature_size,
        ( (double) (end - start) / CLOCKS_PER_SEC));

    if (sprintf(&out_path[out_path_len - num_frames_digits], ".frame%lu",
          (len / 2) - i) >= (int) num_frames_digits)
    {
      STACK_ERROR("%s", "unable to create output string buffer overflow");
      return 0;
    }

    STACK_INFO("generating frame %s", out_path);

    /* create a file */
    FILE *frame = fopen(out_path, "wb");

    if (frame == NULL)
    {
      STACK_ERROR("unable to create file %s", out_path);
      return 0;
    }

    /* write an endiness bit */
    uint8_t endiness_bit = 1;
    fwrite(&endiness_bit, sizeof(uint8_t), 1, frame);

    double total_iterations_time = 0;
    double num_iterations = 0;
    double iterations_per_second = 0;

    /* split the calculation into blocks of number of processors */
    for (size_t sr = 0; sr < feature_set.num_features; sr += processors)
    {
      STACK_INFO("[%5lu / %-5lu] processing rows %lu - %lu / %lu... elapsed %08.3f sec\r\033[F",
        ((len / 2) - i), (len / 2 - 2), sr, sr + processors, feature_set.num_features,
        total_iterations_time);
      start = clock();

      size_t spawned_threads = 0;
      for (size_t row = sr; row < sr + processors && row < feature_set.num_features; ++row)
      {
        /*
         * create the indexing information that uniquely identifies the kernel
         */
        kernel_args_t *idxparams = malloc(sizeof(kernel_args_t));
        CHECK_ALLOC(idxparams, sizeof(kernel_args_t));
        idxparams->features = feature_set.features;
        idxparams->feature_size = feature_set.feature_size;
        idxparams->row_index = row;
        pthread_create(&threads[(row-sr)], NULL, _row_kernel, idxparams);
        spawned_threads += 1;
      }

      for (size_t tjoin = 0; tjoin < spawned_threads; ++tjoin)
      {
        kernel_args_t *result = NULL;
        pthread_join(threads[tjoin], (void*)&result);

        if (result == NULL)
        {
          STACK_ERROR("%s", "thread returned a NULL result");
          return 0;
        }

        if (result->result == NULL)
        {
          STACK_ERROR("%s", "thread did not return a result");
          return 0;
        }

        if (!_flush_row(frame, result->result, result->row_index))
        {
          STACK_ERROR("error writing row %lu to disk", result->row_index);
          return 0;
        }

        free(result->result);
        free(result);
      }
      end = clock();
      num_iterations += (double) spawned_threads;
      total_iterations_time += (double) (end-start) / (double) CLOCKS_PER_SEC;
      iterations_per_second = (num_iterations / total_iterations_time);
    }
    printf("\n");

    /* free the features since we wrote out the matrix to disk */
    double **features = feature_set.features;
    for (size_t rows = 0; rows < feature_set.num_features; ++rows)
    {
      free(features[rows]);
    }
    free(features);
  }

  return 1;
}
