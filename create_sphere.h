#pragma once // Гарантирует, что этот заголовочный файл будет включен компилятором только один раз

#include <string> // Подключаем стандартный класс строк C++ для хранения текста Python-скрипта

// Объявляем глобальную строковую константу, хранящую чистый Python-скрипт генерации 3D-сферы
const std::string SPHERE_SCRIPT = R"(
n = nuke.thisNode() # Хватаем указатель на экземпляр нашей текущей C++ ноды MultiRayTriangulator

# Создаем контейнерную ноду Group, в которой будет жить вся 3D-геометрия
g = nuke.createNode('Group', inpanel=False)
g.knob('tile_color').setValue(0xFF0000FF) # Красим плашку ноды группы на графе в сочный красный цвет
g.knob('label').setValue('Point Container') # Задаем понятное текстовое описание под именем ноды

# Добавляем на саму группу трехкомпонентный кноб для трансляции 3D-координат
xyz_k = nuke.XYZ_Knob('target_pos', 'Point Position')
g.addKnob(xyz_k)
# Связываем координату группы с МНК-результатом C++ ноды через динамическое выражение
g.knob('target_pos').setExpression(n.name() + '.result_pos')

# Добавляем ползунок масштаба для управления физическим радиусом сферы
rad_k = nuke.Double_Knob('sphere_radius', 'Sphere Radius')
rad_k.setRange(0.01, 2.0) # Ограничиваем ползунок радиуса вменяемыми границами (от 0.01 до 2.0)
g.addKnob(rad_k)
g.knob('sphere_radius').setValue(0.1) # Выставляем дефолтный радиус сферы равным 10 сантиметрам (0.1)

# Добавляем кноб выбора цвета, чтобы художники могли перекрашивать геометрию прямо на группе
col_k = nuke.Color_Knob('sphere_color', 'Sphere Color')
g.addKnob(col_k)
# По умолчанию привязываем цвет сферы к выбранному цвету оверлея-крестика C++ ноды
g.knob('sphere_color').setExpression(n.name() + '.cross_color')

g.begin() # Виртуально погружаемся внутрь созданной группы, чтобы наполнить её 3D-нодами
for node in nuke.allNodes(): # Сканируем внутреннее空间 группы на наличие дефолтных нод Nuke
    nuke.delete(node) # Тотально зачищаем внутренности (удаляем автосозданный Input), подготавливая чистый холст

# Создаем плосную ноду Constant, которая будет выступать в роли сплошной заливки цвета текстуры
c = nuke.createNode('Constant', inpanel=False)
# Линкуем цвет генератора Constant к управляющему кнобу sphere_color на панели нашей группы
c.knob('color').setExpression('parent.sphere_color', channel=0)
c.knob('color').setExpression('parent.sphere_color', channel=1)
c.knob('color').setExpression('parent.sphere_color', channel=2)

# Создаем нативную геометрическую 3D-ноду сферы Sphere
s = nuke.createNode('Sphere', inpanel=False)
s.setInput(0, c) # Подключаем нашу константу в нулевой слот (текстура) для закраски полигонов сферы
s.knob('gl_color').setValue(0xFF0000FF) # Задаем каркасной сетке сферы в OpenGL вьювере базовый красный оттенок

# Намертво связываем физическую трансформацию сферы с кнобами управления на панели группы
s.knob('translate').setExpression('parent.target_pos')
s.knob('uniform_scale').setExpression('parent.sphere_radius')

# Создаем выходную ноду Output, чтобы группа могла транслировать 3D-геометрию наружу в общий граф
out = nuke.createNode('Output', inpanel=False)
out.setInput(0, s) # Подаем отрендеренную и покрашенную сферу на вход выходного шлюза
g.end() # Всплываем из внутренностей группы обратно в основное пространство скрипта Nuke
)";
