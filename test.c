#include "aparse.h"
#include <stdio.h>

int custom_cb(void *uptr, ap_cb_data *pdata) {
  (void)uptr;
  printf("%s %u\n", pdata->arg, (unsigned)pdata->arg_len);
  pdata->more = !pdata->idx;
  return pdata->more;
}

int main(int argc, const char *const *argv) {
  ap *par = ap_init("test");
  int flag = 0, i;
  ap_begin_opt(par, 'O', "option");
  ap_type_flag(par, &flag);
  ap_end(par);
  ap_begin_pos(par, "var");
  ap_type_int(par, &i);
  ap_end(par);
  printf("%i\n", ap_parse(par, argc - 1, argv + 1));
  printf("%i %i\n", flag, i);
  ap_destroy(par);
  return 0;
}
