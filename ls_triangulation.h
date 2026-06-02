#pragma once
#include <vector>
#include <cmath>

struct Vector3D {
    double x, y, z;
};

struct FrameRayData {
    Vector3D cameraOrigin;
    Vector3D worldDir;
};

inline bool computeMultiRayIntersection(const std::vector<FrameRayData>& gatheredFrames, double* outPos) {
    outPos[0] = 0.0; outPos[1] = 0.0; outPos[2] = 0.0;
    if (gatheredFrames.empty()) return false;

    double M[3][3] = {{0.0}};
    double b[3] = {0.0};
    int validRaysCount = 0;

    for (const auto& frame : gatheredFrames) {
        double dx = frame.worldDir.x;
        double dy = frame.worldDir.y;
        double dz = frame.worldDir.z;

        double P[3][3];
        P[0][0] = 1.0 - dx * dx; P[0][1] = -dx * dy;     P[0][2] = -dx * dz;
        P[1][0] = -dy * dx;     P[1][1] = 1.0 - dy * dy; P[1][2] = -dy * dz;
        P[2][0] = -dz * dx;     P[2][1] = -dz * dy;     P[2][2] = 1.0 - dz * dz;

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                M[i][j] += P[i][j];
            }
            b[i] += P[i][0] * frame.cameraOrigin.x +
                    P[i][1] * frame.cameraOrigin.y +
                    P[i][2] * frame.cameraOrigin.z;
        }
        validRaysCount++;
    }

    if (validRaysCount < 1) return false;

    double det = M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1]) -
                 M[0][1] * (M[1][0] * M[2][2] - M[1][2] * M[2][0]) +
                 M[0][2] * (M[1][0] * M[2][1] - M[1][1] * M[2][0]);

    if (std::abs(det) <= 1e-7) return false;

    double M_inv[3][3];
    M_inv[0][0] =  (M[1][1] * M[2][2] - M[1][2] * M[2][1]) / det;
    M_inv[0][1] = -(M[0][1] * M[2][2] - M[0][2] * M[2][1]) / det;
    M_inv[0][2] =  (M[0][1] * M[1][2] - M[0][2] * M[1][1]) / det;

    M_inv[1][0] = -(M[1][0] * M[2][2] - M[1][2] * M[2][0]) / det;
    M_inv[1][1] =  (M[0][0] * M[2][2] - M[0][2] * M[2][0]) / det;
    M_inv[1][2] = -(M[0][0] * M[1][2] - M[0][2] * M[1][0]) / det;

    M_inv[2][0] =  (M[1][0] * M[2][1] - M[1][1] * M[2][0]) / det;
    M_inv[2][1] = -(M[0][0] * M[2][1] - M[0][1] * M[2][0]) / det;
    M_inv[2][2] =  (M[0][0] * M[1][1] - M[0][1] * M[1][0]) / det;

    double resX = M_inv[0][0] * b[0] + M_inv[0][1] * b[1] + M_inv[0][2] * b[2];
    double resY = M_inv[1][0] * b[0] + M_inv[1][1] * b[1] + M_inv[1][2] * b[2];
    double resZ = M_inv[2][0] * b[0] + M_inv[2][1] * b[1] + M_inv[2][2] * b[2];

    outPos[0] = resX;
    outPos[1] = resY;
    outPos[2] = resZ;

    return true;
}
