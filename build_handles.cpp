#include "MultiRayTriangulator.h"
#include <algorithm>
#include <cmath>
#include <GL/gl.h>
#include <DDImage/CameraOp.h>
#include <DDImage/Matrix4.h>
#include <DDImage/Vector4.h>

// Рисуем пропорциональный крест строго в точке нативной 3D->2D проекции камеры Nuke
static void draw_proportional_center_cross(void* data, DD::Image::ViewerContext* ctx) {
    if (!ctx || !data) return;

    if (ctx->viewer_mode() == 0) {
        MultiRayTriangulator* node = static_cast<MultiRayTriangulator*>(data);

        // 1. Извлекаем текущие 3D координаты МНК из кноба
        double res[3] = {0.0, 0.0, 0.0};
        if (DD::Image::Knob* resK = node->knob("result_pos")) {
            res[0] = resK->get_value(0);
            res[1] = resK->get_value(1);
            res[2] = resK->get_value(2);
        }

        // Запрашиваем истинный формат картинки из info() ноды
        const DD::Image::IopInfo& info = node->info();
        double imgW = (double)info.w();
        double imgH = (double)info.h();
        if (imgW <= 0.0 || imgH <= 0.0) { imgW = 2048.0; imgH = 1156.0; }

        // По умолчанию выставляем крестик в центр кадра
        float crossX = (float)(imgW / 2.0);
        float crossY = (float)(imgH / 2.0);

        // 2. Рассчитываем нативную проекцию через CameraOp
        DD::Image::Op* opCamera = node->input_op(1);
        if (opCamera && opCamera != node->default_input(1)) {
            DD::Image::CameraOp* cam = dynamic_cast<DD::Image::CameraOp*>(opCamera);
            if (cam) {
                DD::Image::OutputContext viewCtx;
                viewCtx.setFrame(ctx->time());

                // Матрица трансформации камеры (Мир -> Камера)
                DD::Image::Matrix4 camMatrix;
                cam->matrixAt(viewCtx, camMatrix);
                DD::Image::Matrix4 viewMatrix = camMatrix.inverse();

                // Нативная матрица проекции объектива Nuke (Камера -> NDC)
                DD::Image::Matrix4 projMatrix = cam->projectionAt(viewCtx);

                // Полная MVP матрица
                DD::Image::Matrix4 mvpMatrix = projMatrix * viewMatrix;

                // Умножаем реальную 3D точку МНК на MVP матрицу
                DD::Image::Vector4 worldPt(res[0], res[1], res[2], 1.0f);
                DD::Image::Vector4 clipPt = mvpMatrix * worldPt;

                // Проверяем, что перспективное деление возможно
                if (std::abs(clipPt.w) > 1e-4) {
                    float ndcX = clipPt.x / clipPt.w;
                    float ndcY = clipPt.y / clipPt.w;

                    // Применяем выверенную тестом формулу перевода NDC -> Пиксели формата Nuke
                    crossX = (float)((ndcX + 1.0f) * 0.5f * imgW);
                    crossY = (float)((ndcY * 0.5f * imgW) + (imgH / 2.0));
                }
            }
        }

        // 3. Вычисляем длину перекрестия на основе ползунка cross_scale
        double scaleVal = 1.0;
        if (DD::Image::Knob* k = node->knob("cross_scale")) {
            scaleVal = k->get_value();
        }
        double longSide = std::max(imgW, imgH);
        float length = (float)(longSide * 0.02f * scaleVal);

        // 4. Извлекаем толщину линий из кноба (теперь это ЧЕСТНЫЕ пиксели изображения)
        float thick = 1.0f;
        if (DD::Image::Knob* thickK = node->knob("cross_thickness")) {
            thick = (float)thickK->get_value();
        }

        // Извлекаем цвет напрямую из ноды кноба cross_color
        float r = 1.0f, g = 0.0f, b = 0.0f, a = 1.0f;
        if (DD::Image::Knob* colorK = node->knob("cross_color")) {
            r = (float)colorK->get_value(0);
            g = (float)colorK->get_value(1);
            b = (float)colorK->get_value(2);
            a = (float)colorK->get_value(3);
        }

        // --- PIPELINE ОТРИСОВКИ ---
        glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT | GL_COLOR_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        glDisable(GL_BLEND);
        glDisable(GL_ALPHA_TEST);

        // Синхронизируем оверлей с текущим Zoom/Pan вьювера
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadMatrixf(ctx->model_matrix().array());

        // Применяем выбранный цвет
        glColor4f(r, g, b, a);

        // ЖЕЛЕЗОБЕТОННО: Вместо тонких линий OpenGL рисуем честные полигональные прямоугольники.
        // Теперь толщина («ширина» полигона) жестко привязана к пикселям картинки и идеально сжимается при Zoom Out.
        float halfThick = thick / 2.0f;
        float halfLength = length / 2.0f;

        // Горизонтальная линия креста (прямоугольник)
        glRectf(crossX - halfLength, crossY - halfThick, crossX + halfLength, crossY + halfThick);

        // Вертикальная линия креста (прямоугольник)
        glRectf(crossX - halfThick, crossY - halfLength, crossX + halfThick, crossY + halfLength);

        glPopMatrix();
        glPopAttrib();
    }
}

void MultiRayTriangulator::build_handles(DD::Image::ViewerContext* ctx) {
    DD::Image::Iop::build_handles(ctx);
    if (!ctx) return;

    DD::Image::Knob* trackK = knob("screen_pos");
    if (trackK) {
        trackK->build_handle(ctx);
    }

    if (this->_drawCross) {
        ctx->add_draw_handle(draw_proportional_center_cross, this, this->node());
    }
}
