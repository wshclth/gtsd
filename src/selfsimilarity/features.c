#include <selfsimilarity/features.h>

static
int _vector_normalize(double *feature, size_t len)
{
  double inv_norm = 0;
  for (size_t i = 0; i < len; ++i)
  {
    inv_norm += (feature[i] * feature[i]);
  }
  inv_norm = 1.0 / sqrt(inv_norm);

  double norm = 0;
  for (size_t i = 0; i < len; ++i)
  {
    feature[i] *= inv_norm;
    norm += (feature[i] * feature[i]);
  }

  norm = sqrt(norm);

  if (!(0.9999999999999808 < norm &&
      norm < 1.0000000000000180))
  {
    STACK_ERROR("norm of feature = %.16lu, which is not withing acceptable "
                "tolerance", norm);
    return 0;
  }

  return 1;
}

int
selfsimilarity_genfeatures(const double *ts, features_t *_ret)
{

  if (_ret == NULL)
  {
    STACK_ERROR("%s", "_ret was NULL");
    return 0;
  }

  if (ts == NULL)
  {
    STACK_ERROR("%s", "the given time series can not be NULL");
    return 0;
  }

  if (_ret->feature_size == 0)
  {
    STACK_ERROR("%s", "feature_size can not be 0");
    return 0;
  }

  if (_ret->num_features == 0)
  {
    STACK_ERROR("%s", "num_features can not be 0");
    return 0;
  }

  const size_t num_features = _ret->num_features;
  const size_t feature_size = _ret->feature_size;

  _ret->features = malloc(sizeof(double*) * _ret->num_features);
  CHECK_ALLOC((_ret->features), (sizeof(double*) * _ret->num_features));

  for (size_t rows = 0; rows < num_features; ++rows)
  {
    _ret->features[rows] = malloc(sizeof(double) * feature_size);
    CHECK_ALLOC(_ret->features[rows], sizeof(double) * feature_size);

    memcpy(_ret->features[rows], &ts[rows], sizeof(double) * feature_size);
    if (!_vector_normalize(_ret->features[rows], feature_size))
    {
      STACK_ERROR("unable to normalize feature id=%lu", rows);
      return 0;
    }
  }

  return 1;
}
