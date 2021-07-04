#ifndef INCLUDED_GOLAY_RUDIN_SHAPIRO_H
#define INCLUDED_GOLAY_RUDIN_SHAPIRO_H

// Return the n-th integer of the Golay-Rudin-Shapiro sequence
// see https://oeis.org/A020985
inline int golay_rudin_shapiro(int n) {
  if (n == 0)
    return 1;
  if (n % 2 == 0)
    return golay_rudin_shapiro(n / 2);
  return golay_rudin_shapiro((n - 1) / 2) * (1 - 2 * (n & 1));
}

#define A020985(n) golay_rudin_shapiro((n))
#define A020987(n) ((1 - A020985((n))) / 2)

#endif // INCLUDED_GOLAY_RUDIN_SHAPIRO_H
