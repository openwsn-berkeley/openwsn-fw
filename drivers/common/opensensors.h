#ifndef __OPENSENSORS_H__
#define __OPENSENSORS_H__

#define MAXSENSORS           10

typedef uint16_t (*callbackRead_cbt)(void);

typedef float (*callbackConvert_cbt)(uint16_t value);

typedef struct {
   uint8_t                   path1len;
   uint8_t*                  path1val;
   callbackRead_cbt          callbackRead;
   callbackConvert_cbt       callbackConvert;
} opensensors_resource_desc_t;

typedef struct {
   opensensors_resource_desc_t   opensensors_resource[MAXSENSORS];
   uint8_t                   numSensors;
} opensensors_vars_t;

void opensensors_init(void);
void opensensors_register(uint8_t path1len,
   uint8_t* path1val,
   callbackRead_cbt callbackRead,
   callbackConvert_cbt callbackConvert
);
opensensors_vars_t* opensensors_read(void);

#endif // __OPENSENSORS_H__
