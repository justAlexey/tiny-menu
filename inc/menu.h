#pragma once

#include <stdint.h>

// Типы пунктов меню
typedef enum {
    MENU_SUBMENU,
    MENU_PARAMETER,
    MENU_INFO,
    MENU_FUNCTION
} MenuItemType;

// Типы параметров
typedef enum {
    PARAM_INT,
    PARAM_FLOAT,
    PARAM_CHAR,
    PARAM_BOOL
} ParameterType;

// Статус и ошибки работы меню
typedef enum {
    MENU_OK = 0,
    MENU_ERR_NULL_PTR,
    MENU_ERR_INVALID_TYPE,
    MENU_ERR_OUT_OF_RANGE,
    MENU_ERR_NOT_FOUND,
    MENU_ERR_NOT_IMPLEMENTED,
    MENU_ERR_UNKNOWN
} MenuStatus;

// Структура параметра
typedef struct {
    ParameterType type;
    union {
        int32_t *i_val;
        float *f_val;
        uint8_t *c_val;
        struct {
            uint8_t *val;
            uint8_t bit;
        } b_val;
    } value;
    int32_t min;
    int32_t max;
    float step;
} MenuParameter;

// Структура информационного пункта
typedef struct {
    const char *info;
} MenuInfo;

// Структура функции
typedef struct {
    void (*action)(void);
} MenuFunction;

union MenuData{
    struct {
        struct MenuItem *items;
        int count;
    } submenu;
    MenuParameter parameter;
    MenuInfo info;
    MenuFunction function;
};

typedef struct MenuItem {
    const char *name;
    MenuItemType type;
    struct MenuItem *parent;
    union MenuData data;
} MenuItem;

// Callback для вывода строки меню
typedef void (*MenuPrintLineCallback)(const char *str, uint32_t line);

// Регистрация callback-функций с возвратом статуса
MenuStatus menu_register_print_line_callback(MenuPrintLineCallback cb);

// Регистрация корневого меню
MenuStatus menu_register_root(MenuItem *root);

// Прототипы функций для навигации по меню
MenuStatus menu_navigate_up(void);
MenuStatus menu_navigate_down(void);
MenuStatus menu_enter(void);
MenuStatus menu_back(void);
