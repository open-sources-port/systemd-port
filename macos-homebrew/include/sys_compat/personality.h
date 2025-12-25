/* Stubs for macOS */
#define PER_LINUX      0
#define PER_LINUX32    1
#define PERSONALITY_INVALID ((unsigned long) -1)

static inline int personality(unsigned long persona) {
    /* macOS does not support Linux personalities, so noop */
    (void) persona;
    return 0;
}