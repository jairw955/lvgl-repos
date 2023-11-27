#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "mat.h"

#define PI   3.14159265358979f

#define radToDeg(x) ((x) * 180.f / PI)
#define degToRad(x) ((x) * PI / 180.f)

void mat_multiply_to(float *c, float *a, float *b)
{
    float x, y, z, w;
    for (int i = 0; i < 4; i++)
    {
        x = a[0];
        y = a[1];
        z = a[2];
        w = a[3];
        c[0] = x * b[0] + y * b[1] + z * b[2] + w * b[3];
        c[1] = x * b[4] + y * b[5] + z * b[6] + w * b[7];
        c[2] = x * b[8] + y * b[9] + z * b[10] + w * b[11];
        c[3] = x * b[12] + y * b[13] + z * b[14] + w * b[15];
        c += 4;
        a += 4;
    }
}

void mat_multiply(float *dst, float *src)
{
    float x, y, z, w;
    for (int i = 0; i < 4; i++)
    {
        x = dst[0];
        y = dst[1];
        z = dst[2];
        w = dst[3];
        dst[0] = x * src[0] + y * src[1]
            + z * src[2] + w * src[3];
        dst[1] = x * src[4] + y * src[5]
            + z * src[6] + w * src[7];
        dst[2] = x * src[8] + y * src[9]
            + z * src[10] + w * src[11];
        dst[3] = x * src[12] + y * src[13]
            + z * src[14] + w * src[15];
        dst += 4;
    }
}

