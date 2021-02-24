#include <CL/cl.h>
#include <selfsimilarity/selfsimilarity.h>

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

  for (size_t colidx = 0; colidx < kernel_data->row_index + 1; ++colidx)
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
  size_t num_frames_digits = strlen(".frame") + (size_t) num_frames + 100;

  size_t out_path_len = strlen(out) + num_frames_digits;
  char *out_path = malloc(out_path_len);
  CHECK_ALLOC(out_path, out_path_len);
  memcpy(out_path, out, out_path_len - num_frames_digits);

  for (size_t i = len - 1024; i >= 2; --i)
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
    fwrite(&feature_set.num_features, sizeof(size_t), 1, frame);

    double total_iterations_time = 0;
    double num_iterations = 0;

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

        if (!_flush_row(frame, result->result, result->row_index + 1))
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
    }
    printf("\n");

    /* free the features since we wrote out the matrix to disk */
    double **features = feature_set.features;
    for (size_t rows = 0; rows < feature_set.num_features; ++rows)
    {
      free(features[rows]);
    }
    free(features);

    fclose(frame);

  }

  return 1;
}

static
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename) {

   cl_program program;
   FILE *program_handle;
   char *program_buffer, *program_log;
   size_t program_size, log_size;
   int err;

   /* Read program file and place content into buffer */
   program_handle = fopen(filename, "r");
   if(program_handle == NULL) {
      perror("Couldn't find the program file");
      exit(1);
   }
   fseek(program_handle, 0, SEEK_END);
   program_size = (size_t) ftell(program_handle);
   rewind(program_handle);
   program_buffer = (char*)malloc(program_size + 1);
   program_buffer[program_size] = '\0';
   fread(program_buffer, sizeof(char), program_size, program_handle);
   fclose(program_handle);

   /* Create program from file
   Creates a program from the source code in the add_numbers.cl file.
   Specifically, the code reads the file's content into a char array
   called program_buffer, and then calls clCreateProgramWithSource.
   */
   const char *pb = program_buffer;
   program = clCreateProgramWithSource(ctx, 1,
      &pb, &program_size, &err);
   if(err < 0) {
      perror("Couldn't create the program");
      exit(1);
   }
   free(program_buffer);

   /* Build program
   The fourth parameter accepts options that configure the compilation.
   These are similar to the flags used by gcc. For example, you can
   define a macro with the option -DMACRO=VALUE and turn off optimization
   with -cl-opt-disable.
   */
   err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
   if(err < 0) {

      /* Find size of log and print to std output */
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
            0, NULL, &log_size);
      program_log = (char*) malloc(log_size + 1);
      program_log[log_size] = '\0';
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
            log_size + 1, program_log, NULL);
      printf("%s\n", program_log);
      free(program_log);
      exit(1);
   }

   return program;
}

int
selfsimilarity_genmatrix_gpu(size_t len, double *ts, const char *out,
    size_t feature_width)
{

  (void) out;

  size_t *time_series_len_src = malloc(sizeof(size_t));
  *time_series_len_src = len;

  size_t *feature_width_src = malloc(sizeof(size_t));
  CHECK_ALLOC(feature_width_src, sizeof(size_t));
  *feature_width_src = feature_width;

  size_t *diag_src = malloc(sizeof(double) * len);
  CHECK_ALLOC(diag_src, sizeof(double) * len);

  double *result_src = malloc(sizeof(double) * len);
  CHECK_ALLOC(result_src, sizeof(double));

  size_t *diag_index_src = malloc(sizeof(size_t));
  *diag_index_src = 0;
  CHECK_ALLOC(diag_index_src, sizeof(size_t));

  cl_platform_id platforms[2];
  if (clGetPlatformIDs(2, platforms, NULL) !=
      CL_SUCCESS)
  {
    STACK_ERROR("%s", "unable to find a valid platform");
    return 0;
  }

  cl_platform_id platform = platforms[1];

  STACK_INFO("%s", "found an opencl platform, looking for a GPU");

  cl_device_id device;
  if (clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 2, &device, NULL) !=
      CL_SUCCESS)
  {
    STACK_ERROR("%s", "unable to find a valid OpenCL compatible system on this "
        "computer");
    return 0;
  }

  STACK_INFO("%s", "found a gpu");

  cl_int ret;
  cl_context context;
  context = clCreateContext(NULL, 1, &device, NULL, NULL, &ret);

  if (ret != CL_SUCCESS)
  {
    STACK_ERROR("%s", "unable to accept context");
    return 0;
  }

  STACK_INFO("%s", "successfully made connection with gpu");

  cl_program program = build_program(context, device, "cl/recurrence.cl");

  (void) program;

  STACK_INFO("%s", "successfully built cl/recurrence.cl");

  cl_command_queue queue = clCreateCommandQueueWithProperties(context,
      device, NULL, &ret);

  (void) queue;


  if (ret != CL_SUCCESS)
  {
    STACK_ERROR("%s", "unable to create queue");
    return 0;
  }

  STACK_INFO("%s", "successfully made command queue");

  cl_mem time_series_dev = clCreateBuffer(context,
      CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
      sizeof(double) * len, ts, &ret);

  cl_mem time_series_len_dev = clCreateBuffer(context,
      CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
      sizeof(size_t), time_series_len_src, &ret);

  cl_mem feature_width_dev = clCreateBuffer(context,
      CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
      sizeof(size_t), feature_width_src, &ret);

  cl_mem diag_index_dev = clCreateBuffer(context,
      CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
      sizeof(size_t), diag_index_src, &ret);

  cl_mem result_dev = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
      sizeof(double) * len, NULL, NULL);


  cl_kernel kernel = clCreateKernel(program, "recurrence_point", &ret);
  if (ret != CL_SUCCESS)
  {
    STACK_ERROR("%s", "unable to create kernel recurrence_point");
    return 0;
  }

  STACK_INFO("%s", "created kernel recurrence_point");

  ret  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &time_series_dev);
  ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &time_series_len_dev);
  ret |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &feature_width_dev);
  ret |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &diag_index_dev);
  ret |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &result_dev);

  if (ret != CL_SUCCESS)
  {
    STACK_ERROR("%s", "unable to set kernel arguments");
    return 0;
  }

  STACK_INFO("%s", "set kernel arguments successfully");

  int64_t num_features = (*time_series_len_src) - (*feature_width_src) + 1;

  FILE *fp = fopen(out, "wb");
  if (fp == NULL)
  {
    STACK_ERROR("unable to create file %s", out);
    return 0;
  }

  /* write an endiness bit */
  uint8_t endiness_bit = 1;
  fwrite(&endiness_bit, sizeof(uint8_t), 1, fp);

  fwrite(&num_features, sizeof(uint64_t), 1, fp);

  for (int64_t i = 0; i < num_features; ++i)
  {
    *diag_index_src = (uint64_t) i;
    ret = clEnqueueWriteBuffer(queue, diag_index_dev, CL_TRUE, 0, sizeof(size_t),
        diag_index_src, 0, NULL, NULL);

    size_t work_size = num_features - i;
    ret = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &work_size,NULL, 0, NULL, NULL);

    clFinish(queue);
    clEnqueueReadBuffer(queue, result_dev, CL_TRUE, 0, sizeof(double) * work_size, result_src, 0, NULL, NULL);

    _flush_row(fp, result_src, work_size);

    fflush(fp);
    STACK_INFO("[%05.3f%] %lu / %lu", (double)i / (double)num_features, i, num_features);
  }

  _write_ts(len, ts, out);

  return 1;
}
