#include "context.h"
#include "mymath.h"


namespace IceHalo {

namespace Math {


float dot3(const float *vec1, const float *vec2)
{
    return vec1[0]*vec2[0] + vec1[1]*vec2[1] + vec1[2]*vec2[2];
}

void cross3(const float *vec1, const float *vec2, float* vec)
{
    vec[0] = -vec2[1]*vec1[2] + vec1[1]*vec2[2];
    vec[1] = vec2[0]*vec1[2] - vec1[0]*vec2[2];
    vec[2] = -vec2[0]*vec1[1] + vec1[0]*vec2[1];
}

float norm3(const float *vec)
{
    return std::sqrt(dot3(vec, vec));
}

float diffNorm3(const float *vec1, const float *vec2)
{
    float v[3];
    vec3FromTo(vec1, vec2, v);
    return norm3(v);
}

void normalize3(float *vec)
{
    float len = norm3(vec);
    vec[0] /= len;
    vec[1] /= len;
    vec[2] /= len;
}

void normalized3(const float *vec, float *vec_n)
{
    float len = norm3(vec);
    vec_n[0] = vec[0] / len;
    vec_n[1] = vec[1] / len;
    vec_n[2] = vec[2] / len;
}

void vec3FromTo(const float *vec1, const float *vec2, float *vec)
{
    vec[0] = vec2[0] - vec1[0];
    vec[1] = vec2[1] - vec1[1];
    vec[2] = vec2[2] - vec1[2];
}


void rotateBase(const float *ax, float angle, float *vec)
{
    float c = std::cos(angle);
    float s = std::sin(angle);

    float matR[9] = {c + ax[0] * ax[0] * (1-c), ax[1] * ax[0] * (1-c) + ax[2] * s, ax[2] * ax[0] * (1-c) - ax[1] * s,
        ax[0] * ax[1] * (1-c) - ax[2] * s, c + ax[1] * ax[1] * (1-c), ax[2] * ax[1] * (1-c) + ax[0] * s,
        ax[0] * ax[2] * (1-c) + ax[1] * s, ax[1] * ax[2] * (1-c) - ax[0] * s, c + ax[2] * ax[2] * (1-c)};
    float res[9];

    DummyMatrix v(vec, 3, 3);
    DummyMatrix R(matR, 3, 3);
    DummyMatrix vn(res, 3, 3);
    DummyMatrix::multiply(v, R, vn);

    memcpy(vec, res, 9*sizeof(float));
}


void rotateZ(const float *lon_lat_roll, float *vec, uint64_t dataNum)
{
    using namespace std;
    float ax[9] = {-sin(lon_lat_roll[0]), cos(lon_lat_roll[0]), 0.0f,
                   -cos(lon_lat_roll[0]) * sin(lon_lat_roll[1]), -sin(lon_lat_roll[0]) * sin(lon_lat_roll[1]), cos(lon_lat_roll[1]),
                   cos(lon_lat_roll[1]) * cos(lon_lat_roll[0]), cos(lon_lat_roll[1]) * sin(lon_lat_roll[0]), sin(lon_lat_roll[1])};
    float d[3] = {cos(lon_lat_roll[1]) * cos(lon_lat_roll[0]), cos(lon_lat_roll[1]) * sin(lon_lat_roll[0]), sin(lon_lat_roll[1])};
    rotateBase(d, lon_lat_roll[2], ax);

    DummyMatrix matR(ax, 3, 3);
    matR.transpose();

    auto *res = new float[dataNum * 3];
    
    DummyMatrix resVec(res, dataNum, 3);
    DummyMatrix inputVec(vec, dataNum, 3);
    DummyMatrix::multiply(inputVec, matR, resVec);
    memcpy(vec, res, 3 * dataNum * sizeof(float));

    delete[] res;
}


void rotateZBack(const float *lon_lat_roll, float *vec, uint64_t dataNum)
{
    using namespace std;
    float ax[9] = {-sin(lon_lat_roll[0]), cos(lon_lat_roll[0]), 0.0f,
                   -cos(lon_lat_roll[0]) * sin(lon_lat_roll[1]), -sin(lon_lat_roll[0]) * sin(lon_lat_roll[1]), cos(lon_lat_roll[1]),
                   cos(lon_lat_roll[1]) * cos(lon_lat_roll[0]), cos(lon_lat_roll[1]) * sin(lon_lat_roll[0]), sin(lon_lat_roll[1])};
    float d[3] = {cos(lon_lat_roll[1]) * cos(lon_lat_roll[0]), cos(lon_lat_roll[1]) * sin(lon_lat_roll[0]), sin(lon_lat_roll[1])};
    rotateBase(d, lon_lat_roll[2], ax);

    DummyMatrix matR(ax, 3, 3);

    // float res[3] = { 0.0f };
    auto *res = new float[dataNum * 3];
    
    DummyMatrix resVec(res, dataNum, 3);
    DummyMatrix inputVec(vec, dataNum, 3);
    DummyMatrix::multiply(inputVec, matR, resVec);
    memcpy(vec, res, 3 * dataNum * sizeof(float));

    delete[] res;
}


DummyMatrix::DummyMatrix(float *data, uint64_t row, uint64_t col) :
    rowNum(row), colNum(col), data(data)
{ }

int DummyMatrix::multiply(const DummyMatrix &a, const DummyMatrix &b, DummyMatrix &res)
{
    if (a.colNum != b.rowNum) {
        return -1;
    }

    for (uint64_t r = 0; r < a.rowNum; r++) {
        for (uint64_t c = 0; c < b.colNum; c++) {
            float sum = 0.0f;
            for (uint64_t k = 0; k < a.colNum; k++) {
                sum += a.data[r*a.colNum + k] * b.data[k*b.colNum + c];
            }
            res.data[r*res.colNum + c] = sum;
        }
    }
    return 0;
}

void DummyMatrix::transpose()
{
    for (uint64_t r = 0; r < rowNum; r++) {
        for (uint64_t c = r+1; c < colNum; c++) {
            float tmp;
            tmp = data[r*colNum + c];
            data[r*colNum + c] = data[c*colNum +r];
            data[c*colNum +r] = tmp;
        }
    }
}




template <typename T>
Vec3<T>::Vec3(T x, T y, T z)
{
    _val[0] = x;
    _val[1] = y;
    _val[2] = z;
}

template <typename T>
Vec3<T>::Vec3(const T *data)
{
    _val[0] = data[0];
    _val[1] = data[1];
    _val[2] = data[2];
}

template <typename T>
Vec3<T>::Vec3(const Vec3<T> &v)
{
    _val[0] = v._val[0];
    _val[1] = v._val[1];
    _val[2] = v._val[2];
}


template <typename T>
const T* Vec3<T>::val() const
{
    return _val;
}

template <typename T>
void Vec3<T>::val(T x, T y, T z)
{
    _val[0] = x;
    _val[1] = y;
    _val[2] = z;
}

template <typename T>
void Vec3<T>::val(const T *data)
{
    _val[0] = data[0];
    _val[1] = data[1];
    _val[2] = data[2];
}

template <typename T>
T Vec3<T>::x() const
{
    return _val[0];
}

template <typename T>
T Vec3<T>::y() const
{
    return _val[1];
}

template <typename T>
T Vec3<T>::z() const
{
    return _val[2];
}

template <typename T>
void Vec3<T>::x(T x)
{
    _val[0] = x;
}

template <typename T>
void Vec3<T>::y(T y)
{
    _val[1] = y;
}

template <typename T>
void Vec3<T>::z(T z)
{
    _val[2] = z;
}

template <typename T>
Vec3<T> Vec3<T>::normalized()
{
    return Vec3<T>::normalized(*this);
}

template <typename T>
void Vec3<T>::normalize()
{
    Math::normalize3(_val);
}

template <typename T>
Vec3<T> Vec3<T>::normalized(const Vec3<T> &v)
{
    T data[3];
    Math::normalized3(v._val, data);
    return Vec3<T>(data);
}

template <typename T>
T Vec3<T>::dot(const Vec3<T> &v1, const Vec3<T> &v2)
{
    return Math::dot3(v1._val, v2._val);
}

template <typename T>
T Vec3<T>::norm(const Vec3<T> &v)
{
    return Math::norm3(v._val);
}

template <typename T>
Vec3<T> Vec3<T>::cross(const Vec3<T> &v1, const Vec3<T> &v2)
{
    T data[3];
    Math::cross3(v1._val, v2._val, data);
    return Vec3<T>(data);
}

template <typename T>
Vec3<T> Vec3<T>::fromVec(const Vec3<T> &v1, const Vec3<T> &v2)
{
    T data[3];
    Math::vec3FromTo(v1._val, v2._val, data);
    return Vec3<T>(data);
}

template class Vec3<float>;


TriangleIdx::TriangleIdx(int id1, int id2, int id3)
{
    _idx[0] = id1;
    _idx[1] = id2;
    _idx[2] = id3;
}

int TriangleIdx::id1() const
{
    return _idx[0];
}

int TriangleIdx::id2() const
{
    return _idx[1];
}

int TriangleIdx::id3() const
{
    return _idx[2];
}

const int * TriangleIdx::idx() const
{
    return &_idx[0];
}

}  // namespace Math


/*
    top vertex:
        2   1
    3           0
        4   5

    upper vertex:
        8   7
    9           6
        10  11

    lower vertex:
        14  13
    15          12
        16  17

    bottom vertex:
        20  19
    21          18
        22  23
*/


/*
    top vertex:
        2   1
    3           0
        4   5

    upper vertex:
        8   7
    9           6
        10  11

    lower vertex:
        14  13
    15          12
        16  17

    bottom vertex:
        20  19
    21          18
        22  23
*/


