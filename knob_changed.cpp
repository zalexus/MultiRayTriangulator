#include "MultiRayTriangulator.h"
#include "ls_triangulation.h"
#include <DDImage/AxisOp.h>
#include <set>
#include <algorithm>
#include <cmath>

int MultiRayTriangulator::knob_changed(DD::Image::Knob* k)
{
    static bool _isLooping = false;
    if (_isLooping) return 1;

    if (!k) return DD::Image::Iop::knob_changed(k);

    if (k->is("draw_cross") || k->is("cross_scale") || k->is("cross_color") || k->is("cross_thickness")) {
        node_redraw();
        return 1;
    }

    // ДОБАВЛЕНО: Если переключили галочку "Calculate from Keyframes Only", запускаем полный перерасчет
    if (!k->is("screen_pos") && !k->is("start_frame") && !k->is("end_frame") && !k->is("use_keys_only")) {
        return DD::Image::Iop::knob_changed(k);
    }

    DD::Image::Op* opImgNode = input_op(0);
    DD::Image::Op* opCamera  = input_op(1);

    if (opImgNode == default_input(0)) opImgNode = nullptr;
    if (opCamera == default_input(1)) opCamera = nullptr;

    DD::Image::Iop* opImage = opImgNode ? dynamic_cast<DD::Image::Iop*>(opImgNode) : nullptr;
    DD::Image::AxisOp* cam   = opCamera ? dynamic_cast<DD::Image::AxisOp*>(opCamera) : nullptr;
    DD::Image::Op* rOp       = this->rootOp();

    if (!cam || !opImage || !rOp) return 1;

    DD::Image::Knob* focalKnob     = cam->knob("focal");
    DD::Image::Knob* hapertureKnob = cam->knob("haperture");
    DD::Image::Knob* vapertureKnob = cam->knob("vaperture");
    DD::Image::Knob* xyKnob         = knob("screen_pos");

    if (!focalKnob || !hapertureKnob || !xyKnob) return 1;

    _isLooping = true;

    bool hasKeys = false;
    std::set<double> allTimes;
    if (xyKnob->is_animated()) {
        xyKnob->animationTimes(allTimes);
        if (!allTimes.empty()) {
            hasKeys = true;
            _startFrame = std::floor(*allTimes.begin());
            _endFrame = std::ceil(*allTimes.rbegin());

            DD::Image::Knob* startK = knob("start_frame");
            DD::Image::Knob* endK = knob("end_frame");
            if (startK) startK->set_value(_startFrame);
            if (endK) endK->set_value(_endFrame);
        }
    }

    if (!hasKeys) {
        double f_start = 1.0; double f_end = 100.0;
        DD::Image::Knob* rFirst = rOp->knob("first_frame");
        DD::Image::Knob* rLast = rOp->knob("last_frame");
        if (rFirst) f_start = rFirst->get_value();
        if (rLast) f_end = rLast->get_value();

        _startFrame = f_start; _endFrame = f_end;
        DD::Image::Knob* startK = knob("start_frame");
        DD::Image::Knob* endK = knob("end_frame");
        if (startK) startK->set_value(_startFrame);
        if (endK) endK->set_value(_endFrame);
    }

    double imgW = (double)opImage->info().w();
    double imgH = (double)opImage->info().h();
    if (imgW <= 0.0 || imgH <= 0.0) { imgW = 2048.0; imgH = 1156.0; }

    std::vector<FrameRayData> gatheredFrames;

    // ЖЕЛЕЗОБЕТОННО: Собираем список кадров для обхода в зависимости от выбранного режима
    std::vector<double> framesToProcess;

    if (_useKeysOnly) {
        if (hasKeys) {
            // Если режим "только ключи" и ключи есть, переносим их из std::set в наш список
            for (double t : allTimes) {
                framesToProcess.push_back(t);
            }
        } else {
            // Если ключей нет вообще, берем в качестве единственного кадра текущий кадр ползунка Nuke
            framesToProcess.push_back(this->outputContext().frame());
        }
    } else {
        // Старый стандартный режим: заполняем массив последовательно от start до end фрейма
        int start = (int)_startFrame;
        int end = (int)_endFrame;
        if (start > end) std::swap(start, end);
        for (int f = start; f <= end; ++f) {
            framesToProcess.push_back((double)f);
        }
    }

    // Основной цикл генерации лучей по подготовленному списку кадров
    for (double f : framesToProcess) {
        DD::Image::OutputContext localContext;
        localContext.setFrame(f);

        DD::Image::Matrix4 camMatrix;
        cam->matrixAt(localContext, camMatrix);

        double focal = focalKnob->get_value_at(f, 0);
        double hap = hapertureKnob->get_value_at(f, 0);
        if (focal <= 0.0 || hap <= 0.0) continue;

        double vap = vapertureKnob ? vapertureKnob->get_value_at(f, 0) : (hap * imgH) / imgW;

        double px = xyKnob->get_value_at(f, 0);
        double py = xyKnob->get_value_at(f, 1);

        double cx = imgW / 2.0; double cy = imgH / 2.0;

        double localX = (px - cx) * hap / (imgW * focal);
        double localY = (py - cy) * vap / (imgH * focal);
        double localZ = -1.0;

        DD::Image::Vector4 localDir4((float)localX, (float)localY, (float)localZ, 0.0f);
        DD::Image::Vector4 worldDir4 = camMatrix * localDir4;

        FrameRayData frameData;
        frameData.cameraOrigin.x = camMatrix.a03;
        frameData.cameraOrigin.y = camMatrix.a13;
        frameData.cameraOrigin.z = camMatrix.a23;

        double wlen = std::sqrt(worldDir4.x * worldDir4.x + worldDir4.y * worldDir4.y + worldDir4.z * worldDir4.z);
        if (wlen < 1e-6) continue;

        frameData.worldDir.x = worldDir4.x / wlen;
        frameData.worldDir.y = worldDir4.y / wlen;
        frameData.worldDir.z = worldDir4.z / wlen;

        gatheredFrames.push_back(frameData);
    }

    if (gatheredFrames.empty()) {
        _resultPos[0] = 0.0; _resultPos[1] = 0.0; _resultPos[2] = 0.0;
    } else {
        if (!computeMultiRayIntersection(gatheredFrames, _resultPos)) {
            _resultPos[0] = 0.0; _resultPos[1] = 0.0; _resultPos[2] = 0.0;
        }
    }

    DD::Image::Knob* posK = knob("result_pos");
    if (posK) {
        posK->set_value(_resultPos[0], 0);
        posK->set_value(_resultPos[1], 1);
        posK->set_value(_resultPos[2], 2);
    }

    DD::Image::Knob* startK = knob("start_frame");
    DD::Image::Knob* endK = knob("end_frame");
    if (startK) startK->updateWidgets();
    if (endK) endK->updateWidgets();
    if (posK) posK->updateWidgets();

    node_redraw();
    _isLooping = false;
    return 1;
}
