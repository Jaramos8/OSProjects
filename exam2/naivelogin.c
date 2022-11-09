
#include <unistd.h>

#if !defined(LOGIN_SECRET)
#define LOGIN_SECRET "password"
#endif

static int read_character(char *c) {
  char buf;
  ssize_t res;

  res = read(0, &buf, (size_t) 1);
  
  if (res == ((ssize_t) 1)) {
    *c = buf;
    return 0;
  }
  return -1;
}

int main(int argc, char **argv) {
  char secret[] = LOGIN_SECRET;
  int i;
  char c;

  for (i=0; secret[i]!='\0'; i++) {
    if (read_character(&c) < 0) return 1;
    if (c != secret[i]) return 1;
  }
  return 0;
}

