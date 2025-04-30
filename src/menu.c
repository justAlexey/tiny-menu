#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../inc/menu.h"

#define MENU_MAX_LINE_LEN 32

// Статические указатели на callback-функции
static MenuPrintLineCallback print_line_cb = NULL;

// Статический указатель на корневое меню
static MenuItem *root_menu = NULL;

// Состояние меню
static MenuItem *current_menu = NULL;
static int current_index = 0;

// Объявление приватных функций
MenuStatus handler_submenu_navigate_up(void);
MenuStatus handler_submenu_navigate_down(void);
MenuStatus handler_parameter_navigate_up(void);
MenuStatus handler_parameter_navigate_down(void);

MenuStatus menu_register_print_line_callback(MenuPrintLineCallback cb) {
    if (cb == NULL) {
        return MENU_ERR_NULL_PTR;
    }
    print_line_cb = cb;
    return MENU_OK;
}

MenuStatus menu_register_root(MenuItem *root) {
    if (!root) return MENU_ERR_NULL_PTR;
    root_menu = root;
    current_menu = root;
    current_index = 0;
    return MENU_OK;
}

// Вспомогательная функция для форматирования строки пункта меню
static void menu_format_line(const MenuItem *item, int is_selected, char *line, size_t line_size) {
    const char *prefix = (is_selected && (current_menu->type!=MENU_PARAMETER)) ? "> " : "  ";
    const char *parameter_prefix = (is_selected && (current_menu->type == MENU_PARAMETER)) ? ">" : " ";
    const char *info_postfix = ((item->type == MENU_INFO) && strlen(item->data.info.info)>=10)? "..." : "   ";
    switch (item->type) {
        case MENU_PARAMETER:
            switch (item->data.parameter.type) {
                case PARAM_INT:
                    snprintf(line, line_size, "%s%-11s%s: %05ld", prefix, item->name, parameter_prefix, (long)*item->data.parameter.value.i_val);
                    break;
                case PARAM_FLOAT:
                    snprintf(line, line_size, "%s%-11s%s: %03.2f", prefix, item->name, parameter_prefix, *item->data.parameter.value.f_val);
                    break;
                case PARAM_BOOL:
                    snprintf(line, line_size, "%s%-11s%s: %s", prefix, item->name, parameter_prefix, (*item->data.parameter.value.b_val.val &&(1<<item->data.parameter.value.b_val.bit)) ? "ON" : "OFF");
                    break;
                default:
                    snprintf(line, line_size, "%s%-11s%s: ?", prefix, parameter_prefix, item->name);
            }
            break;
        case MENU_INFO:
            snprintf(line, line_size, "%s%-12.12s: %-10.10s%s", prefix, item->name, item->data.info.info, info_postfix);
            break;
        default:
            snprintf(line, line_size, "%s%-27s", prefix, item->name);
            break;
    }
}

// Вспомогательная функция для проверки валидности текущего меню
static int menu_is_valid_submenu(const MenuItem *menu) {
    return menu && menu->type == MENU_SUBMENU;
}

// Вспомогательная функция для отображения списка пунктов меню
static void menu_display_items(const MenuItem *items, int count, int selected_index, int total_lines) {
    char line[MENU_MAX_LINE_LEN];
    for (int i = 0; i < count; ++i) {
        menu_format_line(&items[i], i == selected_index, line, sizeof(line));
        print_line_cb(line, i);
    }
    for (int i = 0; i < total_lines - count; i++) {
        snprintf(line, sizeof(line), "  %-27s", " ");
        print_line_cb(line, i + count);
    }
}

void handler_display_submenu(void){
    int count = current_menu->data.submenu.count;
    int total_lines = current_menu->parent ? current_menu->parent->data.submenu.count : count;
    menu_display_items(current_menu->data.submenu.items, count, current_index, total_lines);
}

void handler_display_parameter(void){
    int count = current_menu->parent->data.submenu.count;
    int total_lines = count;
    menu_display_items(current_menu->parent->data.submenu.items, count, current_index, total_lines);
}

void handler_display_info(void){
    int total_lines = current_menu->parent ? current_menu->parent->data.submenu.count : 0;
    menu_display_items(current_menu->parent->data.submenu.items, 0, current_index, total_lines);
    print_line_cb(current_menu->parent->data.submenu.items[current_index].name, 0);
    print_line_cb(current_menu->data.info.info, 1);

}

static void menu_display(void) {
    if (!print_line_cb) return;
    switch(current_menu->type){
        case MENU_SUBMENU:
            handler_display_submenu();
            break;
        case MENU_PARAMETER:
            handler_display_parameter();
            break;
        case MENU_INFO:
            handler_display_info();
            break;
        default:
            break;
    }
}

