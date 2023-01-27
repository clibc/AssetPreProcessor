#pragma once

#include <math.h>
#include <cmath>
#include <stdlib.h> // for rand()

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif

#define PI  3.14159265359f
#define TAU 6.28318530718f

#define EPSILON 0.00001f

static inline float PowerF32(float, float);

// TODO : Pull these out of union
union v2
{
    struct
    {
        float x,y;
    };

    v2() = default;
    inline
    v2(float ix, float iy)
    {
        x = ix;
        y = iy;
    }
    inline
    v2(float a)
    {
        x = a;
        y = a;
    }
};

union v3
{
    struct
    {
        float x,y,z;
    };

    v3() = default;
    inline
    v3(float ix, float iy, float iz)
    {
        x = ix;
        y = iy;
        z = iz;
    }
    inline
    v3(float a)
    {
        x = a;
        y = a;
        z = a;
    }
};

union v4
{
    struct
    {
        float x,y,z,w;
    };

    v4() = default;
    inline
    v4(float ix, float iy, float iz, float iw)
    {
        x = ix;
        y = iy;
        z = iz;
        w = iw;
    }
    inline
    v4(v3 i, float iw)
    {
        x = i.x;
        y = i.y;
        z = i.z;
        w = iw;
    }
    inline
    v4(float a)
    {
        x = a;
        y = a;
        z = a;
        w = a;
    }
};

struct m4
{ 
    float Elements[4][4];
};

inline void
SetColumn(m4& M, int Column, float A1, float A2, float A3, float A4)
{
    M.Elements[Column][0] = A1;
    M.Elements[Column][1] = A2;
    M.Elements[Column][2] = A3;
    M.Elements[Column][3] = A4;
}

inline void
SetRow(m4& M, int Row, float A1, float A2, float A3, float A4)
{
    M.Elements[0][Row] = A1;
    M.Elements[1][Row] = A2;
    M.Elements[2][Row] = A3;
    M.Elements[3][Row] = A4;
}

inline m4
Identity(float A)
{
    m4 Result = {};
    Result.Elements[0][0] = A;
    Result.Elements[1][1] = A;
    Result.Elements[2][2] = A;
    Result.Elements[3][3] = A;
    return Result;
}

inline m4
Transpose(const m4& A)
{
    m4 Result;
    for(int Row = 0;
        Row < 4;
        ++Row)
    {
        for(int Column = 0;
            Column < 4;
            ++Column)
        {
            Result.Elements[Column][Row] = A.Elements[Row][Column];
        }
    }
    return Result;
}

