/// autogenerated analytical inverse kinematics code from ikfast program part of OpenRAVE
/// \author Rosen Diankov
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///     http://www.apache.org/licenses/LICENSE-2.0
/// 
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// ikfast version 0x10000049 generated on 2022-11-11 01:37:06.473914
/// To compile with gcc:
///     gcc -lstdc++ ik.cpp
/// To compile without any main function as a shared object (might need -llapack):
///     gcc -fPIC -lstdc++ -DIKFAST_NO_MAIN -DIKFAST_CLIBRARY -shared -Wl,-soname,libik.so -o libik.so ik.cpp
#define IKFAST_HAS_LIBRARY
#include "ikfast.h" // found inside share/openrave-X.Y/python/ikfast.h
using namespace ikfast;

// check if the included ikfast version matches what this file was compiled with
#define IKFAST_COMPILE_ASSERT(x) extern int __dummy[(int)x]
IKFAST_COMPILE_ASSERT(IKFAST_VERSION==0x10000049);

#include <cmath>
#include <vector>
#include <limits>
#include <algorithm>
#include <complex>

#ifndef IKFAST_ASSERT
#include <stdexcept>
#include <sstream>
#include <iostream>

#ifdef _MSC_VER
#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCDNAME__
#endif
#endif

#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __func__
#endif

#define IKFAST_ASSERT(b) { if( !(b) ) { std::stringstream ss; ss << "ikfast exception: " << __FILE__ << ":" << __LINE__ << ": " <<__PRETTY_FUNCTION__ << ": Assertion '" << #b << "' failed"; throw std::runtime_error(ss.str()); } }

#endif

#if defined(_MSC_VER)
#define IKFAST_ALIGNED16(x) __declspec(align(16)) x
#else
#define IKFAST_ALIGNED16(x) x __attribute((aligned(16)))
#endif

#define IK2PI  ((IkReal)6.28318530717959)
#define IKPI  ((IkReal)3.14159265358979)
#define IKPI_2  ((IkReal)1.57079632679490)

#ifdef _MSC_VER
#ifndef isnan
#define isnan _isnan
#endif
#ifndef isinf
#define isinf _isinf
#endif
//#ifndef isfinite
//#define isfinite _isfinite
//#endif
#endif // _MSC_VER

// lapack routines
extern "C" {
  void dgetrf_ (const int* m, const int* n, double* a, const int* lda, int* ipiv, int* info);
  void zgetrf_ (const int* m, const int* n, std::complex<double>* a, const int* lda, int* ipiv, int* info);
  void dgetri_(const int* n, const double* a, const int* lda, int* ipiv, double* work, const int* lwork, int* info);
  void dgesv_ (const int* n, const int* nrhs, double* a, const int* lda, int* ipiv, double* b, const int* ldb, int* info);
  void dgetrs_(const char *trans, const int *n, const int *nrhs, double *a, const int *lda, int *ipiv, double *b, const int *ldb, int *info);
  void dgeev_(const char *jobvl, const char *jobvr, const int *n, double *a, const int *lda, double *wr, double *wi,double *vl, const int *ldvl, double *vr, const int *ldvr, double *work, const int *lwork, int *info);
}

using namespace std; // necessary to get std math routines

