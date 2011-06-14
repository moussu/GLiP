#ifndef SET_H
# define SET_H

# include <stm32f10x.h>

# define SET_DECLARE(Name, MaxSize)                                     \
  typedef struct                                                        \
  {                                                                     \
    int n_tuples;                                                       \
    int max_size;                                                       \
    int first_empty;                                                    \
    bool full;                                                          \
    uint8_t bitmap[MaxSize / 8];                                        \
    olsr_##Name##_tuple_t tuples[MaxSize];                              \
  } olsr_##Name##_set_t;                                                \
                                                                        \
  void olsr_##Name##_set_init(olsr_##Name##_set_t* set);                \
  olsr_##Name##_tuple_t*                                                \
  olsr_##Name##_set_insert(olsr_##Name##_set_t* set,                    \
                           const olsr_##Name##_tuple_t* tuple);         \
  void olsr_##Name##_set_delete(olsr_##Name##_set_t* set, int i);       \
                                                                        \
  inline bool                                                           \
  olsr_##Name##_set_is_empty(olsr_##Name##_set_t* set, int i)           \
  {                                                                     \
    if (set->bitmap[i / 8] & (1 << (i % 8)))                            \
      return TRUE;                                                      \
    return FALSE;                                                       \
  }                                                                     \
                                                                        \
  inline void                                                           \
  olsr_##Name##_set_declare_empty(olsr_##Name##_set_t* set, int i)      \
  {                                                                     \
    set->bitmap[i / 8] |= (1 << (i % 8));                               \
  }

# define SET_IMPLEMENT(Name, MaxSize)                                   \
  void                                                                  \
  olsr_##Name##_set_init(olsr_##Name##_set_t* set)                      \
  {                                                                     \
    set->max_size = MaxSize;                                            \
    set->full = FALSE;                                                  \
    set->n_tuples = 0;                                                  \
    set->first_empty = 0;                                               \
    memset(set->bitmap, 0, sizeof set->bitmap);                         \
    memset(set->tuples, 0, sizeof set->tuples);                         \
  }                                                                     \
                                                                        \
  olsr_##Name##_tuple_t*                                                \
  olsr_##Name##_set_insert(olsr_##Name##_set_t* set,                    \
                           const olsr_##Name##_tuple_t* tuple)          \
  {                                                                     \
    if (set->full)                                                      \
      return NULL;                                                      \
                                                                        \
    set->tuples[set->first_empty] = *tuple;                             \
                                                                        \
    for (int i = set->first_empty + 1; i < MaxSize; i++)                \
    {                                                                   \
      if (olsr_##Name##_set_is_empty(set, i))                           \
      {                                                                 \
        set->first_empty = i;                                           \
        break;                                                          \
      }                                                                 \
    }                                                                   \
                                                                        \
    set->n_tuples++;                                                    \
                                                                        \
    if (set->n_tuples == MaxSize)                                       \
      set->full = TRUE;                                                 \
                                                                        \
    return set->tuples + set->first_empty;                              \
  }                                                                     \
                                                                        \
  void                                                                  \
  olsr_##Name##_set_delete(olsr_##Name##_set_t* set, int i)             \
  {                                                                     \
    olsr_##Name##_set_declare_empty(set, i);                            \
    set->n_tuples--;                                                    \
    set->full = FALSE;                                                  \
    if (i < set->first_empty)                                           \
      set->first_empty = i;                                             \
  }

#endif