inline m4
Inverse(const m4& A)
{
    const float (*M)[4] = A.Elements;

    float Determinant =
        M[0][0] * (M[1][1]*M[2][2]*M[3][3] + M[2][1]*M[3][2]*M[1][3] + M[3][1]*M[1][2]*M[2][3]
                   - M[3][1]*M[2][2]*M[1][3] - M[2][1]*M[1][2]*M[3][3] - M[1][1]*M[3][2]*M[2][3]) -
        M[0][1] * (M[1][0]*M[2][2]*M[3][3] + M[2][0]*M[3][2]*M[1][3] + M[3][0]*M[1][2]*M[2][3]
                   - M[3][0]*M[2][2]*M[1][3] - M[2][0]*M[1][2]*M[3][3] - M[1][0]*M[3][2]*M[2][3]) +
        M[0][2] * (M[1][0]*M[2][1]*M[3][3] + M[2][0]*M[3][1]*M[1][3] + M[3][0]*M[1][1]*M[2][3]
                   - M[3][0]*M[2][1]*M[1][3] - M[2][0]*M[1][1]*M[3][3] - M[1][0]*M[3][1]*M[2][3]) -
        M[0][3] * (M[1][0]*M[2][1]*M[3][2] + M[2][0]*M[3][1]*M[1][2] + M[3][0]*M[1][1]*M[2][2]
                   - M[3][0]*M[2][1]*M[1][2] - M[2][0]*M[1][1]*M[3][2] - M[1][0]*M[3][1]*M[2][2]);
    m4 Adjugate;
    
    for(int Row = 0;
        Row < 4;
        ++Row)
    {
        for(int Column = 0;
            Column < 4;
            ++Column)
        {
            float SubMatrix[3][3];
            int SX = 0, SY = 0;
            
            for(int Y = 0;
                Y < 4;
                ++Y)
            {
                if(Row == Y)
                {
                    continue;
                }

                for(int X = 0;
                    X < 4;
                    ++X)
                {
                    if(Column == X)
                    {
                        continue;
                    }
                    
                    SubMatrix[SX][SY] = M[X][Y];
                    SX += 1;
                }
                SY += 1;
                SX = 0;
            }

            // Transpose on the way
            Adjugate.Elements[Row][Column] =
                SubMatrix[0][0] * SubMatrix[1][1] * SubMatrix[2][2] +
                SubMatrix[1][0] * SubMatrix[2][1] * SubMatrix[0][2] +
                SubMatrix[2][0] * SubMatrix[0][1] * SubMatrix[1][2] -
                SubMatrix[2][0] * SubMatrix[1][1] * SubMatrix[0][2] -
                SubMatrix[1][0] * SubMatrix[0][1] * SubMatrix[2][2] -
                SubMatrix[0][0] * SubMatrix[2][1] * SubMatrix[1][2];

            Adjugate.Elements[Row][Column] = PowerF32(-1.0f, (float)(Column+Row)) * Adjugate.Elements[Row][Column] * 1.0f/Determinant;
        }
    }
    
    return Adjugate;
}

inline m4 operator*(const m4& A, m4 const& B)
{
    m4 Result;
    for(int Row = 0;
        Row < 4;
        ++Row)
    {
        for(int Column = 0;
            Column < 4;
            ++Column)
        {
            Result.Elements[Row][Column] =
                A.Elements[0][Column] * B.Elements[Row][0] +
                A.Elements[1][Column] * B.Elements[Row][1] +
                A.Elements[2][Column] * B.Elements[Row][2] +
                A.Elements[3][Column] * B.Elements[Row][3];
        }
    }

    return Result;
}

inline m4 operator+(const m4& A, m4 const& B)
{
    m4 Result;
    for(int Row = 0;
        Row < 4;
        ++Row)
    {
        for(int Column = 0;
            Column < 4;
            ++Column)
        {
            Result.Elements[Column][Row] = A.Elements[Column][Row] + B.Elements[Column][Row];
        }
    }

    return Result;
}

inline m4 operator-(const m4& A, m4 const& B)
{
    m4 Result;
    for(int Row = 0;
        Row < 4;
        ++Row)
    {
        for(int Column = 0;
            Column < 4;
            ++Column)
        {
            Result.Elements[Column][Row] = A.Elements[Column][Row] - B.Elements[Column][Row];
        }
    }

    return Result;
}

inline v4
operator*(const m4& M, const v4& V)
{

    v4 Result;
    
    Result.x = M.Elements[0][0] * V.x + M.Elements[0][1] * V.y + M.Elements[0][2] * V.z + M.Elements[0][3] * V.w;
    Result.y = M.Elements[1][0] * V.x + M.Elements[1][1] * V.y + M.Elements[1][2] * V.z + M.Elements[1][3] * V.w;
    Result.z = M.Elements[2][0] * V.x + M.Elements[2][1] * V.y + M.Elements[2][2] * V.z + M.Elements[2][3] * V.w;
    Result.w = M.Elements[3][0] * V.x + M.Elements[3][1] * V.y + M.Elements[3][2] * V.z + M.Elements[3][3] * V.w;

    return Result;
}

