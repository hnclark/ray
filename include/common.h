#ifndef COMMON
#define COMMON

#include <limits>

#define WINDOW_NAME "RayEngine"
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

#define PRINT_FPS_TIME 1

#define SIZE_MB 1000000

#define CLAMP(lower,x,upper) (std::max(lower, std::min(x, upper)))

#define ARRAY_INDEX(x,y,width) ((y * width) + x)

#define RAY_MISS std::numeric_limits<float>::max()
#define RAY_INVALID -1

enum AXIS{AXIS_X, AXIS_Y, AXIS_Z, AXIS_NUM};
enum FACE{FACE_X_MIN, FACE_X_PLUS, FACE_Y_MIN, FACE_Y_PLUS, FACE_Z_MIN, FACE_Z_PLUS, FACE_NONE};
#define AXIS_TO_FACE(axis) (axis * 2)
#define FACE_MIN_TO_PLUS(face) (++face)
#define FACE_PLUS_TO_MIN(face) (--face)

enum COLOR{COLOR_R, COLOR_G, COLOR_B, COLOR_NUM};
#define COLOR_MAX Vec3(1,1,1)

#define FLOAT_MARGIN_CLOSE 0.01
#define FLOAT_MARGIN_PRECISE 0.001
inline bool geqMargin (float x, float y, float margin = FLOAT_MARGIN_CLOSE) {
	return x + margin >= y;
}
inline bool leqMargin (float x, float y, float margin = FLOAT_MARGIN_CLOSE) {
	return x - margin <= y;
}
inline bool eqMargin (float x, float y, float margin = FLOAT_MARGIN_CLOSE) {
	return geqMargin(x, y, margin) && leqMargin(x, y, margin);
}

#endif
