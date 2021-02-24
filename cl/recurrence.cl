#define v_row (&(time_series[row_index]))
#define v_col (&(time_series[col_index]))

__kernel
void recurrence_point(__global const double *time_series,
                      __global const size_t *time_series_len,
                      __global const size_t *feature_width,
                      __global const size_t *diag_index,
                      __global double *result)
{
  const int id = get_global_id(0);

  int num_features = (*time_series_len - *feature_width) + 1;

  int row_index = id + (*diag_index);
  int col_index = id - (*diag_index);

  if (row_index > num_features || col_index < 0)
  {
    return;
  }

  double rnorm = 0;
  double cnorm = 0;
  double dotrc = 0;

  for (size_t i = 0; i < *feature_width; ++i)
  {
    rnorm += (v_row[i] * v_row[i]);
    cnorm += (v_col[i] * v_col[i]);
    dotrc += (v_row[i] * v_col[i]);
  }

  rnorm = sqrt(rnorm);
  cnorm = sqrt(cnorm);

  result[id - (*diag_index)] = acos(dotrc / (rnorm * cnorm));
}