inline m4
MakeOrthoMatrix(float Left, float Right,
                float Top, float Bottom,
                float Near, float Far)
{
    m4 OrthoMatrix;

    // SetRow(OrthoMatrix, 0, 2.0f/(Right - Left), 0, 0, -((Right + Left)/(Right - Left)));
    // SetRow(OrthoMatrix, 1, 0, 2.0f/(Top - Bottom), 0, -((Top + Bottom)/(Top - Bottom)));
    // SetRow(OrthoMatrix, 2, 0, 0, -2.0f/(Far - Near), -((Far+Near)/(Far-Near)));
    // SetRow(OrthoMatrix, 3, 0, 0, 0, 1);

    OrthoMatrix.Elements[0][0] = 2.0f/(Right - Left);
    OrthoMatrix.Elements[0][1] = 0;
    OrthoMatrix.Elements[0][2] = 0;
    OrthoMatrix.Elements[0][3] = -((Right + Left)/(Right - Left));

    OrthoMatrix.Elements[1][0] = 0;
    OrthoMatrix.Elements[1][1] = 2.0f/(Top - Bottom);
    OrthoMatrix.Elements[1][2] = 0;
    OrthoMatrix.Elements[1][3] = -((Top + Bottom)/(Top - Bottom));

    OrthoMatrix.Elements[2][0] = 0;
    OrthoMatrix.Elements[2][1] = 0;
    OrthoMatrix.Elements[2][2] = (-Far-Near)/(Near-Far);
    OrthoMatrix.Elements[2][3] = (2*Far*Near)/(Near-Far);

    OrthoMatrix.Elements[3][0] = 0;
    OrthoMatrix.Elements[3][1] = 0;
    OrthoMatrix.Elements[3][2] = 0;
    OrthoMatrix.Elements[3][3] = 1;
    
    return OrthoMatrix;
}

inline m4
MakeMatrix(float M00, float M01, float M02, float M03,
              float M10, float M11, float M12, float M13,
              float M20, float M21, float M22, float M23,
              float M30, float M31, float M32, float M33)
{
    m4 Result;

    Result.Elements[0][0] = M00;
    Result.Elements[0][1] = M01;
    Result.Elements[0][2] = M02;
    Result.Elements[0][3] = M03;

    Result.Elements[1][0] = M10;
    Result.Elements[1][1] = M11;
    Result.Elements[1][2] = M12;
    Result.Elements[1][3] = M13;

    Result.Elements[2][0] = M20;
    Result.Elements[2][1] = M21;
    Result.Elements[2][2] = M22;
    Result.Elements[2][3] = M23;

    Result.Elements[3][0] = M30;
    Result.Elements[3][1] = M31;
    Result.Elements[3][2] = M32;
    Result.Elements[3][3] = M33;

    return Result;
}

inline v2 operator+(v2 L, v2 R)    {return {L.x + R.x, L.y + R.y};}
inline v2 operator-(v2 L, v2 R)    {return {L.x - R.x, L.y - R.y};}
inline v2 operator+(v2 L, float R) {return {L.x + R, L.y + R};}
inline v2 operator-(v2 L, float R) {return {L.x - R, L.y - R};}
inline v2 operator*(float L, v2 R) {return {L * R.x, L * R.y};}
inline v2 operator*(v2 L, float R) {return {L.x * R, L.y * R};}
inline v2 operator/(float L, v2 R) {return {L / R.x, L / R.y};}
inline v2 operator/(v2 L, float R) {return {L.x / R, L.y / R};}
inline v2 operator*(v2 L, v2 R)    {return {L.x * R.x, L.y * R.y};}
inline v2& operator+=(v2& L, float R) {return (L = L + R);}
inline v2& operator+=(v2& L, v2 R)    {return (L = L + R);}
inline v2& operator-=(v2& L, float R) {return (L = L - R);}
inline v2& operator-=(v2& L, v2 R)    {return (L = L - R);}
inline v2& operator*=(v2& L, float R) {return (L = L * R);}
inline v2& operator/=(v2& L, float R) {return (L = L / R);}

