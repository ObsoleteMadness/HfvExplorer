#include <stdlib.h>

#include "go.h"
#include "bndl.h"

#include "initicons.proto.h"

enum { HASH_SIZE = ICONTABLESIZE };

PRIVATE type_creator_link_t *hash_table[HASH_SIZE];

PRIVATE unsigned long
hash_func (OSType type, OSType creator)
{
  unsigned long retval;

  retval = ((unsigned long) type + creator) % HASH_SIZE;
  return retval;
}

PRIVATE type_creator_link_t **
hashpp (OSType type, OSType creator)
{
  type_creator_link_t **retval;

  for (retval = &hash_table[hash_func (type, creator)];
       *retval && ((*retval)->type != type || (*retval)->creator != creator);
       retval = &(*retval)->next)
    ;
  return retval;
}

PRIVATE void
insert_into_hash_table (OSType type, OSType creator, icn_sharp_hand icon)
{
  if (icon)
    {
      type_creator_link_t **linkpp;
      
      linkpp = hashpp (type, creator);
      if (!*linkpp) /* currently we don't overwrite an existing match */
        {
          type_creator_link_t *newlink;

          newlink = malloc (sizeof (*newlink));
          if (newlink)
            {
              newlink->type = type;
              newlink->creator = creator;
              newlink->icon = icon;
              newlink->next = 0;
              *linkpp = newlink;
            }
        }
    }
}

PRIVATE bndl_section_t *
extract_bndl_section (bndl_t **bndl_h, ResType code)
{
  bndl_section_t *retval;
  int i, n_sects;

  n_sects = (*bndl_h)->n_sections_minus_1 + 1;
  for (i = 0, retval = (*bndl_h)->section;
       i < n_sects && retval->code != code;
       ++i)
    {
      int n_mappings;
      
      n_mappings = retval->n_mappings_minus_1 + 1;
      retval = (bndl_section_t *)
        ((char *)retval + sizeof (bndl_section_t)
                        + (n_mappings-1) * sizeof (local_mapping_t));
                         /*^^^^^^^^^^^^*/
                         /* we subtract one ANSI doesn't allow zero sized
                            arrays */
    }
  if (i >= n_sects)
    retval = 0;
  return retval;
}

/* still need read_icn_sharp */

PRIVATE fref_hand
get_fref (INTEGER id)
{
  fref_hand retval;

  retval = (fref_hand) GetResource('FREF', id);
  if (retval)
    {
      LoadResource ((Handle) retval);
      if (GetHandleSize ((Handle) retval) < sizeof (**retval))
        retval = 0;
    }
  return retval;
}

PRIVATE icn_sharp_hand
find_local_icon (INTEGER local_id, local_icn_sharp_t local_icns[],
                 INTEGER n_icn)
{
  icn_sharp_hand retval;
  int i;
  
  for (i = 0; i < n_icn && local_icns[i].local_id != local_id; ++i)
    ;
  retval = i < n_icn ? local_icns[i].icon : 0;
  return retval;
}

PUBLIC void
process_bndl (bndl_t **bndl_h, OSType creator)
{
  SignedByte state;

  if (bndl_h)
    {
	  state = HGetState ((Handle) bndl_h);
	  HLock ((Handle) bndl_h);
	  {
	    bndl_section_t *sectp;
	    int n_icn, n_fref;
	    local_icn_sharp_t *local_icns;
	    int i;
	
	    sectp = extract_bndl_section (bndl_h, 'ICN#');
	    if (sectp)
	      {
		    n_icn = sectp->n_mappings_minus_1 + 1;
		    local_icns = malloc (n_icn * sizeof (*local_icns));
		    for (i = 0; i < n_icn ; ++i)
		      {
		        local_icns[i].local_id = sectp->mapping[i].local_id;
		        local_icns[i].icon = read_icn_sharp (sectp->mapping[i].resource_id);
		      }
		    sectp = extract_bndl_section (bndl_h, 'FREF');
		    if (sectp)
		      {
			    n_fref = sectp->n_mappings_minus_1 + 1;
			    for (i = 0; i < n_fref ; ++i)
			      {
			        icn_sharp_hand icon;
			        fref_hand fref_h;
			
			        fref_h = get_fref (sectp->mapping[i].resource_id);
			        if (fref_h)
			          {
			            icon = find_local_icon ((*fref_h)->local_id, local_icns, n_icn);
			            insert_into_hash_table ((*fref_h)->type, creator, icon);
			          }
			      }
			    }
		      free (local_icns);
		    }
	  }
	  HSetState ((Handle) bndl_h, state);
	}
}

PUBLIC icn_sharp_hand
map_type_and_creator (OSType type, OSType creator)
{
  type_creator_link_t **linkpp;
  icn_sharp_hand retval;

  linkpp = hashpp (type, creator);
  retval = (*linkpp) ? (*linkpp)->icon : 0;
  return retval;
}