    namespace Math {

OrientationGenerator::OrientationGenerator() :
    axDist(Distribution::UNIFORM), axMean(0), axStd(0),
    rollDist(Distribution::UNIFORM), rollMean(0), rollStd(0)
{ }

OrientationGenerator::OrientationGenerator(Distribution axDist, float axMean, float axStd,
        Distribution rollDist, float rollMean, float rollStd) :
    axDist(axDist), axMean(axMean), axStd(axStd),
    rollDist(rollDist), rollMean(rollMean), rollStd(rollStd)
{
    unsigned int seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
    // unsigned int seed = 2345;
    generator.seed(seed);
}

void OrientationGenerator::fillData(const float *sunDir, int num, float *rayDir, float *mainAxRot)
{
    for (int i = 0; i < num; i++) {
        float lon, lat, roll;

        switch (axDist) {
            case Distribution::UNIFORM : {
                float v[3] = {gaussDistribution(generator),
                              gaussDistribution(generator),
                              gaussDistribution(generator)};
                Math::normalize3(v);
                lon = atan2(v[1], v[0]);
                lat = asin(v[2] / Math::norm3(v));
            }
                break;
            case Distribution::GAUSS :
                lon = uniformDistribution(generator) * 2 * Math::PI;
                lat = gaussDistribution(generator) * axStd;
                lat += axMean;
                if (lat > Math::PI / 2) {
                    lat = Math::PI - lat;
                }
                if (lat < -Math::PI / 2) {
                    lat = -Math::PI - lat;
                }
                break;
        }

        switch (rollDist) {
            case Distribution::GAUSS :
                roll = gaussDistribution(generator) * rollStd + rollMean;
                break;
            case Distribution::UNIFORM :
                roll = (uniformDistribution(generator) - 0.5f) * rollStd + rollMean;
                break;
        }

        mainAxRot[i*3+0] = lon;
        mainAxRot[i*3+1] = lat;
        mainAxRot[i*3+2] = roll;

        memcpy(rayDir+i*3, sunDir, 3*sizeof(float));
        Math::rotateZ(mainAxRot + i * 3, rayDir + i * 3);
    }
}

}   // namespace Math

}   // namespace IceHalo