MenuStatus handler_submenu_navigate_up(void){
    if (current_menu->type!=MENU_SUBMENU) return MENU_ERR_INVALID_TYPE;
    if (current_menu->data.submenu.count == 0) return MENU_ERR_OUT_OF_RANGE;
    MenuStatus status = MENU_ERR_OUT_OF_RANGE;
    if (current_index > 0) {
        current_index--;
        status = MENU_OK;
    }
    menu_display();
    return status;
}

MenuStatus handler_submenu_navigate_down(void){
    if (!menu_is_valid_submenu(current_menu)) return MENU_ERR_INVALID_TYPE;
    if (current_menu->data.submenu.count == 0) return MENU_ERR_OUT_OF_RANGE;
    MenuStatus status = MENU_ERR_OUT_OF_RANGE;
    if (current_index < current_menu->data.submenu.count - 1) {
        current_index++;
        status = MENU_OK;
    }
    menu_display();
    return status;
}

MenuStatus handler_parameter_navigate_up(void){
    MenuStatus status = MENU_ERR_OUT_OF_RANGE;
    if(current_menu->type != MENU_PARAMETER) return MENU_ERR_INVALID_TYPE;
    switch(current_menu->data.parameter.type){
        case PARAM_INT:
            int32_t temp = *current_menu->data.parameter.value.i_val;
            if(temp < current_menu->data.parameter.max){
                temp += current_menu->data.parameter.step;
                *current_menu->data.parameter.value.i_val = temp;
                status = MENU_OK;
                }
            break;
        case PARAM_BOOL:
            current_menu->data.parameter.value.b_val = !current_menu->data.parameter.value.b_val;
            status = MENU_OK;
            break;
        case PARAM_FLOAT:
            break;
        default:
            break;
    }
    menu_display();
    return status;
}

MenuStatus handler_parameter_navigate_down(void){
    MenuStatus status = MENU_ERR_OUT_OF_RANGE;
    if(current_menu->type != MENU_PARAMETER) return MENU_ERR_INVALID_TYPE;
    switch(current_menu->data.parameter.type){
        case PARAM_INT:
            if(current_menu->data.parameter.value.i_val > current_menu->data.parameter.min){
                current_menu->data.parameter.value.i_val -= current_menu->data.parameter.step;
                status = MENU_OK;
            }
            break;
        case PARAM_BOOL:
            current_menu->data.parameter.value.b_val = !current_menu->data.parameter.value.b_val;
            status = MENU_OK;
            break;
        case PARAM_FLOAT:
            // Добавьте обработку для float при необходимости
            break;
    }
    menu_display();
    return status;
}

MenuStatus menu_navigate_up(void) {
    switch(current_menu->type){
        case MENU_SUBMENU:
            return handler_submenu_navigate_up();
        case MENU_PARAMETER:
            return handler_parameter_navigate_up();
        default:
            return MENU_ERR_UNKNOWN;
    }
}

MenuStatus menu_navigate_down(void) {
    switch(current_menu->type){
        case MENU_SUBMENU:
            return handler_submenu_navigate_down();
        case MENU_PARAMETER:
            return handler_parameter_navigate_down();
        default:
            return MENU_ERR_UNKNOWN;
    }
}

MenuStatus menu_enter(void) {
    if (!menu_is_valid_submenu(current_menu)) return MENU_ERR_INVALID_TYPE;
    MenuItem *item = &current_menu->data.submenu.items[current_index];
    MenuStatus status = MENU_ERR_INVALID_TYPE;
    switch (item->type) {
        case MENU_SUBMENU:
            current_menu = item;
            current_index = 0;
            status = MENU_OK;
            break;

        case MENU_PARAMETER:
            current_menu = item;
            status = MENU_OK;
            break;
        case MENU_FUNCTION:
            if (item->data.function.action) {
                item->data.function.action();
                status = MENU_OK;
            } else {
                status = MENU_ERR_NOT_IMPLEMENTED;
            }
            break;
        case MENU_INFO:
            current_menu = item;
            status = MENU_OK;
            break;
        default:
            status = MENU_ERR_INVALID_TYPE;
    }
    menu_display();
    return status;
}

MenuStatus menu_back(void) {
    if (!current_menu || !current_menu->parent) return MENU_ERR_NOT_FOUND;
    current_menu = current_menu->parent;
    menu_display();
    return MENU_OK;
}

MenuStatus menu_execute_function(void) {
    if (!menu_is_valid_submenu(current_menu)) return MENU_ERR_INVALID_TYPE;
    MenuItem *item = &current_menu->data.submenu.items[current_index];
    if (item->type == MENU_FUNCTION && item->data.function.action) {
        item->data.function.action();
        menu_display();
        return MENU_OK;
    }
    menu_display();
    return MENU_ERR_NOT_IMPLEMENTED;
}