inline v3 operator+(v3 L, v3 R)    {return {L.x + R.x, L.y + R.y, L.z + R.z};}
inline v3 operator-(v3 L, v3 R)    {return {L.x - R.x, L.y - R.y, L.z - R.z};}
inline v3 operator+(v3 L, float R) {return {L.x + R, L.y + R, L.z + R};}
inline v3 operator-(v3 L, float R) {return {L.x - R, L.y - R, L.z - R};}
inline v3 operator*(float L, v3 R) {return {L * R.x, L * R.y, L * R.z};}
inline v3 operator*(v3 L, float R) {return {L.x * R, L.y * R, L.z * R};}
inline v3 operator/(float L, v3 R) {return {L / R.x, L / R.y, L / R.z};}
inline v3 operator/(v3 L, float R) {return {L.x / R, L.y / R, L.z / R};}
inline v3 operator*(v3 L, v3 R)    {return {L.x * R.x, L.y * R.y, L.z * R.z};}
inline v3& operator+=(v3& L, float R) {return (L = L + R);}
inline v3& operator+=(v3& L, v3 R)    {return (L = L + R);}
inline v3& operator-=(v3& L, float R) {return (L = L - R);}
inline v3& operator-=(v3& L, v3 R)    {return (L = L - R);}
inline v3& operator*=(v3& L, float R) {return (L = L * R);}
inline v3& operator/=(v3& L, float R) {return (L = L / R);}

inline v4 operator+(v4 L, v4 R)    {return {L.x + R.x, L.y + R.y, L.z + R.z, L.w + R.w};}
inline v4 operator-(v4 L, v4 R)    {return {L.x - R.x, L.y - R.y, L.z - R.z, L.w - R.w};}
inline v4 operator+(v4 L, float R) {return {L.x + R, L.y + R, L.z + R, L.w + R};}
inline v4 operator-(v4 L, float R) {return {L.x - R, L.y - R, L.z - R, L.w - R};}
inline v4 operator*(float L, v4 R) {return {L * R.x, L * R.y, L * R.z, L * R.w};}
inline v4 operator*(v4 L, float R) {return {L.x * R, L.y * R, L.z * R, L.w * R};}
inline v4 operator/(float L, v4 R) {return {L / R.x, L / R.y, L / R.z, L / R.w};}
inline v4 operator/(v4 L, float R) {return {L.x / R, L.y / R, L.z / R, L.w / R};}
inline v4 operator*(v4 L, v4 R)    {return {L.x * R.x, L.y * R.y, L.z * R.z, L.w * R.w};}
inline v4& operator+=(v4& L, float R) {return (L = L + R);}
inline v4& operator+=(v4& L, v4 R)    {return (L = L + R);}
inline v4& operator-=(v4& L, float R) {return (L = L - R);}
inline v4& operator-=(v4& L, v4 R)    {return (L = L - R);}
inline v4& operator*=(v4& L, float R) {return (L = L * R);}
inline v4& operator/=(v4& L, float R) {return (L = L / R);}

inline void
Print(const v2& A)
{
#ifdef DebugLog
    DebugLog("Vec2(%f, %f)\n", A.x, A.y);
#endif
}

inline void
Print(const v3& A)
{
#ifdef DebugLog
    DebugLog("Vec3(%f, %f, %f)\n", A.x, A.y, A.z);
#endif
}

inline void
Print(const v4& A)
{
#ifdef DebugLog
    DebugLog("Vec4(%f, %f, %f, %f)\n", A.x, A.y, A.z, A.w);
#endif
}

