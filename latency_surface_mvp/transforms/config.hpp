#ifndef ALIGN_BYTES
#define ALIGN_BYTES 0
#endif

#if ALIGN_BYTES > 0
  #define MAYBE_ALIGN alignas(ALIGN_BYTES)
#else
  #define MAYBE_ALIGN
#endif

// other config bits may follow below — keep existing content if any.