void move_xyz(float *m, float x, float y, float z)
{
    m[0] = 1.0f;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = x;

    m[4] = 0.0f;
    m[5] = 1.0f;
    m[6] = 0.0f;
    m[7] = y;

    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = 1.0f;
    m[11] = z;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

void scale_xyz(float *m, float x, float y, float z)
{
    m[0] = x;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;

    m[4] = 0.0f;
    m[5] = y;
    m[6] = 0.0f;
    m[7] = 0.0f;

    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = z;
    m[11] = 0.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

void rotate(float *m, float x, float y, float z)
{
    float rad;
    float sinx, cosx;
    float siny, cosy;
    float sinz, cosz;

    rad = degToRad(x);
    sinx = sin(x);
    cosx = cos(x);
    rad = degToRad(y);
    siny = sin(y);
    cosy = cos(y);
    rad = degToRad(z);
    sinz = sin(z);
    cosz = cos(z);

    m[0] = cosz * cosy;
    m[1] = -sinz * cosy;
    m[2] = -siny;
    m[3] = 0.0f;

    m[4] = sinz * cosx - cosz * siny * sinx;
    m[5] = cosz * cosy + sinz * siny * sinx;
    m[6] = -sinx * cosy;
    m[7] = 0.0f;

    m[8] = cosz * siny * cosx + sinz * sinx;
    m[9] = cosz * sinx - sinz * siny * cosx;
    m[10] = cosy * cosx;
    m[11] = 0.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

void rotate_x(float *m, float degree)
{
    float rad = degToRad(degree);

    m[0] = 1.0f;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;

    m[4] = 0.0f;
    m[5] = cos(rad);
    m[6] = -sin(rad);
    m[7] = 0.0f;

    m[8] = 0.0f;
    m[9] = sin(rad);
    m[10] = cos(rad);
    m[11] = 0.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

void rotate_y(float *m, float degree)
{
    float rad = degToRad(degree);

    m[0] = cos(rad);
    m[1] = 0.0f;
    m[2] = -sin(rad);
    m[3] = 0.0f;

    m[4] = 0.0f;
    m[5] = 1.0f;
    m[6] = 0.0f;
    m[7] = 0.0f;

    m[8] = sin(rad);
    m[9] = 0.0f;
    m[10] = cos(rad);
    m[11] = 0.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

void rotate_z(float *m, float degree)
{
    float rad = degToRad(degree);

    m[0] = cos(rad);
    m[1] = -sin(rad);
    m[2] = 0.0f;
    m[3] = 0.0f;

    m[4] = sin(rad);
    m[5] = cos(rad);
    m[6] = 0.0f;
    m[7] = 0.0f;

    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = 1.0f;
    m[11] = 0.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

void frustumM(float *m,
              int w, int h,
              int near, int far)
{
    float r_width  = 1.0f / w;
    float r_height = 1.0f / h;
    float r_depth  = 1.0f / (near - far);
    float x = 2.0f * (near * r_width);
    float y = 2.0f * (near * r_height);
    float A = w * r_width;
    float B = h * r_height;
    float C = (far + near) * r_depth;
    float D = 2.0f * (far * near * r_depth);
    m[0] = x;
    m[5] = y;
    m[8] = A;
    m[ 9] = B;
    m[10] = C;
    m[14] = D;
    m[11] = -1.0f;
    m[ 1] = 0.0f;
    m[ 2] = 0.0f;
    m[ 3] = 0.0f;
    m[ 4] = 0.0f;
    m[ 6] = 0.0f;
    m[ 7] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[15] = 0.0f;
}

void perspectiveM(float *m, float fovy,
                  float aspect, float zNear, float zFar)
{
    float f;
    float _tan = (float) tan(fovy * (1 * PI / 360.0));
    float rangeReciprocal = 1.0f / (zNear - zFar);

    if (_tan == 0.0)
        f = 1.0f;
    else
        f = 1.0f / _tan;

    m[0] = f / aspect;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;

    m[4] = 0.0f;
    m[5] = f;
    m[6] = 0.0f;
    m[7] = 0.0f;

    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = (zFar + zNear) * rangeReciprocal;
    m[11] = 2.0f * zFar * zNear * rangeReciprocal;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = -1.0f;
    m[15] = 1.0f;  // HACK, w should not be 0.0f
}

void RA4(float *m, float u, float v, float w, float degree)
{
    float rad = degToRad(degree);
    float _cos = cos(rad);
    float _sin = sin(rad);
    float u2 = u * u;
    float v2 = v * v;
    float w2 = w * w;
    float uv = u * v;
    float uw = v * w;
    float vw = v * w;

    m[0] = u2 + (1 - u2) * _cos;
    m[1] = uv * (1 - _cos) - w * _sin;
    m[2] = uw * (1 - _cos) + v * _sin;
    m[3] = 0.0f;

    m[4] = uv * (1 - _cos) + w * _sin;
    m[5] = v2 + (1 - v2) * _cos;
    m[6] = vw * (1 - _cos) - u * _sin;
    m[7] = 0.0f;

    m[8] = uw * (1 - _cos) - v * _sin;
    m[9] = vw * (1 - _cos) + u * _sin;
    m[10] = w2 + (1 - w2) * _cos;
    m[11] = 0.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
}

void mat_test(float *m, float x, float y, float z,
    float degree)
{
    float rad = degToRad(degree);
    float A, B, C, L, V;

    A = x;
    B = y;
    C = z;

    L = sqrt(A * A + B * B + C * C);
    V = sqrt(B * B + C * C);

    float Mrx[16] = {
        1.0, 0.0,  0.0, 0.0,
        0.0, C/V, -B/V, 0.0,
        0.0, B/V,  C/V, 0.0,
        0.0, 0.0,  0.0, 1.0,
    };
    float Mry[16] = {
        V/L, 0.0, -A/L, 0.0,
        0.0, 1.0,  0.0, 0.0,
        A/L, 0.0,  V/L, 0.0,
        0.0, 0.0,  0.0, 1.0,
    };
    float Mrz[16] = {
        cos(rad), -sin(rad), 0.0, 0.0,
        sin(rad),  cos(rad), 0.0, 0.0,
        0.0,       0.0,      1.0, 0.0,
        0.0,       0.0,      0.0, 1.0,
    };
    float Mrry[16] = {
         V/L, 0.0, A/L, 0.0,
         0.0, 1.0, 0.0, 0.0,
        -A/L, 0.0, V/L, 0.0,
         0.0, 0.0, 0.0, 1.0,
    };
    float Mrrx[16] = {
        1.0,  0.0, 0.0, 0.0,
        0.0,  C/V, B/V, 0.0,
        0.0, -B/V, C/V, 0.0,
        0.0,  0.0, 0.0, 1.0,
    };
    memcpy(m, Mrx, sizeof(Mrx));
    mat_multiply(m, Mry);
    mat_multiply(m, Mrz);
    mat_multiply(m, Mrry);
    mat_multiply(m, Mrrx);
}

void init_mat(float *m)
{
    static const float mat[] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    memcpy(m, &mat, sizeof(mat));
}

