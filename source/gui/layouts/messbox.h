












#ifndef MESSBOX_H
#define MESSBOX_H

class RichText;

void FillMess();
void Mess(RichText* mess, void (*cfun)()=NULL);

extern void (*g_continuefun)();

#endif