inline void
Print(const m4& A)
{
#ifdef DebugLog
    DebugLog("m4 RowMajor:\n");
    DebugLog("[%7.3f, %7.3f, %7.3f, %7.3f]\n", A.Elements[0][0],A.Elements[1][0],A.Elements[2][0],A.Elements[3][0]);
    DebugLog("[%7.3f, %7.3f, %7.3f, %7.3f]\n", A.Elements[0][1],A.Elements[1][1],A.Elements[2][1],A.Elements[3][1]);
    DebugLog("[%7.3f, %7.3f, %7.3f, %7.3f]\n", A.Elements[0][2],A.Elements[1][2],A.Elements[2][2],A.Elements[3][2]);
    DebugLog("[%7.3f, %7.3f, %7.3f, %7.3f]\n", A.Elements[0][3],A.Elements[1][3],A.Elements[2][3],A.Elements[3][3]);
#endif
}

inline float
Dot(v2 a, v2 b)
{
    return a.x * b.x + a.y * b.y;
}

inline float
Dot(v3 a, v3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float
Dot(v4 a, v4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w + b.w;
}

inline float
Length(v2 a)
{
    return sqrtf(Dot(a,a));
}

inline float
Length(v3 a)
{
    return sqrtf(Dot(a,a));
}

inline float
Length(v4 a)
{
    return sqrtf(Dot(a,a));
}

inline float
SqrLength(v2 a)
{
    return Dot(a,a);
}

inline float
SqrLength(v3 a)
{
    return Dot(a,a);
}

inline float
SqrLength(v4 a)
{
    return Dot(a,a);
}

inline float
Abs(float a)
{
    return fabs(a);
}

inline v2
Normalize(v2 a)
{
    return a / Length(a);
}

inline v3
Normalize(v3 a)
{
    float L = Length(a);
    if(L < EPSILON)
    {
        return v3(0);
    }
    else
    {
        return a / L;
    }
}

inline v3
Cross(v3 a, v3 b)
{
    v3 r;
    r.x = a.y * b.z - a.z * b.y;
    r.y = a.z * b.x - a.x * b.z;
    r.z = a.x * b.y - a.y * b.x;
    return r;
}

inline v4
Normalize(v4 a)
{
    return a / Length(a);
}

inline float
Sqrt(float a)
{
    return sqrtf(a);
}

inline float
Rand01()
{
    return (float)rand() / (float)RAND_MAX; 
}

inline v3
RandomInUnitSphere()
{
    for(;;)
    {
        v3 p = v3(Rand01() * 2 - 1, Rand01() * 2 - 1, Rand01() * 2 - 1);
        if(SqrLength(p) >= 1) continue;
        return p;
    }
}

inline v3
RandomInHemiSphere(v3 normal)
{
    v3 rand = RandomInUnitSphere();
    return (Dot(normal, rand) > 0) ? rand : rand * -1;
}

inline v3
RandomInUnitDisk()
{
    for(;;)
    {
        v3 p = v3(Rand01()*2-1, Rand01()*2-1, 0);
        if (SqrLength(p) >= 1) continue;
        return p;
    }
}

inline float
Min(float a, float b)
{
    return a < b ? a : b;
}

inline float
Max(float a, float b)
{
    return a > b ? a : b;
}

inline int
Max(int a, int b)
{
    return a > b ? a : b;
}

inline int
Min(int a, int b)
{
    return a < b ? a : b;
}

inline float
PowerF32(float x, float p)
{
    return pow(x, p);
}

inline v3
Reflect(v3 d, v3 n)
{
    return d - 2*Dot(d, n)*n;
}

inline v3
Refract(v3 uv, v3 n, float etai_over_etat)
{
    float cos_theta = Min(Dot(uv * -1, n), 1.0);
    v3 r_out_perp =  etai_over_etat * (uv + cos_theta*n);
    v3 r_out_parallel = -Sqrt(Abs(1.0f - SqrLength(r_out_perp))) * n;
    return r_out_perp + r_out_parallel;
}

inline float
DegToRad(float deg)
{
    return deg * (float)(PI/180.0f);
}

inline float
RadToDeg(float Rad)
{
    return Rad * (float)(180.0f/PI);
}

inline float
Sin(float a)
{
    return sinf(a);
}

inline float
Cos(float a)
{
    return cosf(a);
}

inline float
Tan(float a)
{
    return tanf(a);
}

inline float
SmoothStep(float edge0, float edge1, float x)
{
   if (x < edge0)
      return 0;
   if (x >= edge1)
      return 1;
   x = (x - edge0) / (edge1 - edge0);
   return x * x * (3 - 2 * x);
}

inline float
Sign(float Value)
{
    float Result = 0;
    if(Value > 0)
    {
        Result = 1;
    }
    else
    {
        Result = -1;
    }
    return Result;
}

inline int
TruncateF32ToS32(float Value)
{
    return (int)Value;
}

inline int
RoundF32ToS32(float Value)
{
    return (int)(Value + 0.5f);
}

inline float
Clamp(float x, float min, float max)
{
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

inline int
Clamp(int x, int min, int max)
{
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

static inline float
Lerp(float A, float B, float T)
{
    return A + T*(B-A);
}


static inline v3
Lerp(v3 A, v3 B, float T)
{
    return A + T*(B-A);
}

struct quaternion
{
    float x, y, z, w;
};

static quaternion
MakeQuaternion(v3 Axis, float Angle)
{
    quaternion Result;
    Angle = DegToRad(Angle);
    Axis = Normalize(Axis);
        
    v3 A = Axis * sin(Angle*0.5f);
    Result.x = A.x;
    Result.y = A.y;
    Result.z = A.z;
    Result.w = cos(Angle*0.5f);

    return Result;
}

static m4
QuaternionToMatrix(const quaternion& A)
{
    m4 Result;

    float XX = A.x * A.x;
    float XY = A.x * A.y;
    float XZ = A.x * A.z;
    float XW = A.x * A.w;

    float YY = A.y * A.y;
    float YZ = A.y * A.z;
    float YW = A.y * A.w;

    float ZZ = A.z * A.z;
    float ZW = A.z * A.w;

    Result.Elements[0][0] = 1 - 2 * (YY + ZZ);
    Result.Elements[0][1] = 2 * (XY - ZW);
    Result.Elements[0][2] = 2 * (XZ + YW);
    Result.Elements[0][3] = 0;

    Result.Elements[1][0] = 2 * (XY + ZW);
    Result.Elements[1][1] = 1 - 2 * (XX + ZZ);
    Result.Elements[1][2] = 2 * (YZ - XW);
    Result.Elements[1][3] = 0;

    Result.Elements[2][0] = 2 * (XZ - YW);
    Result.Elements[2][1] = 2 * (YZ + XW);
    Result.Elements[2][2] = 1 - 2 * (XX + YY);
    Result.Elements[2][3] = 0;

    Result.Elements[3][0] = 0;
    Result.Elements[3][1] = 0;
    Result.Elements[3][2] = 0;
    Result.Elements[3][3] = 1;

    return Result;
}

static quaternion
QuaternionInverse(const quaternion& A)
{
    quaternion Result;
    Result = A;
    Result.x *= -1;
    Result.y *= -1;
    Result.z *= -1;
    float SL = Result.x*Result.x + Result.y*Result.y + Result.z*Result.z + Result.w*Result.w;
    Result.x /= SL;
    Result.y /= SL;
    Result.z /= SL;
    Result.w /= SL;

    return Result;
}

quaternion operator*(const quaternion& A, const quaternion& B)
{
    quaternion Result;
    v3 C = Cross(*((v3*)&A), *((v3*)&B)) + (A.w * *((v3*)&B)) + (B.w * *((v3*)&A));
    float W = (A.w * B.w) - Dot(*(v3*)&A, *(v3*)&B);

    *(v3*)&Result = C;
    Result.w = W;
    
    return Result;
}

quaternion operator*(const quaternion& A, const v3& B)
{
    quaternion T;
    T.x = B.x;
    T.y = B.y;
    T.z = B.z;
    T.w = 0;
    
    return A * T;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
