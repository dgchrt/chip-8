#define _DEFAULT_SOURCE
#include "hal.h"
#include <SDL.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

// --- Internal Key Queue ---
#define KEY_QUEUE_SIZE 32
static int key_queue[KEY_QUEUE_SIZE];
static int key_head = 0;
static int key_tail = 0;

static void push_key(int ch) {
  int next = (key_head + 1) % KEY_QUEUE_SIZE;
  if (next != key_tail) {
    key_queue[key_head] = ch;
    key_head = next;
  }
}

static int pop_key(void) {
  if (key_head == key_tail)
    return -1;
  int ch = key_queue[key_tail];
  key_tail = (key_tail + 1) % KEY_QUEUE_SIZE;
  return ch;
}

// --- Terminal State ---
static struct termios old_termios;
static int old_fcntl;

static void terminal_setup(void) {
  tcgetattr(STDIN_FILENO, &old_termios);
  struct termios new_termios = old_termios;
  new_termios.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

  old_fcntl = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, old_fcntl | O_NONBLOCK);
}

static void terminal_restore(void) {
  tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
  fcntl(STDIN_FILENO, F_SETFL, old_fcntl);
}

static void terminal_poll(void) {
  unsigned char ch;
  while (read(STDIN_FILENO, &ch, 1) > 0) {
    push_key(ch);
  }
}

// --- Conio Implementation ---
void clrscr(void) { printf("\033[H\033[J"); }

int getch(void) {
  int ch;
  while ((ch = pop_key()) == -1) {
    terminal_poll();
    usleep(1000); // Wait for input
  }
  return ch;
}

int kbhit(void) {
  terminal_poll();
  return key_head != key_tail;
}

// --- SDL Implementation ---
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

int hal_init(const char *title, int width, int height) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }

  window =
      SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                       width * 8, height * 8, SDL_WINDOW_SHOWN);
  if (!window)
    return -1;

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer)
    return -1;

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                              SDL_TEXTUREACCESS_STREAMING, width, height);
  if (!texture)
    return -1;

  terminal_setup();
  return 0;
}

void hal_present(const uint32_t *buffer, int width, int height) {
  (void)height;
  SDL_UpdateTexture(texture, NULL, buffer, width * sizeof(uint32_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

int hal_handle_events(void) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT)
      return 0;
    if (e.type == SDL_KEYDOWN) {
      int ch = 0;
      switch (e.key.keysym.sym) {
      case SDLK_ESCAPE: return 0;
      case SDLK_1: ch = '1'; break;
      case SDLK_2: ch = '2'; break;
      case SDLK_3: ch = '3'; break;
      case SDLK_4: ch = '4'; break;
      case SDLK_q: ch = 'q'; break;
      case SDLK_w: ch = 'w'; break;
      case SDLK_e: ch = 'e'; break;
      case SDLK_r: ch = 'r'; break;
      case SDLK_a: ch = 'a'; break;
      case SDLK_s: ch = 's'; break;
      case SDLK_d: ch = 'd'; break;
      case SDLK_f: ch = 'f'; break;
      case SDLK_z: ch = 'z'; break;
      case SDLK_x: ch = 'x'; break;
      case SDLK_c: ch = 'c'; break;
      case SDLK_v: ch = 'v'; break;
      }
      if (ch) push_key(ch);
    }
  }
  terminal_poll();
  return 1;
}

void hal_cleanup(void) {
  terminal_restore();
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

// --- Host I/O Implementation ---
void hal_log(const char *message) { printf("%s\n", message); }

void hal_exit(int code) { exit(code); }

int hal_load(const char *path, uint8_t *buffer, size_t size) {
  FILE *file = fopen(path, "rb");
  if (!file)
    return -1;
  size_t read = fread(buffer, 1, size, file);
  fclose(file);
  return (int)read;
}
