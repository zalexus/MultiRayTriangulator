// Подключаем заголовочный файл класса, чтобы компилятор знал структуру нашей ноды
#include "MultiRayTriangulator.h"

// Реализация виртуальной функции knobs, которая вызывается Nuke для отрисовки интерфейса свойств ноды
void MultiRayTriangulator::knobs(DD::Image::Knob_Callback f)
{
    // === БЛОК ПАРАМЕТРОВ ОВЕРЛЕЯ (ВЕРХНЯЯ ПОЗИЦИЯ) ===

    // Создаем логический чекбокс для включения/выключения отображения красного крестика во вьювере
    DD::Image::Bool_knob(f, &_drawCross, "draw_cross", "Draw Center Cross");
    // Если Nuke сейчас генерирует интерфейс ноды (а не сохраняет скрипт), настраиваем свойства кноба
    if (f.makeKnobs()) {
        // Ищем созданный кноб по его уникальному внутреннему имени
        if (DD::Image::Knob* drawK = knob("draw_cross")) {
            // Принудительно заставляем чекбокс встать на САМУЮ ПЕРВУЮ строчку панели с самого начала
            drawK->set_flag(DD::Image::Knob::STARTLINE);
        }
    }

    // Создаем интерактивное двумерное поле (X и Y) для хранения экранных координат трекера
    DD::Image::XY_knob(f, _trackPos, "screen_pos", "Internal Track Position");
    // Проверяем фазу генерации интерфейса
    if (f.makeKnobs()) {
        // Ищем кноб по внутреннему имени "screen_pos"
        if (DD::Image::Knob* trackK = knob("screen_pos")) {
            // Принудительно отправляем двумерный трекер на новую строку интерфейса
            trackK->set_flag(DD::Image::Knob::STARTLINE);
        }
    }

    // Создаем числовое поле типа double для управления масштабом (радиусом) крестика
    DD::Image::Double_knob(f, &_crossScale, "cross_scale", "Cross Scale Multiplier");
    // Проверяем фазу генерации интерфейса
    if (f.makeKnobs()) {
        // Ищем кноб масштаба
        if (DD::Image::Knob* scaleK = knob("cross_scale")) {
            // Принудительно переносим слайдер на новую строку
            scaleK->set_flag(DD::Image::Knob::STARTLINE);
            // Задаем ползунку жесткий диапазон от 0.0 до 10.0 согласно требованиям
            scaleK->set_range(0.0, 10.0);
        }
    }

    // Создаем кноб выбора цвета (поддерживает RGB/RGBA палитру Nuke)
    DD::Image::Color_knob(f, _crossColor, "cross_color", "Cross Color");
    // Проверяем фазу генерации интерфейса
    if (f.makeKnobs()) {
        // Ищем кноб цвета
        if (DD::Image::Knob* colorK = knob("cross_color")) {
            // Принудительно выстраиваем кноб цвета на новую строку панели
            colorK->set_flag(DD::Image::Knob::STARTLINE);
        }
    }

    // Создаем числовое поле типа double для регулирования толщины линий полигонального крестика
    DD::Image::Double_knob(f, &_crossThickness, "cross_thickness", "Cross Line Thickness");
    // Проверяем фазу генерации интерфейса
    if (f.makeKnobs()) {
        // Ищем кноб толщины
        if (DD::Image::Knob* thickK = knob("cross_thickness")) {
            // Принудительно выстраиваем параметр на новую строку
            thickK->set_flag(DD::Image::Knob::STARTLINE);
            // Задаем диапазон толщины линий в пикселях холста изображения от 1.0 до 10.0
            thickK->set_range(1.0, 10.0);
        }
    }


    // === БЛОК НАСТРОЕК ДИАПАЗОНА КАДРОВ ===

    // Создаем визуальную горизонтальную черту-разделитель с подписью для группировки параметров времени
    DD::Image::Divider(f, "Frame Range Settings");

    // Создаем логический чекбокс, переключающий расчет триангуляции (считать по всему диапазону или только по ключам)
    DD::Image::Bool_knob(f, &_useKeysOnly, "use_keys_only", "Calculate from Keyframes Only");
    // Проверяем фазу генерации интерфейса
    if (f.makeKnobs()) {
        // Ищем кноб режима фильтрации ключей
        if (DD::Image::Knob* keysK = knob("use_keys_only")) {
            // Принудительно переносим чекбокс на новую строку, отделяя его от разделителя
            keysK->set_flag(DD::Image::Knob::STARTLINE);
        }
    }

    // Создаем поле типа double для фиксации стартового кадра обработки
    DD::Image::Double_knob(f, &_startFrame, "start_frame", "Start Frame");
    // Проверяем фазу генерации интерфейса
    if (f.makeKnobs()) {
        // Ищем кноб начального кадра
        if (DD::Image::Knob* startK = knob("start_frame")) {
            // Принудительно переносим его на новую строку
            startK->set_flag(DD::Image::Knob::STARTLINE);
            // Если на кнобе нет пользовательского экспрешена, прописываем TCL-выражение начала глобального таймлайна проекта
            if (!startK->hasExpression()) startK->set_expression("[root.first_frame]", 0, -1);
        }
    }

    // Создаем поле типа double для фиксации конечного кадра обработки
    DD::Image::Double_knob(f, &_endFrame, "end_frame", "End Frame");
    // Проверяем фазу генерации интерфейса
    if (f.makeKnobs()) {
        // Ищем кноб конечного кадра
        if (DD::Image::Knob* endK = knob("end_frame")) {
            // Принудительно переносим его на новую строку
            endK->set_flag(DD::Image::Knob::STARTLINE);
            // Если экспрешена нет, прописываем TCL-выражение конца глобального таймлайна проекта
            if (!endK->hasExpression()) endK->set_expression("[root.last_frame]", 0, -1);
        }
    }


    // === БЛОК РЕЗУЛЬТАТОВ МНК-ВЫЧИСЛЕНИЙ ===

    // Создаем горизонтальный разделитель перед блоком вывода финальных мировых 3D-координат
    DD::Image::Divider(f, "Triangulation Result");

    // Создаем трехкомпонентный кноб (X, Y, Z) для отображения итоговой посчитанной 3D-позиции в мировом пространстве
    DD::Image::XYZ_knob(f, _resultPos, "result_pos", "Point Position");
    // Проверяем фазу генерации интерфейса
    if (f.makeKnobs()) {
        // Ищем кноб итогового результата
        if (DD::Image::Knob* posK = knob("result_pos")) {
            // Принудительно переносим трехкомпонентное поле на новую строку
            posK->set_flag(DD::Image::Knob::STARTLINE);
            // Блокируем кноб для ручного ввода художника (READ_ONLY), так как координаты рассчитываются строго алгоритмом МНК
            posK->set_flag(DD::Image::Knob::READ_ONLY);
        }
    }

    // Вставляем пустой невидимый спейсер высотой 10 пикселей, чтобы жестко разорвать контейнер XYZ_knob
    DD::Image::Spacer(f, 10);


    // === ВСТРОЕННЫЙ АВТОМАТИЗИРОВАННЫЙ PYTHON СКРИПТ (UX) ===

    // Описываем многострочный Python-код, который соберёт готовую 3D-геометрию внутри Nuke при клике на кнопку
    const char* pythonScript =
        "n = nuke.thisNode()\n" // Получаем указатель на текущую ноду нашего плагина MultiRayTriangulator
        "\n"
        "g = nuke.createNode('Group', inpanel=False)\n" // Создаем новую чистую ноду Group, не открывая её панель свойств
        "g.knob('tile_color').setValue(0xFF0000FF)\n" // Красим созданную группу в сочный красный цвет на графе нод (RGBA)
        "g.knob('label').setValue('Point Container')\n" // Задаем понятную текстовую метку на ноде
        "\n"
        "xyz_k = nuke.XYZ_Knob('target_pos', 'Point Position')\n" // Создаем кастомный XYZ кноб внутри свойств созданной группы
        "g.addKnob(xyz_k)\n" // Физически добавляем этот кноб на панель группы
        "\n"
        "g.knob('target_pos').setExpression(n.name() + '.result_pos')\n" // Линкуем его через экспрешен к результату МНК нашего плагина
        "\n"
        "rad_k = nuke.Double_Knob('sphere_radius', 'Sphere Radius')\n" // Создаем слайдер управления размером будущей 3D-сферы
        "rad_k.setRange(0.01, 2.0)\n" // Задаем слайдеру безопасный диапазон
        "g.addKnob(rad_k)\n" // Добавляем слайдер радиуса на панель группы
        "g.knob('sphere_radius').setValue(0.1)\n" // Ставим дефолтный размер 0.1
        "\n"
        "g.begin()\n" // Проваливаемся внутрь созданной группы, чтобы наполнить её 3D-нодами
        "for node in nuke.allNodes():\n" // Пробегаемся по всем нодам, которые Nuke мог автоматически создать внутри группы
        "    nuke.delete(node)\n" // Полностью удаляем их, гарантируя абсолютную чистоту контейнера
        "\n"
        "c = nuke.createNode('Constant', inpanel=False)\n" // Создаем ноду Constant для заливки цвета шейдера сферы
        "c.knob('color').setValue([1.0, 0.0, 0.0, 1.0])\n" // Ставим Constant чистый красный цвет
        "\n"
        "s = nuke.createNode('Sphere', inpanel=False)\n" // Создаем геометрическую 3D-ноду Sphere
        "s.setInput(0, c)\n" // Подключаем Constant в первый вход сферы (вход текстуры/карты цвета)
        "s.knob('gl_color').setValue(0xFF0000FF)\n" // Задаем цвет отображения сферы во вьювере в 3D режиме
        "\n"
        "s.knob('translate').setExpression('parent.target_pos')\n" // Привязываем позицию сферы к линкованному результату группы
        "s.knob('uniform_scale').setExpression('parent.sphere_radius')\n" // Привязываем масштаб сферы к слайдеру на группе
        "\n"
        "out = nuke.createNode('Output', inpanel=False)\n" // Создаем обязательную ноду Output для вывода геометрии из группы наружу
        "out.setInput(0, s)\n" // Подключаем сферу на вход Output
        "g.end()\n"; // Выходим из редактирования внутренностей группы обратно на основной граф

    // Создаем интерактивную кнопку, которая при нажатии выполнит наш Python-код
    DD::Image::PyScript_knob(f, pythonScript, "create_sphere_btn", "Create Linked Red Sphere Group");
    // Проверяем фазу генерации интерфейса
    if (f.makeKnobs()) {
        // Ищем созданную кнопку по имени
        if (DD::Image::Knob* btnK = knob("create_sphere_btn")) {
            // Благодаря нашему спейсеру выше и этому флагу, кнопка ГАРАНТИРОВАННО встает на новую строчку
            btnK->set_flag(DD::Image::Knob::STARTLINE);
        }
    }
}