#ifdef IKFAST_NAMESPACE
namespace IKFAST_NAMESPACE {
#endif

inline float IKabs(float f) { return fabsf(f); }
inline double IKabs(double f) { return fabs(f); }

inline float IKsqr(float f) { return f*f; }
inline double IKsqr(double f) { return f*f; }

inline float IKlog(float f) { return logf(f); }
inline double IKlog(double f) { return log(f); }

// allows asin and acos to exceed 1. has to be smaller than thresholds used for branch conds and evaluation
#ifndef IKFAST_SINCOS_THRESH
#define IKFAST_SINCOS_THRESH ((IkReal)1e-7)
#endif

// used to check input to atan2 for degenerate cases. has to be smaller than thresholds used for branch conds and evaluation
#ifndef IKFAST_ATAN2_MAGTHRESH
#define IKFAST_ATAN2_MAGTHRESH ((IkReal)1e-7)
#endif

// minimum distance of separate solutions
#ifndef IKFAST_SOLUTION_THRESH
#define IKFAST_SOLUTION_THRESH ((IkReal)1e-6)
#endif

// there are checkpoints in ikfast that are evaluated to make sure they are 0. This threshold speicfies by how much they can deviate
#ifndef IKFAST_EVALCOND_THRESH
#define IKFAST_EVALCOND_THRESH ((IkReal)0.00001)
#endif


inline float IKasin(float f)
{
IKFAST_ASSERT( f > -1-IKFAST_SINCOS_THRESH && f < 1+IKFAST_SINCOS_THRESH ); // any more error implies something is wrong with the solver
if( f <= -1 ) return float(-IKPI_2);
else if( f >= 1 ) return float(IKPI_2);
return asinf(f);
}
inline double IKasin(double f)
{
IKFAST_ASSERT( f > -1-IKFAST_SINCOS_THRESH && f < 1+IKFAST_SINCOS_THRESH ); // any more error implies something is wrong with the solver
if( f <= -1 ) return -IKPI_2;
else if( f >= 1 ) return IKPI_2;
return asin(f);
}

// return positive value in [0,y)
inline float IKfmod(float x, float y)
{
    while(x < 0) {
        x += y;
    }
    return fmodf(x,y);
}

// return positive value in [0,y)
inline double IKfmod(double x, double y)
{
    while(x < 0) {
        x += y;
    }
    return fmod(x,y);
}

inline float IKacos(float f)
{
IKFAST_ASSERT( f > -1-IKFAST_SINCOS_THRESH && f < 1+IKFAST_SINCOS_THRESH ); // any more error implies something is wrong with the solver
if( f <= -1 ) return float(IKPI);
else if( f >= 1 ) return float(0);
return acosf(f);
}
inline double IKacos(double f)
{
IKFAST_ASSERT( f > -1-IKFAST_SINCOS_THRESH && f < 1+IKFAST_SINCOS_THRESH ); // any more error implies something is wrong with the solver
if( f <= -1 ) return IKPI;
else if( f >= 1 ) return 0;
return acos(f);
}
inline float IKsin(float f) { return sinf(f); }
inline double IKsin(double f) { return sin(f); }
inline float IKcos(float f) { return cosf(f); }
inline double IKcos(double f) { return cos(f); }
inline float IKtan(float f) { return tanf(f); }
inline double IKtan(double f) { return tan(f); }
inline float IKsqrt(float f) { if( f <= 0.0f ) return 0.0f; return sqrtf(f); }
inline double IKsqrt(double f) { if( f <= 0.0 ) return 0.0; return sqrt(f); }
inline float IKatan2Simple(float fy, float fx) {
    return atan2f(fy,fx);
}
inline float IKatan2(float fy, float fx) {
    if( isnan(fy) ) {
        IKFAST_ASSERT(!isnan(fx)); // if both are nan, probably wrong value will be returned
        return float(IKPI_2);
    }
    else if( isnan(fx) ) {
        return 0;
    }
    return atan2f(fy,fx);
}
inline double IKatan2Simple(double fy, double fx) {
    return atan2(fy,fx);
}
inline double IKatan2(double fy, double fx) {
    if( isnan(fy) ) {
        IKFAST_ASSERT(!isnan(fx)); // if both are nan, probably wrong value will be returned
        return IKPI_2;
    }
    else if( isnan(fx) ) {
        return 0;
    }
    return atan2(fy,fx);
}

template <typename T>
struct CheckValue
{
    T value;
    bool valid;
};

template <typename T>
inline CheckValue<T> IKatan2WithCheck(T fy, T fx, T epsilon)
{
    CheckValue<T> ret;
    ret.valid = false;
    ret.value = 0;
    if( !isnan(fy) && !isnan(fx) ) {
        if( IKabs(fy) >= IKFAST_ATAN2_MAGTHRESH || IKabs(fx) > IKFAST_ATAN2_MAGTHRESH ) {
            ret.value = IKatan2Simple(fy,fx);
            ret.valid = true;
        }
    }
    return ret;
}

inline float IKsign(float f) {
    if( f > 0 ) {
        return float(1);
    }
    else if( f < 0 ) {
        return float(-1);
    }
    return 0;
}

inline double IKsign(double f) {
    if( f > 0 ) {
        return 1.0;
    }
    else if( f < 0 ) {
        return -1.0;
    }
    return 0;
}

template <typename T>
inline CheckValue<T> IKPowWithIntegerCheck(T f, int n)
{
    CheckValue<T> ret;
    ret.valid = true;
    if( n == 0 ) {
        ret.value = 1.0;
        return ret;
    }
    else if( n == 1 )
    {
        ret.value = f;
        return ret;
    }
    else if( n < 0 )
    {
        if( f == 0 )
        {
            ret.valid = false;
            ret.value = (T)1.0e30;
            return ret;
        }
        if( n == -1 ) {
            ret.value = T(1.0)/f;
            return ret;
        }
    }

    int num = n > 0 ? n : -n;
    if( num == 2 ) {
        ret.value = f*f;
    }
    else if( num == 3 ) {
        ret.value = f*f*f;
    }
    else {
        ret.value = 1.0;
        while(num>0) {
            if( num & 1 ) {
                ret.value *= f;
            }
            num >>= 1;
            f *= f;
        }
    }
    
    if( n < 0 ) {
        ret.value = T(1.0)/ret.value;
    }
    return ret;
}

/// solves the forward kinematics equations.
/// \param pfree is an array specifying the free joints of the chain.
IKFAST_API void ComputeFk(const IkReal* j, IkReal* eetrans, IkReal* eerot) {
for(int dummyiter = 0; dummyiter < 1; ++dummyiter) {
IkReal x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15;
x0=IKsin(j[0]);
x1=IKcos(j[1]);
x2=IKcos(j[2]);
x3=IKsin(j[1]);
x4=IKsin(j[2]);
x5=IKcos(j[0]);
x6=((4.48)*x2);
x7=((6.82)*x2);
x8=((4.48)*x4);
x9=((6.82)*x4);
x10=((1.0)*x2);
x11=((1.0)*x4);
x12=(x1*x5);
x13=(x3*x5);
x14=(x0*x3);
x15=(x0*x1);
eetrans[0]=((((6.75)*x15))+(((4.6)*x14))+((x14*x6))+(((0.375)*x5))+(((-1.0)*x14*x9))+(((-0.05)*x0))+((x15*x7))+((x15*x8)));
IkReal x16=((1.0)*x12);
eetrans[1]=((((-6.75)*x12))+(((0.05)*x5))+(((-4.6)*x13))+(((0.375)*x0))+(((-1.0)*x16*x8))+(((-1.0)*x16*x7))+((x13*x9))+(((-1.0)*x13*x6)));
IkReal x17=((1.0)*x3);
eetrans[2]=((2.3)+((x1*x6))+(((-6.75)*x3))+(((-1.0)*x1*x9))+(((-1.0)*x17*x8))+(((-1.0)*x17*x7))+(((4.6)*x1)));
if( ((((((((x11*x15))+((x10*x14))))*(IKcos(j[3]))))+(((((((-1.0)*x11*x14))+((x10*x15))))*(IKsin(j[3])))))) < -1-IKFAST_SINCOS_THRESH || ((((((((x11*x15))+((x10*x14))))*(IKcos(j[3]))))+(((((((-1.0)*x11*x14))+((x10*x15))))*(IKsin(j[3])))))) > 1+IKFAST_SINCOS_THRESH )
    continue;
eerot[0]=IKacos((((((((x11*x15))+((x10*x14))))*(IKcos(j[3]))))+(((((((-1.0)*x11*x14))+((x10*x15))))*(IKsin(j[3]))))));
return;
}
IKFAST_ASSERT(0);
}

IKFAST_API int GetNumFreeParameters() { return 0; }
IKFAST_API int* GetFreeParameters() { return NULL; }
IKFAST_API int GetNumJoints() { return 4; }

IKFAST_API int GetIkRealSize() { return sizeof(IkReal); }

IKFAST_API int GetIkType() { return 0x4400000b; }

class IKSolver {
public:
IkReal j0,cj0,sj0,htj0,j0mul,j1,cj1,sj1,htj1,j1mul,j2,cj2,sj2,htj2,j2mul,j3,cj3,sj3,htj3,j3mul,new_r00,r00,rxp0_0,new_r01,r01,rxp0_1,new_r02,r02,rxp0_2,new_px,px,npx,new_py,py,npy,new_pz,pz,npz,pp;
unsigned char _ij0[2], _nj0,_ij1[2], _nj1,_ij2[2], _nj2,_ij3[2], _nj3;

IkReal j100, cj100, sj100;
unsigned char _ij100[2], _nj100;
bool ComputeIk(const IkReal* eetrans, const IkReal* eerot, const IkReal* pfree, IkSolutionListBase<IkReal>& solutions) {
j0=numeric_limits<IkReal>::quiet_NaN(); _ij0[0] = -1; _ij0[1] = -1; _nj0 = -1; j1=numeric_limits<IkReal>::quiet_NaN(); _ij1[0] = -1; _ij1[1] = -1; _nj1 = -1; j2=numeric_limits<IkReal>::quiet_NaN(); _ij2[0] = -1; _ij2[1] = -1; _nj2 = -1; j3=numeric_limits<IkReal>::quiet_NaN(); _ij3[0] = -1; _ij3[1] = -1; _nj3 = -1; 
for(int dummyiter = 0; dummyiter < 1; ++dummyiter) {
    solutions.Clear();
px = eetrans[0]; py = eetrans[1]; pz = eetrans[2];

r00 = eerot[0];
px = eetrans[0]; py = eetrans[1]; pz = eetrans[2];
new_px=px;
new_py=py;
new_pz=pz;
new_r00=r00;
r00 = new_r00; px = new_px; py = new_py; pz = new_pz;

pp=((px*px)+(py*py)+(pz*pz));
{
IkReal j0eval[2];
j0eval[0]=((px*px)+(py*py));
j0eval[1]=((IKabs(px))+(IKabs(py)));
if( IKabs(j0eval[0]) < 0.0000010000000000  || IKabs(j0eval[1]) < 0.0000010000000000  )
{
continue; // no branches [j0, j1, j2, j3]

} else
{
{
IkReal j0array[2], cj0array[2], sj0array[2];
bool j0valid[2]={false};
_nj0 = 2;
CheckValue<IkReal> x20 = IKatan2WithCheck(IkReal(((-1.0)*px)),IkReal(((-1.0)*py)),IKFAST_ATAN2_MAGTHRESH);
if(!x20.valid){
continue;
}
IkReal x18=((1.0)*(x20.value));
if((((px*px)+(py*py))) < -0.00001)
continue;
CheckValue<IkReal> x21=IKPowWithIntegerCheck(IKabs(IKsqrt(((px*px)+(py*py)))),-1);
if(!x21.valid){
continue;
}
if( (((0.375)*(x21.value))) < -1-IKFAST_SINCOS_THRESH || (((0.375)*(x21.value))) > 1+IKFAST_SINCOS_THRESH )
    continue;
IkReal x19=IKasin(((0.375)*(x21.value)));
j0array[0]=((((-1.0)*x18))+(((-1.0)*x19)));
sj0array[0]=IKsin(j0array[0]);
cj0array[0]=IKcos(j0array[0]);
j0array[1]=((3.14159265358979)+(((-1.0)*x18))+x19);
sj0array[1]=IKsin(j0array[1]);
cj0array[1]=IKcos(j0array[1]);
if( j0array[0] > IKPI )
{
    j0array[0]-=IK2PI;
}
else if( j0array[0] < -IKPI )
{    j0array[0]+=IK2PI;
}
j0valid[0] = true;
if( j0array[1] > IKPI )
{
    j0array[1]-=IK2PI;
}
else if( j0array[1] < -IKPI )
{    j0array[1]+=IK2PI;
}
j0valid[1] = true;
for(int ij0 = 0; ij0 < 2; ++ij0)
{
if( !j0valid[ij0] )
{
    continue;
}
_ij0[0] = ij0; _ij0[1] = -1;
for(int iij0 = ij0+1; iij0 < 2; ++iij0)
{
if( j0valid[iij0] && IKabs(cj0array[ij0]-cj0array[iij0]) < IKFAST_SOLUTION_THRESH && IKabs(sj0array[ij0]-sj0array[iij0]) < IKFAST_SOLUTION_THRESH )
{
    j0valid[iij0]=false; _ij0[1] = iij0; break; 
}
}
j0 = j0array[ij0]; cj0 = cj0array[ij0]; sj0 = sj0array[ij0];

{
IkReal j1eval[2];
IkReal x22=(cj0*py);
IkReal x23=(px*sj0);
j1eval[0]=((52.925)+(((-1.0)*x22))+(((-20.0)*x22*x23))+(((10.0)*(pz*pz)))+(((-46.0)*pz))+x23+(((10.0)*(x23*x23)))+(((10.0)*(x22*x22))));
j1eval[1]=((IKabs(((31.51)+(((9.2)*x23))+(((-9.2)*x22))+(((-13.5)*pz)))))+(IKabs(((-20.485)+(((-13.5)*x22))+(((13.5)*x23))+(((9.2)*pz))))));
if( IKabs(j1eval[0]) < 0.0000010000000000  || IKabs(j1eval[1]) < 0.0000010000000000  )
{
continue; // no branches [j1, j2, j3]

} else
{
{
IkReal j1array[2], cj1array[2], sj1array[2];
bool j1valid[2]={false};
_nj1 = 2;
IkReal x24=(cj0*py);
IkReal x25=(px*sj0);
IkReal x26=((-20.485)+(((-13.5)*x24))+(((13.5)*x25))+(((9.2)*pz)));
IkReal x27=((31.51)+(((9.2)*x25))+(((-9.2)*x24))+(((-13.5)*pz)));
CheckValue<IkReal> x30 = IKatan2WithCheck(IkReal(x26),IkReal(x27),IKFAST_ATAN2_MAGTHRESH);
if(!x30.valid){
continue;
}
IkReal x28=((1.0)*(x30.value));
if((((x26*x26)+(x27*x27))) < -0.00001)
continue;
CheckValue<IkReal> x31=IKPowWithIntegerCheck(IKabs(IKsqrt(((x26*x26)+(x27*x27)))),-1);
if(!x31.valid){
continue;
}
if( (((x31.value)*(((-5.291575)+(((-1.0)*(px*px)))+(((4.6)*pz))+(((-0.1)*x25))+(((-1.0)*(pz*pz)))+(((0.1)*x24))+(((-1.0)*(py*py))))))) < -1-IKFAST_SINCOS_THRESH || (((x31.value)*(((-5.291575)+(((-1.0)*(px*px)))+(((4.6)*pz))+(((-0.1)*x25))+(((-1.0)*(pz*pz)))+(((0.1)*x24))+(((-1.0)*(py*py))))))) > 1+IKFAST_SINCOS_THRESH )
    continue;
IkReal x29=IKasin(((x31.value)*(((-5.291575)+(((-1.0)*(px*px)))+(((4.6)*pz))+(((-0.1)*x25))+(((-1.0)*(pz*pz)))+(((0.1)*x24))+(((-1.0)*(py*py)))))));
j1array[0]=((((-1.0)*x28))+(((-1.0)*x29)));
sj1array[0]=IKsin(j1array[0]);
cj1array[0]=IKcos(j1array[0]);
j1array[1]=((3.14159265358979)+(((-1.0)*x28))+x29);
sj1array[1]=IKsin(j1array[1]);
cj1array[1]=IKcos(j1array[1]);
if( j1array[0] > IKPI )
{
    j1array[0]-=IK2PI;
}
else if( j1array[0] < -IKPI )
{    j1array[0]+=IK2PI;
}
j1valid[0] = true;
if( j1array[1] > IKPI )
{
    j1array[1]-=IK2PI;
}
else if( j1array[1] < -IKPI )
{    j1array[1]+=IK2PI;
}
j1valid[1] = true;
for(int ij1 = 0; ij1 < 2; ++ij1)
{
if( !j1valid[ij1] )
{
    continue;
}
_ij1[0] = ij1; _ij1[1] = -1;
for(int iij1 = ij1+1; iij1 < 2; ++iij1)
{
if( j1valid[iij1] && IKabs(cj1array[ij1]-cj1array[iij1]) < IKFAST_SOLUTION_THRESH && IKabs(sj1array[ij1]-sj1array[iij1]) < IKFAST_SOLUTION_THRESH )
{
    j1valid[iij1]=false; _ij1[1] = iij1; break; 
}
}
j1 = j1array[ij1]; cj1 = cj1array[ij1]; sj1 = sj1array[ij1];

{
IkReal j2array[1], cj2array[1], sj2array[1];
bool j2valid[1]={false};
_nj2 = 1;
IkReal x32=((0.0672846440822555)*sj1);
IkReal x33=(cj0*py);
IkReal x34=(px*sj0);
IkReal x35=((0.102428855500219)*pz);
IkReal x36=((0.102428855500219)*cj1);
IkReal x37=((0.0672846440822555)*cj1);
IkReal x38=((0.102428855500219)*sj1);
if( IKabs(((0.0170013877457842)+((x34*x37))+(((0.238950599854617)*cj1))+(((-1.0)*pz*x32))+(((0.149633238614177)*sj1))+(((-1.0)*cj1*x35))+(((-1.0)*x33*x37))+(((-1.0)*x34*x38))+((x33*x38)))) < IKFAST_ATAN2_MAGTHRESH && IKabs(((-1.00090413740486)+(((0.238950599854617)*sj1))+((pz*x37))+((x34*x36))+(((-1.0)*x32*x33))+(((-1.0)*x33*x36))+(((-1.0)*sj1*x35))+((x32*x34))+(((-0.149633238614177)*cj1)))) < IKFAST_ATAN2_MAGTHRESH && IKabs(IKsqr(((0.0170013877457842)+((x34*x37))+(((0.238950599854617)*cj1))+(((-1.0)*pz*x32))+(((0.149633238614177)*sj1))+(((-1.0)*cj1*x35))+(((-1.0)*x33*x37))+(((-1.0)*x34*x38))+((x33*x38))))+IKsqr(((-1.00090413740486)+(((0.238950599854617)*sj1))+((pz*x37))+((x34*x36))+(((-1.0)*x32*x33))+(((-1.0)*x33*x36))+(((-1.0)*sj1*x35))+((x32*x34))+(((-0.149633238614177)*cj1))))-1) <= IKFAST_SINCOS_THRESH )
    continue;
j2array[0]=IKatan2(((0.0170013877457842)+((x34*x37))+(((0.238950599854617)*cj1))+(((-1.0)*pz*x32))+(((0.149633238614177)*sj1))+(((-1.0)*cj1*x35))+(((-1.0)*x33*x37))+(((-1.0)*x34*x38))+((x33*x38))), ((-1.00090413740486)+(((0.238950599854617)*sj1))+((pz*x37))+((x34*x36))+(((-1.0)*x32*x33))+(((-1.0)*x33*x36))+(((-1.0)*sj1*x35))+((x32*x34))+(((-0.149633238614177)*cj1))));
sj2array[0]=IKsin(j2array[0]);
cj2array[0]=IKcos(j2array[0]);
if( j2array[0] > IKPI )
{
    j2array[0]-=IK2PI;
}
else if( j2array[0] < -IKPI )
{    j2array[0]+=IK2PI;
}
j2valid[0] = true;
for(int ij2 = 0; ij2 < 1; ++ij2)
{
if( !j2valid[ij2] )
{
    continue;
}
_ij2[0] = ij2; _ij2[1] = -1;
for(int iij2 = ij2+1; iij2 < 1; ++iij2)
{
if( j2valid[iij2] && IKabs(cj2array[ij2]-cj2array[iij2]) < IKFAST_SOLUTION_THRESH && IKabs(sj2array[ij2]-sj2array[iij2]) < IKFAST_SOLUTION_THRESH )
{
    j2valid[iij2]=false; _ij2[1] = iij2; break; 
}
}
j2 = j2array[ij2]; cj2 = cj2array[ij2]; sj2 = sj2array[ij2];
{
IkReal evalcond[4];
IkReal x39=IKcos(j2);
IkReal x40=IKsin(j2);
IkReal x41=(px*sj0);
IkReal x42=(cj0*py);
IkReal x43=((1.0)*pz);
IkReal x44=(cj1*x39);
IkReal x45=(cj1*x40);
IkReal x46=(sj1*x40);
IkReal x47=(sj1*x39);
evalcond[0]=((-4.6)+(((6.82)*x40))+((cj1*pz))+((sj1*x41))+(((-1.0)*sj1*x42))+(((-2.3)*cj1))+(((-4.48)*x39))+(((0.05)*sj1)));
evalcond[1]=((-6.75)+((cj1*x41))+(((-1.0)*cj1*x42))+(((-6.82)*x39))+(((2.3)*sj1))+(((-1.0)*sj1*x43))+(((0.05)*cj1))+(((-4.48)*x40)));
evalcond[2]=((-4.48)+(((-6.75)*x40))+((x41*x47))+((x41*x45))+(((2.3)*x46))+((pz*x44))+(((-1.0)*x43*x46))+(((-2.3)*x44))+(((0.05)*x47))+(((0.05)*x45))+(((-1.0)*x42*x47))+(((-1.0)*x42*x45))+(((-4.6)*x39)));
evalcond[3]=((-6.82)+(((-6.75)*x39))+((x41*x44))+(((2.3)*x45))+(((2.3)*x47))+(((-1.0)*x43*x47))+(((-1.0)*x43*x45))+(((-1.0)*x41*x46))+(((4.6)*x40))+((x42*x46))+(((0.05)*x44))+(((-1.0)*x42*x44))+(((-0.05)*x46)));
if( IKabs(evalcond[0]) > IKFAST_EVALCOND_THRESH  || IKabs(evalcond[1]) > IKFAST_EVALCOND_THRESH  || IKabs(evalcond[2]) > IKFAST_EVALCOND_THRESH  || IKabs(evalcond[3]) > IKFAST_EVALCOND_THRESH  )
{
continue;
}
}

{
IkReal j3eval[2];
IkReal x48=sj0*sj0;
IkReal x49=sj1*sj1;
IkReal x50=cj2*cj2;
IkReal x51=cj1*cj1;
IkReal x52=sj2*sj2;
IkReal x53=(cj2*sj0);
IkReal x54=(sj0*sj2);
IkReal x55=(x48*x51);
IkReal x56=(x48*x49);
j3eval[0]=(((x50*x56))+((x50*x55))+((x52*x55))+((x52*x56)));
j3eval[1]=((IKabs((((cj1*x54))+((sj1*x53)))))+(IKabs((((cj1*x53))+(((-1.0)*sj1*x54))))));
if( IKabs(j3eval[0]) < 0.0000010000000000  || IKabs(j3eval[1]) < 0.0000010000000000  )
{
{
IkReal evalcond[1];
bool bgotonextstatement = true;
do
{
evalcond[0]=((-3.14159265358979)+(IKfmod(((3.14159265358979)+(IKabs(j0))), 6.28318530717959)));
if( IKabs(evalcond[0]) < 0.0000050000000000  )
{
bgotonextstatement=false;
{
IkReal j3array[1], cj3array[1], sj3array[1];
bool j3valid[1]={false};
_nj3 = 1;
j3array[0]=0;
sj3array[0]=IKsin(j3array[0]);
cj3array[0]=IKcos(j3array[0]);
if( j3array[0] > IKPI )
{
    j3array[0]-=IK2PI;
}
else if( j3array[0] < -IKPI )
{    j3array[0]+=IK2PI;
}
j3valid[0] = true;
for(int ij3 = 0; ij3 < 1; ++ij3)
{
if( !j3valid[ij3] )
{
    continue;
}
_ij3[0] = ij3; _ij3[1] = -1;
for(int iij3 = ij3+1; iij3 < 1; ++iij3)
{
if( j3valid[iij3] && IKabs(cj3array[ij3]-cj3array[iij3]) < IKFAST_SOLUTION_THRESH && IKabs(sj3array[ij3]-sj3array[iij3]) < IKFAST_SOLUTION_THRESH )
{
    j3valid[iij3]=false; _ij3[1] = iij3; break; 
}
}
j3 = j3array[ij3]; cj3 = cj3array[ij3]; sj3 = sj3array[ij3];

{
std::vector<IkSingleDOFSolutionBase<IkReal> > vinfos(4);
vinfos[0].jointtype = 1;
vinfos[0].foffset = j0;
vinfos[0].indices[0] = _ij0[0];
vinfos[0].indices[1] = _ij0[1];
vinfos[0].maxsolutions = _nj0;
vinfos[1].jointtype = 1;
vinfos[1].foffset = j1;
vinfos[1].indices[0] = _ij1[0];
vinfos[1].indices[1] = _ij1[1];
vinfos[1].maxsolutions = _nj1;
vinfos[2].jointtype = 1;
vinfos[2].foffset = j2;
vinfos[2].indices[0] = _ij2[0];
vinfos[2].indices[1] = _ij2[1];
vinfos[2].maxsolutions = _nj2;
vinfos[3].jointtype = 1;
vinfos[3].foffset = j3;
vinfos[3].indices[0] = _ij3[0];
vinfos[3].indices[1] = _ij3[1];
vinfos[3].maxsolutions = _nj3;
std::vector<int> vfree(0);
solutions.AddSolution(vinfos,vfree);
}
}
}

}
} while(0);
if( bgotonextstatement )
{
bool bgotonextstatement = true;
do
{
evalcond[0]=((-3.14159265358979)+(IKfmod(((3.14159265358979)+(IKabs(((-3.14159265358979)+j0)))), 6.28318530717959)));
if( IKabs(evalcond[0]) < 0.0000050000000000  )
{
bgotonextstatement=false;
{
IkReal j3array[1], cj3array[1], sj3array[1];
bool j3valid[1]={false};
_nj3 = 1;
j3array[0]=0;
sj3array[0]=IKsin(j3array[0]);
cj3array[0]=IKcos(j3array[0]);
if( j3array[0] > IKPI )
{
    j3array[0]-=IK2PI;
}
else if( j3array[0] < -IKPI )
{    j3array[0]+=IK2PI;
}
j3valid[0] = true;
for(int ij3 = 0; ij3 < 1; ++ij3)
{
if( !j3valid[ij3] )
{
    continue;
}
_ij3[0] = ij3; _ij3[1] = -1;
for(int iij3 = ij3+1; iij3 < 1; ++iij3)
{
if( j3valid[iij3] && IKabs(cj3array[ij3]-cj3array[iij3]) < IKFAST_SOLUTION_THRESH && IKabs(sj3array[ij3]-sj3array[iij3]) < IKFAST_SOLUTION_THRESH )
{
    j3valid[iij3]=false; _ij3[1] = iij3; break; 
}
}
j3 = j3array[ij3]; cj3 = cj3array[ij3]; sj3 = sj3array[ij3];

{
std::vector<IkSingleDOFSolutionBase<IkReal> > vinfos(4);
vinfos[0].jointtype = 1;
vinfos[0].foffset = j0;
vinfos[0].indices[0] = _ij0[0];
vinfos[0].indices[1] = _ij0[1];
vinfos[0].maxsolutions = _nj0;
vinfos[1].jointtype = 1;
vinfos[1].foffset = j1;
vinfos[1].indices[0] = _ij1[0];
vinfos[1].indices[1] = _ij1[1];
vinfos[1].maxsolutions = _nj1;
vinfos[2].jointtype = 1;
vinfos[2].foffset = j2;
vinfos[2].indices[0] = _ij2[0];
vinfos[2].indices[1] = _ij2[1];
vinfos[2].maxsolutions = _nj2;
vinfos[3].jointtype = 1;
vinfos[3].foffset = j3;
vinfos[3].indices[0] = _ij3[0];
vinfos[3].indices[1] = _ij3[1];
vinfos[3].maxsolutions = _nj3;
std::vector<int> vfree(0);
solutions.AddSolution(vinfos,vfree);
}
}
}

}
} while(0);
if( bgotonextstatement )
{
bool bgotonextstatement = true;
do
{
if( 1 )
{
bgotonextstatement=false;
continue; // branch miss [j3]

}
} while(0);
if( bgotonextstatement )
{
}
}
}
}

} else
{
{
IkReal j3array[2], cj3array[2], sj3array[2];
bool j3valid[2]={false};
_nj3 = 2;
IkReal x57=(cj2*sj0);
IkReal x58=(sj0*sj2);
IkReal x59=(((cj1*x58))+((sj1*x57)));
IkReal x60=(((cj1*x57))+(((-1.0)*sj1*x58)));
CheckValue<IkReal> x63 = IKatan2WithCheck(IkReal(x59),IkReal(x60),IKFAST_ATAN2_MAGTHRESH);
if(!x63.valid){
continue;
}
IkReal x61=((1.0)*(x63.value));
if((((x59*x59)+(x60*x60))) < -0.00001)
continue;
CheckValue<IkReal> x64=IKPowWithIntegerCheck(IKabs(IKsqrt(((x59*x59)+(x60*x60)))),-1);
if(!x64.valid){
continue;
}
if( (((x64.value)*(IKcos(r00)))) < -1-IKFAST_SINCOS_THRESH || (((x64.value)*(IKcos(r00)))) > 1+IKFAST_SINCOS_THRESH )
    continue;
IkReal x62=IKasin(((x64.value)*(IKcos(r00))));
j3array[0]=(x62+(((-1.0)*x61)));
sj3array[0]=IKsin(j3array[0]);
cj3array[0]=IKcos(j3array[0]);
j3array[1]=((3.14159265358979)+(((-1.0)*x61))+(((-1.0)*x62)));
sj3array[1]=IKsin(j3array[1]);
cj3array[1]=IKcos(j3array[1]);
if( j3array[0] > IKPI )
{
    j3array[0]-=IK2PI;
}
else if( j3array[0] < -IKPI )
{    j3array[0]+=IK2PI;
}
j3valid[0] = true;
if( j3array[1] > IKPI )
{
    j3array[1]-=IK2PI;
}
else if( j3array[1] < -IKPI )
{    j3array[1]+=IK2PI;
}
j3valid[1] = true;
for(int ij3 = 0; ij3 < 2; ++ij3)
{
if( !j3valid[ij3] )
{
    continue;
}
_ij3[0] = ij3; _ij3[1] = -1;
for(int iij3 = ij3+1; iij3 < 2; ++iij3)
{
if( j3valid[iij3] && IKabs(cj3array[ij3]-cj3array[iij3]) < IKFAST_SOLUTION_THRESH && IKabs(sj3array[ij3]-sj3array[iij3]) < IKFAST_SOLUTION_THRESH )
{
    j3valid[iij3]=false; _ij3[1] = iij3; break; 
}
}
j3 = j3array[ij3]; cj3 = cj3array[ij3]; sj3 = sj3array[ij3];

{
std::vector<IkSingleDOFSolutionBase<IkReal> > vinfos(4);
vinfos[0].jointtype = 1;
vinfos[0].foffset = j0;
vinfos[0].indices[0] = _ij0[0];
vinfos[0].indices[1] = _ij0[1];
vinfos[0].maxsolutions = _nj0;
vinfos[1].jointtype = 1;
vinfos[1].foffset = j1;
vinfos[1].indices[0] = _ij1[0];
vinfos[1].indices[1] = _ij1[1];
vinfos[1].maxsolutions = _nj1;
vinfos[2].jointtype = 1;
vinfos[2].foffset = j2;
vinfos[2].indices[0] = _ij2[0];
vinfos[2].indices[1] = _ij2[1];
vinfos[2].maxsolutions = _nj2;
vinfos[3].jointtype = 1;
vinfos[3].foffset = j3;
vinfos[3].indices[0] = _ij3[0];
vinfos[3].indices[1] = _ij3[1];
vinfos[3].maxsolutions = _nj3;
std::vector<int> vfree(0);
solutions.AddSolution(vinfos,vfree);
}
}
}

}

}
}
}
}
}

}

}
}
}

}

}
}
return solutions.GetNumSolutions()>0;
}
};


/// solves the inverse kinematics equations.
/// \param pfree is an array specifying the free joints of the chain.
IKFAST_API bool ComputeIk(const IkReal* eetrans, const IkReal* eerot, const IkReal* pfree, IkSolutionListBase<IkReal>& solutions) {
IKSolver solver;
return solver.ComputeIk(eetrans,eerot,pfree,solutions);
}

IKFAST_API bool ComputeIk2(const IkReal* eetrans, const IkReal* eerot, const IkReal* pfree, IkSolutionListBase<IkReal>& solutions, void* pOpenRAVEManip) {
IKSolver solver;
return solver.ComputeIk(eetrans,eerot,pfree,solutions);
}

IKFAST_API const char* GetKinematicsHash() { return "313691d5fc821fa290c8544423d7a924"; }

IKFAST_API const char* GetIkFastVersion() { return "0x10000049"; }

#ifdef IKFAST_NAMESPACE
} // end namespace
#endif

