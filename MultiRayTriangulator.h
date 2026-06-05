#pragma once // Гарантирует компилятору защиту от повторного включения заголовочного файла в одном модуле трансляции
#include <DDImage/Iop.h> // Подключаем базовый класс Iop (Image Operation), фундамент для всех 2D/Image нод в Nuke NDK
#include <DDImage/Knobs.h> // Подключаем заголовок для регистрации и управления параметрами (knobs) в интерфейсе Nuke
#include <DDImage/Row.h> // Подключаем класс Row, необходимый для построчной обработки пиксельного буфера в движке Nuke

class MultiRayTriangulator : public DD::Image::Iop
{
protected: // Секция внутренних защищенных переменных, доступных членам класса и его будущим наследникам
    double _startFrame, _endFrame; // Переменные double для хранения границ диапазона кадров расчета триангуляции
    bool _drawCross;               // Переменная bool, управляющая видимостью интерактивного оверлея-крестика во вьювере
    bool _useKeysOnly;             // Переменная bool, переключающая плагин в режим генерации лучей строго по ключам анимации
    double _crossScale;            // Переменная double, хранящая пользовательский масштаб (множитель радиуса) оверлея
    double _crossThickness;        // Переменная double, задающая толщину линий полигонального крестика в пикселях холста
    float _crossColor[4];          // Нативный массив float из 4 элементов для хранения цвета крестика в формате RGBA (0.0-1.0)
    float _trackPos[2];            // Нативный массив float из 2 элементов под хранение двумерных XY-координат экранного трекера
    double _resultPos[3];          // Нативный массив double из 3 элементов для хранения финальных вычисленных 3D-координат XYZ

public: // Секция открытых виртуальных методов интерфейса NDK Nuke
    MultiRayTriangulator(Node* node); // Конструктор класса ноды, вызываемый Nuke при создании экземпляра плагина на холсте
    virtual ~MultiRayTriangulator() {} // Виртуальный деструктор класса, обеспечивающий корректную очистку памяти при удалении ноды

    int minimum_inputs() const override; // Переопределение минимального количества входов (ноде строго необходим 1 вход Image)
    int maximum_inputs() const override; // Переопределение maximalного количества входов (у ноды максимум 2 входа: Image и Camera)

    const char* Class() const override { return "MultiRayTriangulator"; } // Задает уникальное имя C++ класса нашей кастомной ноды
    const char* node_help() const override { return "Interactive Multi-ray Least Squares Triangulator node."; } // Справка о ноде

    bool test_input(int n, Op* op) const override; // Виртуальный метод NDK для жесткой фильтрации типов подключаемых нод на входы
    const char* input_label(int n, char* charbuf) const override; // Метод для текстового именования стрелочек входов на графе

    Op* default_input(int n) const override; // Задает дефолтное поведение для входов, если к ним ничего не подключено пользователем

    void knobs(DD::Image::Knob_Callback f) override; // Главный метод NDK для декларации и размещения ручек управления в панели свойств
    void build_handles(DD::Image::ViewerContext* ctx) override; // Метод для создания и регистрации оверлеев рисования во вьювере
    int knob_changed(DD::Image::Knob* k) override; // Колбэк-перехватчик событий изменения любых параметров пользователем в UI

    void _validate(bool for_real) override; // Важнейший метод расчета параметров графа, форматов данных и проверки каналов
    void _request(int x, int y, int r, int t, DD::Image::ChannelMask channels, int count) override; // Запрос области пикселей у входа
    void engine(int y, int x, int r, DD::Image::ChannelMask channels, DD::Image::Row& row) override; // Построчный движок рендера
};
