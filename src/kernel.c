/*
Commented code at https://wiki.osdev.org/Bare_Bones#Writing_a_kernel_in_C
*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(
	enum vga_color fg, enum vga_color bg
){
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) {
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

void strcpy(char* dst, const char* src) {
	while(*dst++ = *src++);
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(
		VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK
	);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH +x;
			terminal_buffer[index] = vga_entry(
				' ', terminal_color
			);
		}
	}
}

void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

void newline() {
	terminal_column = 0;
	if (++terminal_row == VGA_HEIGHT)
		terminal_row = 0;
}

void terminal_putchar(char c) {
	if (c == '\n') {
		newline();
	}
	else {
		terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
		if (++terminal_column == VGA_WIDTH) {
			newline();
		}
	}
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}

char cat[] = "\x7c\x5c\x20\x20\x20\x20\x20\x20\x20\x20\x20\x2f\x7c\x0a\x7c\x20\x5c\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2f\x20\x7c\x0a\x7c\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x7c\x0a\x7c\x20\x20\x4f\x20\x20\x20\x20\x20\x4f\x20\x20\x7c\x0a\x7c\x20\x20\x28\x20\x20\x59\x20\x20\x29\x20\x20\x7c\x0a\x7c\x2d\x2d\x2d\x2d\x2d\x4f\x2d\x2d\x2d\x2d\x2d\x7c\x5c\x0a\x7c\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x7c\x20\x5c\x0a\x7c\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x7c\x20\x20\x5c\x0a\x7c\x20\x20\x7c\x20\x20\x20\x20\x20\x7c\x20\x20\x7c\x20\x20\x20\x5c\x0a\x7c\x20\x20\x7c\x20\x20\x20\x20\x20\x7c\x20\x20\x7c\x20\x20\x20\x20\x5c\x0a\x7c\x20\x20\x7c\x20\x20\x20\x20\x20\x7c\x20\x20\x7c\x20\x20\x20\x20\x20\x5c\x0a\x7c\x20\x20\x7c\x20\x20\x20\x20\x20\x7c\x20\x20\x7c\x20\x20\x20\x20\x20\x20\x5c\x20\x20\x20\x20\x4b\x65\x72\x6e\x65\x6c\x20\x43\x61\x74\x0a\x7c\x20\x20\x7c\x20\x20\x20\x20\x20\x7c\x20\x20\x7c\x20\x20\x20\x20\x20\x20\x20\x5c\x5f\x5f\x5f\x5f\x5f\x5f\x5f\x5f\x5f\x5f\x5f\x5f\x5f\x5f\x0a\x28\x28\x20\x29\x2d\x2d\x2d\x2d\x2d\x28\x20\x20\x29\x29\x20\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x2d\x20\x20\x0a";

void kernel_main(void) {
	terminal_initialize();
	terminal_writestring(cat);
	terminal_writestring("wiki.osdev.org\n");
	terminal_writestring("Made by vinicyus");
}
