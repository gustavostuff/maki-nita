#ifndef FW_FLUSH_H
#define FW_FLUSH_H

/* Power-up register sequence, then the current frame, four words per row. */
void drivers_init(void);
void drivers_flush(void);

#endif
