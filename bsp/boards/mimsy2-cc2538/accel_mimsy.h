#ifndef __XL_MIMSY_H__
#define __XL_MIMSY_H__

#include "flash_mimsy.h"
extern void mimsyIMURead6Dof(IMUData *data);
unsigned short alt_inv_orientation_matrix_to_scalar(const signed char *mtx);
void alt_inv_q_rotate(const long *q, const long *in, long *out);
void alt_inv_quaternion_to_rotation(const long *quat, long *rot);
void alt_mlMatrixVectorMult(long matrix[9], const long vecIn[3], long *vecOut);
void alt_inv_q_normalizef(float *q);
void mimsyIMUInit(void);
void mimsyIMURead6DofInv(IMUData *data);
void mimsyDmpBegin(void);
void mimsySetAccelFsr(int fsr);
void mimsySetGyroFsr(int fsr);

#endif
