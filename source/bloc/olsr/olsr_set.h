#ifndef SET_H
# define SET_H

# include <stm32f10x.h>
# include <string.h>

# define SET_FOREACH__(Name, Code)                                      \
  for (int __i = Name##_set.first_empty + 1;                            \
       __i < Name##_set.max_size; ++__i)                                \
  {                                                                     \
    Code;                                                               \
  }

# define SET_FOREACH_(Name, Var, Code)                          \
  SET_FOREACH__(Name,                                           \
                olsr_##Name##_tuple_t* Var =                    \
                Name##_set.tuples + __i;                        \
                Code)

# define SET_FOREACH(Name, Var, Code)                   \
  SET_FOREACH_(Name, Var,                               \
               if (olsr_##Name##_set_is_empty_(__i))    \
                 continue;                              \
               Code)

# define SET_APPLY(Name, Action)                        \
  SET_FOREACH(Name, __tuple, (*(Action))(__tuple))

# define SET_FIND(Name, Cond, RetVar)            \
  RetVar = NULL;                                 \
  SET_FOREACH(Name, ___tuple,                    \
              if ((*(Cond))(__tuple))            \
              {                                  \
                RetVar = ___tuple;               \
                break;                           \
              }                                  \
    )

# define SET_EMPTY(Name)                                        \
  SET_FOREACH__(Name, olsr_##Name##_set_declare_empty_(__i))

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
  extern olsr_##Name##_set_t Name##_set;                                \
                                                                        \
  void olsr_##Name##_set_init_();                                       \
  void olsr_##Name##_set_empty_();                                      \
  olsr_##Name##_tuple_t*                                                \
  olsr_##Name##_set_insert_(const olsr_##Name##_tuple_t* tuple);        \
  void olsr_##Name##_set_delete_(int i);                                \
  void olsr_##Name##_set_apply_(olsr_##Name##_action_t action);         \
  olsr_##Name##_tuple_t*                                                \
  olsr_##Name##_set_find_(olsr_##Name##_condition_t cond);              \
                                                                        \
  inline bool                                                           \
  olsr_##Name##_set_is_empty_(int i)                                    \
  {                                                                     \
    if (Name##_set.bitmap[i / 8] & (1 << (i % 8)))                      \
      return TRUE;                                                      \
    return FALSE;                                                       \
  }                                                                     \
                                                                        \
  inline void                                                           \
  olsr_##Name##_set_declare_empty_(int i)                               \
  {                                                                     \
    Name##_set.bitmap[i / 8] |= (1 << (i % 8));                         \
  }

# define SET_IMPLEMENT(Name, MaxSize)                           \
  olsr_##Name##_set_t Name##_set;                               \
  void                                                          \
  olsr_##Name##_set_init_()                                     \
  {                                                             \
    Name##_set.max_size = MaxSize;                              \
    Name##_set.full = FALSE;                                    \
    Name##_set.n_tuples = 0;                                    \
    Name##_set.first_empty = 0;                                 \
    memset(Name##_set.bitmap, 0, sizeof Name##_set.bitmap);     \
    memset(Name##_set.tuples, 0, sizeof Name##_set.tuples);     \
    SET_FOREACH(Name, tuple, olsr_##Name##_tuple_init(tuple));  \
  }                                                             \
                                                                \
  void olsr_##Name##_set_empty_()                               \
  {                                                             \
    SET_EMPTY(Name);                                            \
  }                                                             \
                                                                \
  olsr_##Name##_tuple_t*                                        \
  olsr_##Name##_set_insert_(const olsr_##Name##_tuple_t* tuple) \
  {                                                             \
    if (Name##_set.full)                                        \
      return NULL;                                              \
                                                                \
    Name##_set.tuples[Name##_set.first_empty] = *tuple;         \
                                                                \
    for (int i = Name##_set.first_empty + 1; i < MaxSize; i++)  \
    {                                                           \
      if (olsr_##Name##_set_is_empty_(i))                       \
      {                                                         \
        Name##_set.first_empty = i;                             \
        break;                                                  \
      }                                                         \
    }                                                           \
                                                                \
    Name##_set.n_tuples++;                                      \
                                                                \
    if (Name##_set.n_tuples == MaxSize)                         \
      Name##_set.full = TRUE;                                   \
                                                                \
    return Name##_set.tuples + Name##_set.first_empty;          \
  }                                                             \
                                                                \
  void                                                          \
  olsr_##Name##_set_delete_(int i)                              \
  {                                                             \
    olsr_##Name##_set_declare_empty_(i);                        \
    Name##_set.n_tuples--;                                      \
    Name##_set.full = FALSE;                                    \
    if (i < Name##_set.first_empty)                             \
      Name##_set.first_empty = i;                               \
  }                                                             \
                                                                \
  void olsr_##Name##_set_apply_(olsr_##Name##_action_t action)  \
  {                                                             \
    SET_APPLY(Name, action);                                    \
  }                                                             \
                                                                \
  olsr_##Name##_tuple_t*                                        \
  olsr_##Name##_set_find_(olsr_##Name##_condition_t cond)       \
  {                                                             \
    olsr_##Name##_tuple_t* __tuple = NULL;                      \
    SET_FIND(Name, cond, __tuple);                              \
    return __tuple;                                             \
  }


# define SET_DEFAULT_BINDINGS(Name)                                     \
  inline void                                                           \
  olsr_##Name##_set_init()                                              \
  {                                                                     \
    olsr_##Name##_set_init_();                                          \
  }                                                                     \
                                                                        \
  inline void                                                           \
  olsr_##Name##_set_empty()                                             \
  {                                                                     \
    olsr_##Name##_set_empty_();                                         \
  }                                                                     \
                                                                        \
  inline olsr_##Name##_tuple_t*                                         \
  olsr_##Name##_set_insert(const olsr_##Name##_tuple_t* tuple)          \
  {                                                                     \
    return olsr_##Name##_set_insert_(tuple);                            \
  }                                                                     \
                                                                        \
  inline void                                                           \
  olsr_##Name##_set_delete(int i)                                       \
  {                                                                     \
    olsr_##Name##_set_delete_(i);                                       \
  }                                                                     \
                                                                        \
  inline bool                                                           \
  olsr_##Name##_set_is_empty(int i)                                     \
  {                                                                     \
    return olsr_##Name##_set_is_empty_(i);                              \
  }                                                                     \
                                                                        \
  inline void                                                           \
  olsr_##Name##_set_declare_empty(int i)                                \
  {                                                                     \
    olsr_##Name##_set_declare_empty_(i);                                \
  }                                                                     \
                                                                        \
  inline void                                                           \
  olsr_##Name##_set_apply(olsr_##Name##_action_t action)                \
  {                                                                     \
    olsr_##Name##_set_apply_(action);                                   \
  }                                                                     \
                                                                        \
  inline olsr_##Name##_tuple_t*                                         \
  olsr_##Name##_set_find(olsr_##Name##_condition_t cond)                \
  {                                                                     \
    return olsr_##Name##_set_find_(cond);                               \
  }

#endif