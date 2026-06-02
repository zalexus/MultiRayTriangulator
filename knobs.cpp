#include "MultiRayTriangulator.h"

void MultiRayTriangulator::knobs(DD::Image::Knob_Callback f)
{
    // 1. Чекбокс включения крестика (с новой строки)
    DD::Image::Bool_knob(f, &_drawCross, "draw_cross", "Draw Center Cross");
    if (f.makeKnobs()) {
        if (DD::Image::Knob* drawK = knob("draw_cross")) {
            drawK->set_flag(DD::Image::Knob::STARTLINE);
        }
    }

    // 2. Позиция трекера (с новой строки)
    DD::Image::XY_knob(f, _trackPos, "screen_pos", "Internal Track Position");
    if (f.makeKnobs()) {
        if (DD::Image::Knob* trackK = knob("screen_pos")) {
            trackK->set_flag(DD::Image::Knob::STARTLINE);
        }
    }

    // 3. Слайдер масштаба крестика (с новой строки)
    DD::Image::Double_knob(f, &_crossScale, "cross_scale", "Cross Scale Multiplier");
    if (f.makeKnobs()) {
        if (DD::Image::Knob* scaleK = knob("cross_scale")) {
            scaleK->set_flag(DD::Image::Knob::STARTLINE);
            scaleK->set_range(0.0, 10.0);
        }
    }

    // 4. Цвет крестика (с новой строки)
    DD::Image::Color_knob(f, _crossColor, "cross_color", "Cross Color");
    if (f.makeKnobs()) {
        if (DD::Image::Knob* colorK = knob("cross_color")) {
            colorK->set_flag(DD::Image::Knob::STARTLINE);
        }
    }

    // 5. Слайдер толщины линий крестика (с новой строки)
    DD::Image::Double_knob(f, &_crossThickness, "cross_thickness", "Cross Line Thickness");
    if (f.makeKnobs()) {
        if (DD::Image::Knob* thickK = knob("cross_thickness")) {
            thickK->set_flag(DD::Image::Knob::STARTLINE);
            thickK->set_range(1.0, 10.0);
        }
    }

    // Разделитель диапазона кадров (с новой строки)
    DD::Image::Divider(f, "Frame Range Settings");

    // 6. Чекбокс режима триангуляции по ключам (с новой строки)
    DD::Image::Bool_knob(f, &_useKeysOnly, "use_keys_only", "Calculate from Keyframes Only");
    if (f.makeKnobs()) {
        if (DD::Image::Knob* keysK = knob("use_keys_only")) {
            keysK->set_flag(DD::Image::Knob::STARTLINE);
        }
    }

    // 7. Поле начального кадра (с новой строки)
    DD::Image::Double_knob(f, &_startFrame, "start_frame", "Start Frame");
    if (f.makeKnobs()) {
        if (DD::Image::Knob* startK = knob("start_frame")) {
            startK->set_flag(DD::Image::Knob::STARTLINE);
            if (!startK->hasExpression()) startK->set_expression("[root.first_frame]", 0, -1);
        }
    }

    // 8. Поле конечного кадра (с новой строки)
    DD::Image::Double_knob(f, &_endFrame, "end_frame", "End Frame");
    if (f.makeKnobs()) {
        if (DD::Image::Knob* endK = knob("end_frame")) {
            endK->set_flag(DD::Image::Knob::STARTLINE);
            if (!endK->hasExpression()) endK->set_expression("[root.last_frame]", 0, -1);
        }
    }

    // Разделитель перед блоком результатов (с новой строки)
    DD::Image::Divider(f, "Triangulation Result");

    // 9. Результат МНК (с новой строки)
    DD::Image::XYZ_knob(f, _resultPos, "result_pos", "Point Position");
    if (f.makeKnobs()) {
        if (DD::Image::Knob* posK = knob("result_pos")) {
            posK->set_flag(DD::Image::Knob::STARTLINE);
            posK->set_flag(DD::Image::Knob::READ_ONLY);
        }
    }

    // ЖЕЛЕЗОБЕТОННО: Принудительный разделитель без текста, чтобы закрыть строку XYZ_knob
    DD::Image::Spacer(f, 10);

    const char* pythonScript =
        "n = nuke.thisNode()\n"
        "\n"
        "g = nuke.createNode('Group', inpanel=False)\n"
        "g.knob('tile_color').setValue(0xFF0000FF)\n"
        "g.knob('label').setValue('Point Container')\n"
        "\n"
        "xyz_k = nuke.XYZ_Knob('target_pos', 'Point Position')\n"
        "g.addKnob(xyz_k)\n"
        "\n"
        "g.knob('target_pos').setExpression(n.name() + '.result_pos')\n"
        "\n"
        "rad_k = nuke.Double_Knob('sphere_radius', 'Sphere Radius')\n"
        "rad_k.setRange(0.01, 2.0)\n"
        "g.addKnob(rad_k)\n"
        "g.knob('sphere_radius').setValue(0.1)\n"
        "\n"
        "g.begin()\n"
        "for node in nuke.allNodes():\n"
        "    nuke.delete(node)\n"
        "\n"
        "c = nuke.createNode('Constant', inpanel=False)\n"
        "c.knob('color').setValue([1.0, 0.0, 0.0, 1.0])\n"
        "\n"
        "s = nuke.createNode('Sphere', inpanel=False)\n"
        "s.setInput(0, c)\n"
        "s.knob('gl_color').setValue(0xFF0000FF)\n"
        "\n"
        "s.knob('translate').setExpression('parent.target_pos')\n"
        "s.knob('uniform_scale').setExpression('parent.sphere_radius')\n"
        "\n"
        "out = nuke.createNode('Output', inpanel=False)\n"
        "out.setInput(0, s)\n"
        "g.end()\n";

    // 10. Интерактивная кнопка создания сферы (теперь гарантированно на новой строке)
    DD::Image::PyScript_knob(f, pythonScript, "create_sphere_btn", "Create Linked Red Sphere Group");
    if (f.makeKnobs()) {
        if (DD::Image::Knob* btnK = knob("create_sphere_btn")) {
            btnK->set_flag(DD::Image::Knob::STARTLINE);
        }
    }
}
