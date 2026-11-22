#ifndef SETTINGS_H
#define SETTINGS_H

// C runs no code before main, so a value that has to be worked out is behind a
// function, which works it out the first time it is called.
const char *settings_greeting(void);

#endif
