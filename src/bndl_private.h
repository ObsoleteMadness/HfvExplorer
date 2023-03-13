#if !defined(_BNDL_PRIVATE_H_)
#define _BNDL_PRIVATE_H_

// #include "mytype.h"
// #include "iconcontrol.h"

#if !defined(PRIVATE)
#define PRIVATE static
#endif

#if !defined(PUBLIC)
#define PUBLIC
#endif

typedef struct
{
  INTEGER local_id;
  INTEGER resource_id;
} local_mapping_t;

typedef struct
{
  ResType code;
  INTEGER n_mappings_minus_1;
  local_mapping_t mapping[1]; /* would like to put 0 here */
} bndl_section_t;

typedef struct
{
  INTEGER local_id;
  icn_sharp_hand icon;
} local_icn_sharp_t;

typedef struct type_creator_link_str
{
  struct type_creator_link_str *next;
  OSType type;
  OSType creator;
  icn_sharp_hand icon;
} type_creator_link_t;

typedef struct
{
  OSType type;
  INTEGER local_id;
  /* more, but we don't care about it */
} fref_t, **fref_hand;
#endif
