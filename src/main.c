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

typedef struct
{
  /* always the first argument in argv */
  const char *load_location;

  /* if true will generate the specified time series type */
  bool timeseries_generate;

  /* if true will load the timeseries from a file */
  bool timeseries_load;

  /* an enum describing what generator was specified */
  enum GENERATOR_FUNCTION generator;

  /* during generation specify the number of data points */
  size_t data_points;

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
  (*_args)->load_location = argv[0];
  (*_args)->timeseries_generate = false;
  (*_args)->timeseries_load = false;
  (*_args)->data_points = 0;

  for (int argidx = 1; argidx < argc; argidx++)
  {
    if (strcmp(argv[argidx], "-s") == 0)
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
        else
        {
          STACK_ERROR("found -source but unknown generator %s",
              argv[argidx]);
          return 0;
        }
      }
      else
      {
        STACK_ERROR("%s", "found -source but no source type");
        free(*_args);
        *_args = NULL;
        return 0;
      }
    }
    if  (strcmp(argv[argidx], "-dp") == 0)
    {
      if (argidx + 1 < argc)
      {
        argidx += 1;
        (*_args)->data_points = strtol(argv[argidx], NULL, 10);
      }
      else
      {
        STACK_ERROR("%s", "found -dp but no data point length was given");
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
  srand(seed);
  STACK_INFO("initalized random seed to %lu", seed);

  cli_args *args = NULL;
  if (!_parse_cli(argc, argv, &args))
  {
    STACK_TRACE();
    return 1;
  }

  double *data = NULL;
  if (args->timeseries_load)
  {
    STACK_ERROR("%s", "loading from file is not supported yet");
    return 1;
  }

  if (args->timeseries_generate)
  {
    STACK_INFO("generating with specified generator %lu points",
        (args->data_points));

    if (args->data_points == 0)
    {
      STACK_ERROR("%s\n", "-dp was not specified with -source");
      return 1;
    }

    switch (args->generator)
    {
      case RANDOM_WALK:
        {
          if (!financial_randomwalk(args->data_points, 0, &data))
          {
            STACK_TRACE();
            return 1;
          }
        }
    }
  }

  return 0;
}
