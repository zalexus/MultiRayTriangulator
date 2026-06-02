#include "MultiRayTriangulator.h"
#include <algorithm>
#include <cstring>
#include <DDImage/AxisOp.h>

MultiRayTriangulator::MultiRayTriangulator(Node* node) : DD::Image::Iop(node)
{
    _startFrame = 1.0;
    _endFrame = 100.0;

    _trackPos[0] = 1024.0f;
    _trackPos[1] = 578.0f;

    _resultPos[0] = 0.0;
    _resultPos[1] = 0.0;
    _resultPos[2] = 0.0;

    _drawCross = true;
    _useKeysOnly = true; // ИСПРАВЛЕНО: По умолчанию режим расчета по ключам включен (ON)
    _crossScale = 1.0;
    _crossThickness = 1.0;

    _crossColor[0] = 1.0f;
    _crossColor[1] = 0.0f;
    _crossColor[2] = 0.0f;
    _crossColor[3] = 1.0f;
}

int MultiRayTriangulator::minimum_inputs() const { return 1; }
int MultiRayTriangulator::maximum_inputs() const { return 2; }

DD::Image::Op* MultiRayTriangulator::default_input(int n) const {
    if (n == 1) return nullptr;
    return DD::Image::Iop::default_input(n);
}

bool MultiRayTriangulator::test_input(int n, Op* op) const {
    if (op == default_input(n)) return true;
    if (n == 1) return dynamic_cast<DD::Image::AxisOp*>(op) != nullptr;
    if (n == 0) return dynamic_cast<DD::Image::Iop*>(op) != nullptr;
    return false;
}

const char* MultiRayTriangulator::input_label(int n, char*) const {
    switch (n) {
        case 1: return "Camera";
        case 0: return "Image";
        default: return "";
    }
}

void MultiRayTriangulator::_validate(bool for_real) {
    DD::Image::Op* opCamera = input_op(1);
    DD::Image::Op* opImgNode = input_op(0);

    if (!opImgNode || opImgNode == default_input(0)) {
        info_ = DD::Image::IopInfo();
        return;
    }

    DD::Image::Iop* img = dynamic_cast<DD::Image::Iop*>(opImgNode);
    if (!img) {
        info_ = DD::Image::IopInfo();
        return;
    }

    img->validate(for_real);
    info_ = img->info();

    if (opCamera && opCamera != default_input(1)) {
        DD::Image::AxisOp* cam = dynamic_cast<DD::Image::AxisOp*>(opCamera);
        if (cam) {
            cam->validate(for_real);
        }
    }
}

void MultiRayTriangulator::_request(int x, int y, int r, int t, DD::Image::ChannelMask channels, int count) {
    if (input(0)) {
        input(0)->request(x, y, r, t, channels, count);
    }
}

void MultiRayTriangulator::engine(int y, int x, int r, DD::Image::ChannelMask channels, DD::Image::Row& row) {
    if (input(0)) {
        input(0)->get(y, x, r, channels, row);
        return;
    }
    row.erase(channels);
}

extern "C" {
    static DD::Image::Iop* build(Node* node) {
        return new MultiRayTriangulator(node);
    }
    const DD::Image::Iop::Description MultiRayTriangulator_description(
        "MultiRayTriangulator",
        "Custom/MultiRayTriangulator",
        build
    );
}
