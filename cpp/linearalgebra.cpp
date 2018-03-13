#include "linearalgebra.h"

#include <cmath>
#include <cstring>
#include <cstdio>

float LinearAlgebra::dot3(const float *vec1, const float *vec2)
{
    return vec1[0]*vec2[0] + vec1[1]*vec2[1] + vec1[2]*vec2[2];
}

void LinearAlgebra::cross3(const float *vec1, const float *vec2, float* vec)
{
    vec[0] = -vec2[1]*vec1[2] + vec1[1]*vec2[2];
    vec[1] = vec2[0]*vec1[2] - vec1[0]*vec2[2];
    vec[2] = -vec2[0]*vec1[1] + vec1[0]*vec2[1];
}

float LinearAlgebra::norm3(const float *vec)
{
    return std::sqrt(dot3(vec, vec));
}

void LinearAlgebra::normalize3(float *vec)
{
    float len = norm3(vec);
    vec[0] /= len;
    vec[1] /= len;
    vec[2] /= len;
}

void LinearAlgebra::normalized3(const float *vec, float *vec_n)
{
    float len = norm3(vec);
    vec_n[0] = vec[0] / len;
    vec_n[1] = vec[1] / len;
    vec_n[2] = vec[2] / len;
}

void LinearAlgebra::vec3FromTo(const float *vec1, const float *vec2, float *vec)
{
    vec[0] = vec2[0] - vec1[0];
    vec[1] = vec2[1] - vec1[1];
    vec[2] = vec2[2] - vec1[2];
}

void LinearAlgebra::rotateByAxisAngle(const float *ax, float angle, float *vec)
{
    float c = std::cos(angle);
    float s = std::sin(angle);

    float matR[9] = {c + ax[0] * ax[0] * (1-c), ax[1] * ax[0] * (1-c) + ax[2] * s, ax[2] * ax[0] * (1-c) - ax[1] * s,
        ax[0] * ax[1] * (1-c) - ax[2] * s, c + ax[1] * ax[1] * (1-c), ax[2] * ax[1] * (1-c) + ax[0] * s,
        ax[0] * ax[2] * (1-c) + ax[1] * s, ax[1] * ax[2] * (1-c) - ax[0] * s, c + ax[2] * ax[2] * (1-c)};
    float res[3] = {0.0f};

    DummyMatrix v(vec, 1, 3);
    DummyMatrix R(matR, 3, 3);
    DummyMatrix vn(res, 1, 3);
    DummyMatrix::multiply(v, R, vn);

    memcpy(vec, res, 3*sizeof(float));
}

void LinearAlgebra::rotateByAxisAngle(const float *ax, float angle, int num, float *vec)
{
    float c = std::cos(angle);
    float s = std::sin(angle);

    float matR[9] = {c + ax[0] * ax[0] * (1-c), ax[1] * ax[0] * (1-c) + ax[2] * s, ax[2] * ax[0] * (1-c) - ax[1] * s,
        ax[0] * ax[1] * (1-c) - ax[2] * s, c + ax[1] * ax[1] * (1-c), ax[2] * ax[1] * (1-c) + ax[0] * s,
        ax[0] * ax[2] * (1-c) + ax[1] * s, ax[1] * ax[2] * (1-c) - ax[0] * s, c + ax[2] * ax[2] * (1-c)};
    float *res = new float[num * 3];

    DummyMatrix v(vec, num, 3);
    DummyMatrix R(matR, 3, 3);
    DummyMatrix vn(res, num, 3);
    DummyMatrix::multiply(v, R, vn);

    memcpy(vec, res, 3*num*sizeof(float));

    delete[] res;
}

void LinearAlgebra::rotateZ(float lon, float lat, float roll, int num, float *vec)
{
    float ax[9] = {-std::sin(lon), std::cos(lon), 0.0f, 
        -std::cos(lon) * std::sin(lat), -std::sin(lon) * std::sin(lat), std::cos(lat),
        std::cos(lat) * std::cos(lon), std::cos(lat) * std::sin(lon), std::sin(lat)};
    float d[3] = {std::cos(lat) * std::cos(lon), std::cos(lat) * std::sin(lon), std::sin(lat)};
    rotateByAxisAngle(d, roll, 3, ax);

    DummyMatrix matR(ax, 3, 3);
    matR.transpose();

    DummyMatrix inputVec(vec, num, 3);
    float *res = new float[num * 3];
    DummyMatrix resVec(res, num, 3);

    DummyMatrix::multiply(inputVec, matR, resVec);
    memcpy(vec, res, 3*num*sizeof(float));

    delete[] res;
}

void LinearAlgebra::rotateZBack(float lon, float lat, float roll, int num, float *vec)
{
    float ax[9] = {-std::sin(lon), std::cos(lon), 0.0f, 
        -std::cos(lon) * std::sin(lat), -std::sin(lon) * std::sin(lat), std::cos(lat),
        std::cos(lat) * std::cos(lon), std::cos(lat) * std::sin(lon), std::sin(lat)};
    float d[3] = {std::cos(lat) * std::cos(lon), std::cos(lat) * std::sin(lon), std::sin(lat)};
    rotateByAxisAngle(d, roll, 3, ax);

    DummyMatrix matR(ax, 3, 3);

    DummyMatrix inputVec(vec, num, 3);
    float *res = new float[num * 3];
    DummyMatrix resVec(res, num, 3);

    DummyMatrix::multiply(inputVec, matR, resVec);
    memcpy(vec, res, 3*num*sizeof(float));

    delete[] res;
}


DummyMatrix::DummyMatrix(float *data, int row, int col) :
    rowNum(row), colNum(col), data(data)
{ }

int DummyMatrix::multiply(const DummyMatrix &a, const DummyMatrix &b, DummyMatrix &res)
{
    if (a.colNum != b.rowNum)
    {
        return -1;
    }

    for (int r = 0; r < a.rowNum; r++)
    {
        for (int c = 0; c < b.colNum; c++)
        {
            float sum = 0.0f;
            for (int k = 0; k < a.colNum; k++)
            {
                sum += a.data[r*a.colNum + k] * b.data[k*b.colNum + c];
            }
            res.data[r*res.colNum + c] = sum;
        }
    }
    return 0;
}

void DummyMatrix::transpose()
{
    for (int r = 0; r < rowNum; r++)
    {
        for (int c = r+1; c < colNum; c++)
        {
            float tmp;
            tmp = data[r*colNum + c];
            data[r*colNum + c] = data[c*colNum +r];
            data[c*colNum +r] = tmp;
        }
    }
}