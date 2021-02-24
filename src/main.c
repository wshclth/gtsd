/* standardized console output as well as pointer checking */
#include <info/info.h>
#include <info/ptrcheck.h>

/* exposes the generator function enum */
#include <tsgen/tsgen.h>

/* basic cstring functions */
#include <string.h>

/* memory allocators and misc functions */
#include <stdlib.h>

/* creates the bool keyword */
#include <stdbool.h>

/* expose the time library */
#include <time.h>

/* exposes the selfsimilarity perform action */
#include <selfsimilarity/selfsimilarity.h>

typedef struct
{
  /* the location to load a file from */
  const char *load_location;

  /* if true will generate the specified time series type */
  bool timeseries_generate;

  /* if true will load the timeseries from a file */
  bool timeseries_load;

  /* an enum describing what generator was specified */
  enum GENERATOR_FUNCTION generator;

  /* during generation specify the number of data points */
  size_t data_points;

  /* a place to put the self similarity matrix */
  const char *out_file;

  /* the width of the feature that is required */
  size_t feature_width;
} cli_args;


/*
 * Parses the command line arguments into a cli_args struct to be used through-
 * out the program and define the overal goal of the execution instance.
 *
 */
static int
_parse_cli(int argc, char **argv, cli_args **_args)
{
  /* allocate and verify successfull allocation */
  *_args = malloc(sizeof(cli_args));

  CHECK_ALLOC(*_args, sizeof(cli_args));

  /* set the cli_args to their default value */
  (*_args)->load_location = NULL;
  (*_args)->timeseries_generate = false;
  (*_args)->timeseries_load = false;
  (*_args)->data_points = 0;
  (*_args)->out_file = NULL;
  (*_args)->feature_width = 0;

  for (int argidx = 1; argidx < argc; argidx++)
  {
    if (strcmp(argv[argidx], "-w") == 0)
    {
      STACK_INFO("%s", "found feature width tag");
      if (argidx + 1 < argc)
      {
        argidx += 1;
        (*_args)->feature_width = strtoul(argv[argidx], NULL, 10);
        STACK_INFO("set feature width to %lu", (*_args)->feature_width);
        continue;
      }
      else
      {
        STACK_ERROR("%s", "width tag found but no value");
        return 1;
      }
    }
    /* -s for specifying source type */
    else if (strcmp(argv[argidx], "-s") == 0)
    {
      STACK_INFO("%s", "found source type tag");
      if (argidx + 1 < argc)
      {
        argidx += 1;
        (*_args)->timeseries_generate = true;
        if (strcmp(argv[argidx], "randomwalk") == 0)
        {
          STACK_INFO("%s", "selected RANDOM_WALK as generator");
          (*_args)->generator = RANDOM_WALK;
          continue;
        }
        else if (strcmp(argv[argidx], "file") == 0)
        {
          STACK_INFO("%s", "selected tsgen_loadfile as generator");
          (*_args)->generator = LOAD_FILE;

          if (argidx + 1 < argc)
          {
            argidx += 1;
            (*_args)->load_location = argv[argidx];
            STACK_INFO("set load_location to %s", argv[argidx]);
            (*_args)->timeseries_generate = false;
            (*_args)->timeseries_load = true;
            continue;
          }
          else
          {
            STACK_ERROR("%s", "please specify a file location with file source");
            return 0;
          }
        }
        else
        {
          STACK_ERROR("found -s but unknown generator %s",
              argv[argidx]);
          return 0;
        }
      }
      else
      {
        STACK_ERROR("%s", "found -s but no source type");
        free(*_args);
        *_args = NULL;
        return 0;
      }
    }
    /* sepcify the number of data points */
    else if (strcmp(argv[argidx], "-dp") == 0)
    {
      if (argidx + 1 < argc)
      {
        argidx += 1;
        (*_args)->data_points = strtoul(argv[argidx], NULL, 10);
      }
      else
      {
        STACK_ERROR("%s", "found -dp but no data point length was given");
        free(*_args);
        *_args = NULL;
        return 0;
      }
    }
    /* specify the output */
    else if (strcmp(argv[argidx], "-out") == 0)
    {
      STACK_INFO("%s", "found output file tag");
      if (argidx + 1 < argc)
      {
        argidx += 1;
        STACK_INFO("setting output file to %s", argv[argidx]);
        (*_args)->out_file = argv[argidx];
      }
      else
      {
        STACK_INFO("%s", "found -out but no output file");
        free(*_args);
        *_args = NULL;
        return 0;
      }
    }
  }
  return 1;
}

int
main(int argc, char **argv)
{

  time_t seed = time(NULL);
  srand((unsigned int) seed);
  STACK_INFO("initalized random seed to %lu", seed);

  cli_args *args = NULL;
  if (!_parse_cli(argc, argv, &args))
  {
    STACK_TRACE();
    return 1;
  }

  if (args->out_file == NULL)
  {
    STACK_ERROR("%s", "-out is required");
    return 1;
  }

  if (!args->timeseries_generate && !args->timeseries_load)
  {
    STACK_ERROR("%s", "no action specified");
    return 1;
  }

  if (args->feature_width == 0)
  {
    STACK_ERROR("%s", "feature width has not been specified");
    return 1;
  }

  double *data = NULL;
  if (args->timeseries_generate)
  {
    STACK_INFO("generating with specified generator %lu points",
        (args->data_points));

    if (args->data_points == 0)
    {
      STACK_ERROR("%s\n", "-dp was not specified with -s");
      return 1;
    }
  }

  /* call the generator that was specified */
  switch (args->generator)
  {
    case RANDOM_WALK:
      {
        if (!financial_randomwalk(args->data_points, 10, &data))
        {
          STACK_TRACE();
          return 1;
        }
        break;
      }
    case LOAD_FILE:
      {
        if (!tsgen_loadfile(args->load_location, &data, &(args->data_points)))
        {
          STACK_TRACE();
          return 1;
        }
        break;
      }
  }


  /* generate the self similarity matrix */
  if (!selfsimilarity_genmatrix_gpu(args->data_points, data, args->out_file,
        args->feature_width))
  {
    STACK_TRACE();
    return 1;
  }

  return 0;
}