#ifndef IKFAST_NO_MAIN
#include <stdio.h>
#include <stdlib.h>
#ifdef IKFAST_NAMESPACE
using namespace IKFAST_NAMESPACE;
#endif
int main(int argc, char** argv)
{
    if( argc != 12+GetNumFreeParameters()+1 ) {
        printf("\nUsage: ./ik r00 r01 r02 t0 r10 r11 r12 t1 r20 r21 r22 t2 free0 ...\n\n"
               "Returns the ik solutions given the transformation of the end effector specified by\n"
               "a 3x3 rotation R (rXX), and a 3x1 translation (tX).\n"
               "There are %d free parameters that have to be specified.\n\n",GetNumFreeParameters());
        return 1;
    }

    IkSolutionList<IkReal> solutions;
    std::vector<IkReal> vfree(GetNumFreeParameters());
    IkReal eerot[9],eetrans[3];
    eerot[0] = atof(argv[1]); eerot[1] = atof(argv[2]); eerot[2] = atof(argv[3]); eetrans[0] = atof(argv[4]);
    eerot[3] = atof(argv[5]); eerot[4] = atof(argv[6]); eerot[5] = atof(argv[7]); eetrans[1] = atof(argv[8]);
    eerot[6] = atof(argv[9]); eerot[7] = atof(argv[10]); eerot[8] = atof(argv[11]); eetrans[2] = atof(argv[12]);
    for(std::size_t i = 0; i < vfree.size(); ++i)
        vfree[i] = atof(argv[13+i]);
    bool bSuccess = ComputeIk(eetrans, eerot, vfree.size() > 0 ? &vfree[0] : NULL, solutions);

    if( !bSuccess ) {
        fprintf(stderr,"Failed to get ik solution\n");
        return -1;
    }

    printf("Found %d ik solutions:\n", (int)solutions.GetNumSolutions());
    std::vector<IkReal> solvalues(GetNumJoints());
    for(std::size_t i = 0; i < solutions.GetNumSolutions(); ++i) {
        const IkSolutionBase<IkReal>& sol = solutions.GetSolution(i);
        printf("sol%d (free=%d): ", (int)i, (int)sol.GetFree().size());
        std::vector<IkReal> vsolfree(sol.GetFree().size());
        sol.GetSolution(&solvalues[0],vsolfree.size()>0?&vsolfree[0]:NULL);
        for( std::size_t j = 0; j < solvalues.size(); ++j)
            printf("%.15f, ", solvalues[j]);
        printf("\n");
    }
    return 0;
}

#endif
