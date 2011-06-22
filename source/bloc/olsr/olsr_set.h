#ifndef SET_H
# define SET_H

# include <stm32f10x.h>
# include <string.h>

# define SET_FOREACH___(Name, Code)                     \
  for (int __i_##Name = Name##_set.first_empty + 1;     \
       __i_##Name < Name##_set.max_size; ++__i_##Name)  \
  {                                                     \
    Code;                                               \
  }

# define SET_FOREACH__(Name, Tuple, Var, Code)          \
  SET_FOREACH___(Name,                                  \
                 olsr_##Tuple##_tuple_t* Var =          \
                 Name##_set.tuples + __i_##Name;        \
                 Code)

# define SET_FOREACH_(Name, Tuple, Var, Code)                   \
  SET_FOREACH__(Name, Tuple, Var,                               \
                if (olsr_##Name##_set_is_empty_(__i_##Name))    \
                  continue;                                     \
                Code)

# define SET_FOREACH(Name, Var, Code)                   \
  SET_FOREACH_(Name, Name, Var, Code)

# define SET_APPLY_(Name, Tuple, Action)                \
  SET_FOREACH_(Name, Tuple, __tuple, (*(Action))(__tuple))

# define SET_APPLY(Name, Action)                        \
  SET_APPLY_(Name, Name, Action)

# define SET_FIND_(Name, Tuple, Cond, RetVar)   \
  RetVar = NULL;                                \
  SET_FOREACH_(Name, Tuple, ___tuple,           \
              if ((*(Cond))(__tuple))           \
              {                                 \
                RetVar = ___tuple;              \
                break;                          \
              }                                 \
    )

# define SET_FIND(Name, Cond, RetVar)           \
  SET_FIND_(Name, Name, Cond, RetVar)

# define SET_EMPTY(Name)                                                \
  SET_FOREACH___(Name, olsr_##Name##_set_declare_empty_(__i_##Name))

# define SET_DECLARE_(Name, Tuple, MaxSize)                             \
  typedef bool (*olsr_##Name##_condition_t)                             \
    (const olsr_##Tuple##_tuple_t* t);                                  \
  typedef void (*olsr_##Name##_action_t)(olsr_##Tuple##_tuple_t* t);    \
  typedef struct                                                        \
  {                                                                     \
    int n_tuples;                                                       \
    int max_size;                                                       \
    int first_empty;                                                    \
    bool full;                                                          \
    uint8_t bitmap[MaxSize / 8];                                        \
    olsr_##Tuple##_tuple_t tuples[MaxSize];                             \
  } olsr_##Name##_set_t;                                                \
  extern olsr_##Name##_set_t Name##_set;                                \
                                                                        \
  void olsr_##Name##_set_init_();                                       \
  void olsr_##Name##_set_empty_();                                      \
  olsr_##Tuple##_tuple_t*                                               \
  olsr_##Name##_set_insert_(const olsr_##Tuple##_tuple_t* tuple);       \
  void olsr_##Name##_set_delete_(int i);                                \
  void olsr_##Name##_set_apply_(olsr_##Name##_action_t action);         \
  olsr_##Tuple##_tuple_t*                                               \
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

# define SET_DECLARE(Name, MaxSize)             \
  SET_DECLARE_(Name, Name, MaxSize)

# define SET_IMPLEMENT_(Name, Tuple, MaxSize)                           \
  olsr_##Name##_set_t Name##_set;                                       \
                                                                        \
  void                                                                  \
  olsr_##Name##_set_init_()                                             \
  {                                                                     \
    Name##_set.max_size = MaxSize;                                      \
    Name##_set.full = FALSE;                                            \
    Name##_set.n_tuples = 0;                                            \
    Name##_set.first_empty = 0;                                         \
    memset(Name##_set.bitmap, 0, sizeof Name##_set.bitmap);             \
    memset(Name##_set.tuples, 0, sizeof Name##_set.tuples);             \
    SET_FOREACH_(Name, Tuple, tuple, olsr_##Tuple##_tuple_init(tuple)); \
  }                                                                     \
                                                                        \
  void olsr_##Name##_set_empty_()                                       \
  {                                                                     \
    SET_EMPTY(Name);                                                    \
  }                                                                     \
                                                                        \
  olsr_##Tuple##_tuple_t*                                               \
  olsr_##Name##_set_insert_(const olsr_##Tuple##_tuple_t* tuple)        \
  {                                                                     \
    if (Name##_set.full)                                                \
      return NULL;                                                      \
                                                                        \
    Name##_set.tuples[Name##_set.first_empty] = *tuple;                 \
                                                                        \
    for (int i = Name##_set.first_empty + 1; i < MaxSize; i++)          \
    {                                                                   \
      if (olsr_##Name##_set_is_empty_(i))                               \
      {                                                                 \
        Name##_set.first_empty = i;                                     \
        break;                                                          \
      }                                                                 \
    }                                                                   \
                                                                        \
    Name##_set.n_tuples++;                                              \
                                                                        \
    if (Name##_set.n_tuples == MaxSize)                                 \
      Name##_set.full = TRUE;                                           \
                                                                        \
    return Name##_set.tuples + Name##_set.first_empty;                  \
  }                                                                     \
                                                                        \
  void                                                                  \
  olsr_##Name##_set_delete_(int i)                                      \
  {                                                                     \
    olsr_##Name##_set_declare_empty_(i);                                \
    Name##_set.n_tuples--;                                              \
    Name##_set.full = FALSE;                                            \
    if (i < Name##_set.first_empty)                                     \
      Name##_set.first_empty = i;                                       \
  }                                                                     \
                                                                        \
  void olsr_##Name##_set_apply_(olsr_##Name##_action_t action)          \
  {                                                                     \
    SET_APPLY_(Name, Tuple, action);                                    \
  }                                                                     \
                                                                        \
  olsr_##Tuple##_tuple_t*                                               \
  olsr_##Name##_set_find_(olsr_##Name##_condition_t cond)               \
  {                                                                     \
    olsr_##Tuple##_tuple_t* __tuple = NULL;                             \
    SET_FIND_(Name, Tuple, cond, __tuple);                              \
    return __tuple;                                                     \
  }

# define SET_IMPLEMENT(Name, MaxSize)           \
  SET_IMPLEMENT_(Name, Name, MaxSize)

# define SET_DEFAULT_INIT(Name)                 \
  inline void                                   \
  olsr_##Name##_set_init()                      \
  {                                             \
    olsr_##Name##_set_init_();                  \
  }

# define SET_DEFAULT_EMPTY(Name)                \
  inline void                                   \
  olsr_##Name##_set_empty()                     \
  {                                             \
    olsr_##Name##_set_empty_();                 \
  }

# define SET_DEFAULT_INSERT(Name)                               \
  inline olsr_##Name##_tuple_t*                                 \
  olsr_##Name##_set_insert(const olsr_##Name##_tuple_t* tuple)  \
  {                                                             \
    return olsr_##Name##_set_insert_(tuple);                    \
  }

# define SET_DEFAULT_DELETE(Name)               \
  inline void                                   \
  olsr_##Name##_set_delete(int i)               \
  {                                             \
    olsr_##Name##_set_delete_(i);               \
  }

# define SET_DEFAULT_APPLY(Name)                                \
  inline void                                                   \
  olsr_##Name##_set_apply(olsr_##Name##_action_t action)        \
  {                                                             \
    olsr_##Name##_set_apply_(action);                           \
  }

# define SET_DEFAULT_FIND(Name)                                 \
  inline olsr_##Name##_tuple_t*                                 \
  olsr_##Name##_set_find(olsr_##Name##_condition_t cond)        \
  {                                                             \
    return olsr_##Name##_set_find_(cond);                       \
  }

# define SET_DEFAULT_IS_EMPTY(Name)             \
  inline bool                                   \
  olsr_##Name##_set_is_empty(int i)             \
  {                                             \
    return olsr_##Name##_set_is_empty_(i);      \
  }

# define SET_DEFAULT_DECLARE_EMPTY(Name)        \
  inline void                                   \
  olsr_##Name##_set_declare_empty(int i)        \
  {                                             \
    olsr_##Name##_set_declare_empty_(i);        \
  }

# define SET_DEFAULT_BINDINGS(Name)             \
  SET_DEFAULT_INIT(Name)                        \
  SET_DEFAULT_EMPTY(Name)                       \
  SET_DEFAULT_INSERT(Name)                      \
  SET_DEFAULT_DELETE(Name)                      \
  SET_DEFAULT_APPLY(Name)                       \
  SET_DEFAULT_FIND(Name)                        \
  SET_DEFAULT_IS_EMPTY(Name)                    \
  SET_DEFAULT_DECLARE_EMPTY(Name)

#endif
