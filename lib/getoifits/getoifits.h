#ifndef GETOIFITS_H
#define GETOIFITS_H
#endif

/* Header for oifits routines.
 * This package uses the Oifits Exchange routines by John Young to view,
 * select and extract oi data.
 *
 */
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <complex>
#include <string>

extern "C"{
	#include "exchange.h"
	#include "fitsio.h"
}

using namespace std;

#define maxins 50
#define billion (1.0e9)
#define PARAM_NUMBER 8

typedef struct _usersel
{
  char file[FLEN_FILENAME];
  int target_id;
  char target[FLEN_VALUE];
  long numvis2;
  long numt3;
  float wavel;
  char insname[maxins][FLEN_VALUE];
  long numins;
  float minband;
  float maxband;
  int ntelescopes;
}oi_usersel;

typedef struct _uvpnt
{
  short sign;
  long uvpnt;
}oi_uvpnt;

typedef struct _bsref
{
  /* Structure for bisp to uv coord table referencing.
   * Negative uv table number means the particular baseline is conjugated.
   */
  oi_uvpnt ab;
  oi_uvpnt bc;
  oi_uvpnt ca;
}oi_bsref;

typedef struct _uv
{
  float u;
  float v;
  float bandwidth;
  float wavelength;
}oi_uv;

typedef struct _data
{
  float *pow;
  float *powerr;
  float *bisamp;
  float *bisphs;
  float *bisamperr;
  float *bisphserr;
  float *time;
  oi_uv *uv;
  oi_bsref *bsref;
  int npow;
  int nbis;
  int nuv;
}oi_data;

typedef struct _dataonly
{
  float *pow;
  complex<float> * t3;
  int nbis;
  int npow;
}data_only;

/* Constants */
#define EXPON1  2.71828182845904523
#define PI      3.14159265358979323
#define RPMAS (3.14159265358979323/180.0)/3600000.0
#define NMOD 20

/* Function declarations */
int get_oi_fits_selection(oi_usersel *usersel, int* status);
int get_oi_fits_data(oi_usersel* usersel, oi_data *data, int* status);
int compare_uv(oi_uv uv, oi_uv withuv, float thresh);
void free_oi_target(oi_target *targets);
void free_oi_wavelength(oi_wavelength *wave);
void free_oi_vis2(oi_vis2 *vis2);
void free_oi_t3(oi_t3 *t3);
void free_oi_data(oi_data *data);
int count_redundant_bsuv(oi_bsref *bsref, int nbs);
float bsuv_coverage_quality(oi_bsref *bsref, int nbs, oi_uv *uv, int nuv);
void write_fits_image( float* image , int* status);
