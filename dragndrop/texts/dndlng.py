# -*- coding: utf-8 -*-
'''
Template for lng files:
'''

ENCODING = 'utf-8' # optional

HEADER_NAME = 'ddlng.h'

LANGUAGES = [
    ('dndrus.lng', 'Russian', u'Russian (Русский)'),
    ('dndeng.lng', 'English', 'English'),
]

# h file header
HEADER = \
"""
#ifndef __KARBAZOL_DRAGNDROP_2_0__DDLNG_H__
#define __KARBAZOL_DRAGNDROP_2_0__DDLNG_H__

"""

# h file footer
FOOTER = \
"""
#endif // __KARBAZOL_DRAGNDROP_2_0__DDLNG_H__

"""

ITEMS = [
    (
        "MOK=0",
        u"Продолжить",
        "OK"
    ),

    (
        "MCancel",
        u"Отменить",
        "Cancel"
    ),

    (
        "MConfigMenu",
        "Drag'n'drop",
        "Drag'n'drop"
    ),

    (
        "MConfigTitle",
        u"Drag'n'drop - настройки",
        "Drag'n'drop - Options"
    ),

    (
        "MUseKeyToStartDND",
        u"Drag'n'drop если нажата к&лавиша",
        "Start drag only if the &key pressed"
    ),

    (
        "MLeftCtl",
        u"Левый Ctrl",
        "Left Ctrl"
    ),

    (
        "MLeftAlt",
        u"Левый Alt",
        "Left Alt"
    ),

    (
        "MShift",
        "Shift",
        "Shift"
    ),

    (
        "MRightCtl",
        u"Правый Ctrl",
        "Right Ctrl"
    ),

    (
        "MRightAlt",
        u"Правый Alt",
        "Right Alt"
    ),

    (
        "MPanels",
        u"Панели",
        "Panels"
    ),

    (
        "MOptions",
        u"Дополнительно",
        "Options"
    ),

    (
        "MUseShellCopy",
        u"Копировать Проводником",
        "Use Explorer copy function"
    ),

    (
        "MShowMenu",
        u"Показывать меню по правой кнопке",
        "Right click shows menu"
    ),

    (
        "MPixelsPassed",
        u"Количество пикселей, после которого",
        "Number of pixels to pass"
    ),

    (
        "MPixelsPassed2",
        u"следует начинать Drag",
        "before start dragging"
    ),

    (
        "MOpCaneling",
        u"Действие прервано",
        "Operation has been interrupted"
    ),
    (
        "MRUSureCancel",
        u"Вы действительно хотите его прервать?",
        "Are you sure to cancel it?"
    ),

    (
        "MMove",
        u"Перемещение",
        "Move"
    ),
    (
        "MCopy",
        u"Копирование",
        "Copy"
    ),

    (
        "MMovingTheFile",
        u"Перемещение файла:",
        "Moving the file:"
    ),
    (
        "MCopyingTheFile",
        u"Копирование файла:",
        "Copying the file:"
    ),

    (
        "MCopyingTo",
        u"в",
        "to"
    ),
    (
        "MFilesProcessed",
        u"Обработано файлов: %d of %d",
        "Files processed: %d of %d"
    ),

    (
        "MFileCopyingTimes",
        u"Время: %.2d:%.2d:%.2d Ост: %.2d:%.2d:%.2d %6dКб/с",
        "Time: %.2d:%.2d:%.2d Left: %.2d:%.2d:%.2d %6dKb/s"
    ),
    (
        "MTotalSize",
        u" Всего: %s ",
        " Total: %s "
    ),

    #Overwrite dialog
    (
        "MWarning",
        u"Предупреждение",
        "Warning"
    ),

    (
        "MFileExists",
        u"Файл уже существует",
        "File already exists"
    ),

    (
        "MNew",
        u"Новый",
        "New"
    ),

    (
        "MExisting",
        u"Существующий",
        "Existing"
    ),

    (
        "MOverwrite",
        u"В&место",
        "&Overwrite"
    ),

    (
        "MAll",
        u"&Все",
        "&All"
    ),

    (
        "MSkip",
        u"&Пропустить",
        "&Skip"
    ),

    (
        "MSkipall",
        u"П&ропустить все",
        "S&kip all"
    ),

    (
        "MAppend",
        u"&Дописать",
        "A&ppend"
    ),

    (
        "MResume",
        u"Возоб&новить",
        "&Resume"
    ),

    (
        "MAmpCancel",
        u"&Отменить",
        "&Cancel"
    ),

    (
        "MHolderFailed",
        u"Ошибка запуска HOLDER.DND. Сброс файлов на FAR невозможен.",
        "HOLDER.DND failed to initialize. No drop will available to FAR."
    ),

    (
        "MError",
        u"Ошибка",
        "Error"
    ),

    (
        "MCopyError",
        u"Невозможно скопировать",
        "Cannot copy"
    ),

    (
        "MCopyOverwrite",
        u"Переписать",
        "&Overwrite"
    ),

    (
        "MRetry",
        u"Повторить",
        "&Retry"
    )
]

