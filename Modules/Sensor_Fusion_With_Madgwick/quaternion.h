#ifndef QUATERNION_H
#define QUATERNION_H

#include <cmath>

class Quaternion {
public:
    float w, x, y, z;

    Quaternion() : w(1), x(0), y(0), z(0) {}
    Quaternion(float _w, float _x, float _y, float _z) : w(_w), x(_x), y(_y), z(_z) {}

    Quaternion operator*(const Quaternion& q) const {
        return Quaternion(
            w * q.w - x * q.x - y * q.y - z * q.z,
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y - x * q.z + y * q.w + z * q.x,
            w * q.z + x * q.y - y * q.x + z * q.w
        );
    }

    Quaternion conjugate() const {
        return Quaternion(w, -x, -y, -z);
    }

    void normalize() {
        float norm = sqrt(w * w + x * x + y * y + z * z);
        if (norm > 0.0f) {
            float invNorm = 1.0f / norm;
            w *= invNorm;
            x *= invNorm;
            y *= invNorm;
            z *= invNorm;
        }
    }

    void rotateVector(float& vx, float& vy, float& vz) const {
        Quaternion vecQuat(0, vx, vy, vz);
        Quaternion result = (*this) * vecQuat * this->conjugate();
        vx = result.x;
        vy = result.y;
        vz = result.z;
    }

    static Quaternion fromEuler(float roll, float pitch, float yaw) {
        float cy = cos(yaw * 0.5);
        float sy = sin(yaw * 0.5);
        float cp = cos(pitch * 0.5);
        float sp = sin(pitch * 0.5);
        float cr = cos(roll * 0.5);
        float sr = sin(roll * 0.5);
        
        return Quaternion(
            cr * cp * cy + sr * sp * sy,
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy
        );
    }
};

#endif
