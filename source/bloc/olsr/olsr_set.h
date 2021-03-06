#ifndef SET_H
# define SET_H

# include <stm32f10x.h>
# include <stdlib.h>
# include <string.h>
# include <FreeRTOS.h>
# include <semphr.h>
# include <task.h>
# include "olsr_time.h"
# include "utils/max.h"

# ifdef DEBUG
#  define SET_ABORT abort()
# else
#  define SET_ABORT
# endif

# define SET_FOREACH___(Name, Code)                     \
  for (int __i_##Name = 0;                              \
       __i_##Name < Name##_set.max_size;                \
       ++__i_##Name)                                    \
  {                                                     \
    Code;                                               \
  }

# define SET_FOREACH__(Name, Tuple, Var, Code)                          \
  SET_FOREACH___(Name,                                                  \
                 olsr_##Tuple##_tuple_t* Var __attribute__((unused)) =  \
                 Name##_set.tuples + __i_##Name;                        \
                 Code)

# define SET_FOREACH_(Name, Tuple, Var, Code)                   \
  SET_FOREACH__(Name, Tuple, Var,                               \
                if (olsr_##Name##_set_is_empty_(__i_##Name))    \
                  continue;                                     \
                Code)

# define SET_FOREACH(Name, Var, Code)                   \
  SET_FOREACH_(Name, Name, Var, Code)

# define SET_FOREACH_AUTOREMOVE(Name, Var, TimeField, Code)     \
  SET_FOREACH_(Name, Name, Var,                                 \
    if (Var->TimeField < olsr_get_current_time())               \
    {                                                           \
      olsr_##Name##_set_delete(__i_##Name);                     \
      continue;                                                 \
    }                                                           \
    Code)

# define SET_APPLY_(Name, Tuple, Action)                \
  SET_FOREACH_(Name, Tuple, __tuple, (*(Action))(__tuple))

# define SET_APPLY(Name, Action)                        \
  SET_APPLY_(Name, Name, Action)

# define SET_FIND_(Name, Tuple, RetVar, Cond)   \
  RetVar = NULL;                                \
  SET_FOREACH_(Name, Tuple, ___tuple,           \
              if ((*(Cond))(__tuple))           \
              {                                 \
                RetVar = ___tuple;              \
                break;                          \
              }                                 \
    )

# define SET_FIND(Name, RetVar, Cond)           \
  SET_FIND_(Name, Name, RetVar, Cond)

# define SET_DECLARE_(Name, Tuple, MaxSize)                             \
  typedef bool (*olsr_##Name##_condition_t)                             \
    (const olsr_##Tuple##_tuple_t* t);                                  \
  typedef void (*olsr_##Name##_action_t)(olsr_##Tuple##_tuple_t* t);    \
  typedef struct                                                        \
  {                                                                     \
    int n_tuples;                                                       \
    int max_size;                                                       \
    int first_empty;                                                    \
    int bitmap_size;                                                    \
    bool full;                                                          \
    uint8_t bitmap[DIVUP(MaxSize, 8)];                                  \
    olsr_##Tuple##_tuple_t tuples[MaxSize];                             \
  } olsr_##Name##_set_t;                                                \
                                                                        \
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
    return (Name##_set.bitmap[i / 8] & (1 << (i % 8))) ? FALSE : TRUE;  \
  }                                                                     \
                                                                        \
  inline void                                                           \
  olsr_##Name##_set_declare_empty_(int i)                               \
  {                                                                     \
    Name##_set.bitmap[i / 8] &= ~(1 << (i % 8));                        \
  }                                                                     \
                                                                        \
  inline void                                                           \
  olsr_##Name##_set_declare_used_(int i)                                \
  {                                                                     \
    Name##_set.bitmap[i / 8] |= (1 << (i % 8));                         \
  }

# define SET_DECLARE(Name, MaxSize)             \
  SET_DECLARE_(Name, Name, MaxSize)

# define SET_IMPLEMENT_(Name, Tuple, MaxSize, ...)                      \
  olsr_##Name##_set_t Name##_set;                                       \
                                                                        \
  void                                                                  \
  olsr_##Name##_set_init_()                                             \
  {                                                                     \
    Name##_set.max_size = MaxSize;                                      \
    Name##_set.full = FALSE;                                            \
    Name##_set.n_tuples = 0;                                            \
    Name##_set.bitmap_size = DIVUP(MaxSize, 8);                         \
    Name##_set.first_empty = 0;                                         \
    memset(Name##_set.bitmap, 0, Name##_set.bitmap_size);               \
    memset(Name##_set.tuples, 0,                                        \
           MaxSize * sizeof(olsr_##Tuple##_tuple_t));                   \
    SET_FOREACH_(Name, Tuple, tuple, olsr_##Tuple##_tuple_init(tuple)); \
    __VA_ARGS__                                                         \
  }                                                                     \
                                                                        \
  void olsr_##Name##_set_empty_()                                       \
  {                                                                     \
    Name##_set.first_empty = 0;                                         \
    Name##_set.n_tuples = 0;                                            \
    Name##_set.full = FALSE;                                            \
    memset(Name##_set.bitmap, 0, Name##_set.bitmap_size);               \
  }                                                                     \
                                                                        \
  olsr_##Tuple##_tuple_t*                                               \
  olsr_##Name##_set_insert_(const olsr_##Tuple##_tuple_t* tuple)        \
  {                                                                     \
    const int old = Name##_set.first_empty;                             \
    if (Name##_set.full)                                                \
    {                                                                   \
      ERROR(#Name " set full! [max_size:%d]", Name##_set.max_size);     \
      SET_ABORT;                                                        \
      return NULL;                                                      \
    }                                                                   \
                                                                        \
    Name##_set.tuples[old] = *tuple;                                    \
    olsr_##Name##_set_declare_used_(old);                               \
    Name##_set.n_tuples++;                                              \
                                                                        \
    if (Name##_set.n_tuples == MaxSize)                                 \
    {                                                                   \
      Name##_set.first_empty = MaxSize;                                 \
      Name##_set.full = TRUE;                                           \
    }                                                                   \
    else                                                                \
    {                                                                   \
      for (int i = Name##_set.first_empty + 1; i < MaxSize; i++)        \
      {                                                                 \
        if (olsr_##Name##_set_is_empty_(i))                             \
        {                                                               \
          Name##_set.first_empty = i;                                   \
          break;                                                        \
        }                                                               \
      }                                                                 \
    }                                                                   \
                                                                        \
    return Name##_set.tuples + old;                                     \
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
    SET_FIND_(Name, Tuple, __tuple, cond);                              \
    return __tuple;                                                     \
  }

# define SET_IMPLEMENT(Name, MaxSize, ...)      \
  SET_IMPLEMENT_(Name, Name, MaxSize, __VA_ARGS__)

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

# define SET_REFRESH_TIME_MS 100

# define SET_SYNCHRO_DECLARE(Name)                                      \
  extern xSemaphoreHandle olsr_##Name##_set_mutex;                      \
  void olsr_##Name##_set_task(void* pvParameters);

# define SET_SYNCHRO_INIT(Name)                                         \
  olsr_##Name##_set_mutex = xSemaphoreCreateMutex();                    \
  xTaskCreate(olsr_##Name##_set_task,                                   \
              (signed portCHAR*) #Name "setTask",                       \
              configMINIMAL_STACK_SIZE, NULL,                           \
              tskIDLE_PRIORITY, NULL);

# define SET_SYNCHRO_IMPLEMENT(Name, MaxSize)           \
  xSemaphoreHandle olsr_##Name##_set_mutex;             \
  SET_IMPLEMENT(Name, MaxSize, SET_SYNCHRO_INIT(Name))

# define SET_MUTEX_TAKE(Name)                                   \
  xSemaphoreTake(olsr_##Name##_set_mutex, portMAX_DELAY)

# define SET_MUTEX_GIVE(Name)                   \
  xSemaphoreGive(olsr_##Name##_set_mutex)

# define SET_SYNCHRO_DEFAULT_TASK(Name, TimeField)                      \
  void olsr_##Name##_set_task(void* pvParameters)                       \
  {                                                                     \
    portTickType xLastWakeTime;                                         \
    for (;;)                                                            \
    {                                                                   \
      vTaskDelayUntil(&xLastWakeTime, SET_REFRESH_TIME_MS               \
                      / portTICK_RATE_MS);                              \
                                                                        \
      if (Name##_set.n_tuples == 0)                                     \
        continue;                                                       \
                                                                        \
      SET_MUTEX_TAKE(Name);                                             \
                                                                        \
      SET_FOREACH(Name, Name,                                           \
                  if (Name->TimeField >= olsr_get_current_time())       \
                    olsr_##Name##_set_delete(__i_##Name));              \
                                                                        \
      SET_MUTEX_GIVE(Name);                                             \
                                                                        \
    }                                                                   \
  }

# define SET_SYNCHRO_DEFAULT_IMPLEMENT(Name, MaxSize, TimeField)        \
  SET_SYNCHRO_IMPLEMENT(Name, MaxSize)                                  \
  SET_SYNCHRO_DEFAULT_TASK(Name, TimeField)

#endif
