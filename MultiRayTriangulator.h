#pragma once
#include <DDImage/Iop.h>
#include <DDImage/Knobs.h>
#include <DDImage/Row.h>

class MultiRayTriangulator : public DD::Image::Iop
{
protected:
    double _startFrame, _endFrame;
    bool _drawCross;
    bool _useKeysOnly;    // ДОБАВЛЕНО: Режим расчета только по ключевым кадрам
    double _crossScale;
    double _crossThickness;
    float _crossColor[4];
    float _trackPos[2];
    double _resultPos[3];

public:
    MultiRayTriangulator(Node* node);
    virtual ~MultiRayTriangulator() {}

    int minimum_inputs() const override;
    int maximum_inputs() const override;

    const char* Class() const override { return "MultiRayTriangulator"; }
    const char* node_help() const override { return "Interactive Multi-ray Least Squares Triangulator node."; }

    bool test_input(int n, Op* op) const override;
    const char* input_label(int n, char* charbuf) const override;

    Op* default_input(int n) const override;

    void knobs(DD::Image::Knob_Callback f) override;
    void build_handles(DD::Image::ViewerContext* ctx) override;
    int knob_changed(DD::Image::Knob* k) override;

    void _validate(bool for_real) override;
    void _request(int x, int y, int r, int t, DD::Image::ChannelMask channels, int count) override;
    void engine(int y, int x, int r, DD::Image::ChannelMask channels, DD::Image::Row& row) override;
};
