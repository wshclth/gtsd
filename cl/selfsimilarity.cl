__kernel void
selfsimilarity(__global const int *didx,
               __global const double *ts,
               __global const int *frame_size,
               __global const int *diag_len,
               __global double *diag)
{
  int i = get_global_id(0);

  int row_idx = i + (*didx);
  int col_idx = i - (*didx);

  if (col_idx < 0 || row_idx > (*diag_len))
  {
    return;
  }

  int fs = *frame_size;

  double *vec1 = &ts[row_idx];
  double *vec2 = &ts[col_idx];

  double norm_vec1 = 0;
  double norm_vec2 = 0;
  double dot_vec12 = 0;

  for (int i = 0; i < fs; ++i)
  {
    norm_vec1 += (vec1[i]*vec1[i]);
    norm_vec2 += (vec2[i]*vec2[i]);
    dot_vec12 += (vec1[i]*vec2[i]);
  }

  norm_vec1 = sqrt(norm_vec1);
  norm_vec2 = sqrt(norm_vec2);

  diag[i] = dot_vec12 / (norm_vec1 * norm_vec2);
}
