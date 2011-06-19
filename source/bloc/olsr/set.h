#ifndef SET_H
# define SET_H

# include <stm32f10x.h>
# include <string.h>

# define SET_FOREACH__(Set, Code)                                       \
  for (int __i = (Set)->first_empty + 1; __i < (Set)->max_size; __i++)  \
  {                                                                     \
    Code;                                                               \
  }

# define SET_FOREACH_(Name, Set, Var, Code)                             \
  SET_FOREACH__(Set,                                                    \
                olsr_##Name##_tuple_t* Var = (Set)->tuples + __i;       \
                Code)

# define SET_FOREACH(Name, Set, Var, Code)                              \
  SET_FOREACH_(Name, Set, Var,                                          \
               if (olsr_##Name##_set_is_empty_(Set, __i))               \
                 continue;                                              \
               Code)

# define SET_APPLY(Name, Set, Action)           \
  SET_FOREACH(Name, Set, __tuple, (*(Action))(__tuple))

# define SET_FIND(Name, Set, Cond, RetVar)                              \
  RetVar = NULL;                                                        \
  SET_FOREACH(Name, Set, __tuple,                                       \
              if ((*(Cond))(__tuple))                                   \
              {                                                         \
                RetVar = __tuple;                                       \
                break;                                                  \
              }                                                         \
    )

# define SET_EMPTY(Name, Set)                                           \
  SET_FOREACH__(Set,                                                    \
                olsr_##Name##_set_declare_empty_(Set, __i))

# define SET_DECLARE(Name, MaxSize)                                     \
  typedef bool (*olsr_##Name##_condition_t)                             \
    (const olsr_##Name##_tuple_t* t);                                   \
  typedef void (*olsr_##Name##_action_t)(olsr_##Name##_tuple_t* t);     \
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
  void olsr_##Name##_set_init_(olsr_##Name##_set_t* set);               \
  void olsr_##Name##_set_empty_(olsr_##Name##_set_t* set);              \
  olsr_##Name##_tuple_t*                                                \
  olsr_##Name##_set_insert_(olsr_##Name##_set_t* set,                   \
                            const olsr_##Name##_tuple_t* tuple);        \
  void olsr_##Name##_set_delete_(olsr_##Name##_set_t* set, int i);      \
  void olsr_##Name##_set_apply_(olsr_##Name##_set_t* set,               \
                                olsr_##Name##_action_t action);         \
  olsr_##Name##_tuple_t*                                                \
  olsr_##Name##_set_find_(olsr_##Name##_set_t* set,                     \
                          olsr_##Name##_condition_t cond);              \
                                                                        \
  inline bool                                                           \
  olsr_##Name##_set_is_empty_(olsr_##Name##_set_t* set, int i)          \
  {                                                                     \
    if (set->bitmap[i / 8] & (1 << (i % 8)))                            \
      return TRUE;                                                      \
    return FALSE;                                                       \
  }                                                                     \
                                                                        \
  inline void                                                           \
  olsr_##Name##_set_declare_empty_(olsr_##Name##_set_t* set, int i)     \
  {                                                                     \
    set->bitmap[i / 8] |= (1 << (i % 8));                               \
  }

# define SET_IMPLEMENT(Name, MaxSize)                                   \
  void                                                                  \
  olsr_##Name##_set_init_(olsr_##Name##_set_t* set)                     \
  {                                                                     \
    set->max_size = MaxSize;                                            \
    set->full = FALSE;                                                  \
    set->n_tuples = 0;                                                  \
    set->first_empty = 0;                                               \
    memset(set->bitmap, 0, sizeof set->bitmap);                         \
    memset(set->tuples, 0, sizeof set->tuples);                         \
  }                                                                     \
                                                                        \
  void olsr_##Name##_set_empty_(olsr_##Name##_set_t* set)               \
  {                                                                     \
    SET_EMPTY(Name, set);                                               \
  }                                                                     \
                                                                        \
  olsr_##Name##_tuple_t*                                                \
  olsr_##Name##_set_insert_(olsr_##Name##_set_t* set,                   \
                            const olsr_##Name##_tuple_t* tuple)         \
  {                                                                     \
    if (set->full)                                                      \
      return NULL;                                                      \
                                                                        \
    set->tuples[set->first_empty] = *tuple;                             \
                                                                        \
    for (int i = set->first_empty + 1; i < MaxSize; i++)                \
    {                                                                   \
      if (olsr_##Name##_set_is_empty_(set, i))                          \
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
  olsr_##Name##_set_delete_(olsr_##Name##_set_t* set, int i)            \
  {                                                                     \
    olsr_##Name##_set_declare_empty_(set, i);                           \
    set->n_tuples--;                                                    \
    set->full = FALSE;                                                  \
    if (i < set->first_empty)                                           \
      set->first_empty = i;                                             \
  }                                                                     \
  void olsr_##Name##_set_apply_(olsr_##Name##_set_t* set,               \
                                olsr_##Name##_action_t action)          \
  {                                                                     \
    SET_APPLY(Name, set, action)                                        \
  }                                                                     \
                                                                        \
  olsr_##Name##_tuple_t*                                                \
  olsr_##Name##_set_find_(olsr_##Name##_set_t* set,                     \
                          olsr_##Name##_condition_t cond)               \
  {                                                                     \
    olsr_##Name##_tuple_t* tuple = NULL;                                \
    SET_FIND(Name, set, cond, tuple);                                   \
    return tuple;                                                       \
  }


# define SET_DEFAULT_BINDINGS(Name)                                     \
  inline void                                                           \
  olsr_##Name##_set_init(olsr_##Name##_set_t* set)                      \
  {                                                                     \
    olsr_##Name##_set_init_(set);                                       \
  }                                                                     \
                                                                        \
  inline void                                                           \
  olsr_##Name##_set_empty(olsr_##Name##_set_t* set)                     \
  {                                                                     \
    olsr_##Name##_set_empty_(set);                                      \
  }                                                                     \
                                                                        \
  inline olsr_##Name##_tuple_t*                                         \
  olsr_##Name##_set_insert(olsr_##Name##_set_t* set,                    \
                            const olsr_##Name##_tuple_t* tuple)         \
  {                                                                     \
    return olsr_##Name##_set_insert_(set, tuple);                       \
  }                                                                     \
                                                                        \
  inline void                                                           \
  olsr_##Name##_set_delete(olsr_##Name##_set_t* set, int i)             \
  {                                                                     \
    olsr_##Name##_set_delete_(set, i);                                  \
  }                                                                     \
                                                                        \
  inline bool                                                           \
  olsr_##Name##_set_is_empty(olsr_##Name##_set_t* set, int i)           \
  {                                                                     \
    return olsr_##Name##_set_is_empty_(set, i);                         \
  }                                                                     \
                                                                        \
  inline void                                                           \
  olsr_##Name##_set_declare_empty(olsr_##Name##_set_t* set, int i)      \
  {                                                                     \
    olsr_##Name##_set_declare_empty_(set, i);                           \
  }                                                                     \
                                                                        \
  inline void                                                           \
  olsr_##Name##_set_apply(olsr_##Name##_set_t* set,                     \
                          olsr_##Name##_action_t action)                \
  {                                                                     \
    olsr_##Name##_set_apply_(set, action);                              \
  }                                                                     \
                                                                        \
  inline olsr_##Name##_tuple_t*                                         \
  olsr_##Name##_set_find(olsr_##Name##_set_t* set,                      \
                           olsr_##Name##_condition_t cond)              \
  {                                                                     \
    return olsr_##Name##_set_find_(set, cond);                          \
  }

#endif
