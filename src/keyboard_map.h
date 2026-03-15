#ifndef KEYBOARD_MAP_H
#define KEYBOARD_MAP_H

#include "types.h"

#define KEY_BACKSPACE 0x0E
#define KEY_ENTER 0x1C
#define KEY_LSHIFT 0x2A
#define KEY_RSHIFT 0x36
#define KEY_EXTENDED_PREFIX 0xE0
#define KEY_LSHIFT_RELEASE 0xAA
#define KEY_RSHIFT_RELEASE 0xB6
#define KEY_ARROW_UP 0x48
#define KEY_ARROW_LEFT 0x4B
#define KEY_ARROW_RIGHT 0x4D
#define KEY_ARROW_DOWN 0x50

// Scancode to ASCII mapping (US keyboard layout)
static const char scancode_to_ascii[] = {
    0,    0,   '1', '2',  '3',  '4', '5',  '6',  // 0x00-0x07
    '7',  '8', '9', '0',  '-',  '=', '\b', '\t', // 0x08-0x0F
    'q',  'w', 'e', 'r',  't',  'y', 'u',  'i',  // 0x10-0x17
    'o',  'p', '[', ']',  '\n', 0,   'a',  's',  // 0x18-0x1F
    'd',  'f', 'g', 'h',  'j',  'k', 'l',  ';',  // 0x20-0x27
    '\'', '`', 0,   '\\', 'z',  'x', 'c',  'v',  // 0x28-0x2F
    'b',  'n', 'm', ',',  '.',  '/', 0,    '*',  // 0x30-0x37
    0,    ' ', 0,   0,    0,    0,   0,    0,    // 0x38-0x3F
};

// Shifted characters
static const char scancode_to_ascii_shift[] = {
    0,   0,   '!', '@', '#',  '$', '%',  '^',  // 0x00-0x07
    '&', '*', '(', ')', '_',  '+', '\b', '\t', // 0x08-0x0F
    'Q', 'W', 'E', 'R', 'T',  'Y', 'U',  'I',  // 0x10-0x17
    'O', 'P', '{', '}', '\n', 0,   'A',  'S',  // 0x18-0x1F
    'D', 'F', 'G', 'H', 'J',  'K', 'L',  ':',  // 0x20-0x27
    '"', '~', 0,   '|', 'Z',  'X', 'C',  'V',  // 0x28-0x2F
    'B', 'N', 'M', '<', '>',  '?', 0,    '*',  // 0x30-0x37
    0,   ' ', 0,   0,   0,    0,   0,    0,    // 0x38-0x3F
};

#endif
