#include <string.h>
#include <stdio.h>

#include "dwarf.h"
#include "libdwarf.h"

char *makename(char *s)
{
    char *newstr;

    if (!s) {
			return (char *)"";
    }

    newstr = strdup(s);
    if (newstr == 0) {
        fprintf(stderr, "Out of memory mallocing %d bytes\n",
                (int) strlen(s));
				//        exit(1);
    }
    return newstr;
}

static const char *
skipunder(const char *v)
{
     const char *cp = v;
     int undercount = 0;
     for(  ; *cp ; ++cp) {
        if( *cp == '_') {
             ++undercount;
             if(undercount == 2) {
                  return cp+1;
             }
        }
     }
     return "";
}

static const char *
ellipname(int res, int val_in, const char *v,const char *ty)
{
    if(res != DW_DLV_OK) {
        char buf[100];
        char *n;
        snprintf(buf,sizeof(buf),"<Unknown %s value 0x%x>",ty,val_in);
				fprintf(stderr,"%s of %d (0x%x) is unknown to dwarfdump. "
								"Continuing. \n",ty,val_in,val_in );
        n = makename(buf);
        return n;
    }
    return skipunder(v);
}

const char * get_TAG_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_TAG_name(val_in,&v);
   return ellipname(res,val_in,v,"TAG");
}
const char * get_children_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_children_name(val_in,&v);
   return ellipname(res,val_in,v,"children");
}
const char * get_FORM_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_FORM_name(val_in,&v);
   return ellipname(res,val_in,v,"FORM");
}
const char * get_AT_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_AT_name(val_in,&v);
   return ellipname(res,val_in,v,"AT");
}
const char * get_OP_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_OP_name(val_in,&v);
   return ellipname(res,val_in,v,"OP");
}
const char * get_ATE_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_ATE_name(val_in,&v);
   return ellipname(res,val_in,v,"ATE");
}
const char * get_DS_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_DS_name(val_in,&v);
   return ellipname(res,val_in,v,"DS");
}
const char * get_END_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_END_name(val_in,&v);
   return ellipname(res,val_in,v,"END");
}
const char * get_ATCF_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_ATCF_name(val_in,&v);
   return ellipname(res,val_in,v,"ATCF");
}
const char * get_ACCESS_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_ACCESS_name(val_in,&v);
   return ellipname(res,val_in,v,"ACCESS");
}
const char * get_VIS_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_VIS_name(val_in,&v);
   return ellipname(res,val_in,v,"VIS");
}
const char * get_VIRTUALITY_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_VIRTUALITY_name(val_in,&v);
   return ellipname(res,val_in,v,"VIRTUALITY");
}
const char * get_LANG_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_LANG_name(val_in,&v);
   return ellipname(res,val_in,v,"LANG");
}
const char * get_ID_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_ID_name(val_in,&v);
   return ellipname(res,val_in,v,"ID");
}
const char * get_CC_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_CC_name(val_in,&v);
   return ellipname(res,val_in,v,"CC");
}
const char * get_INL_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_INL_name(val_in,&v);
   return ellipname(res,val_in,v,"INL");
}
const char * get_ORD_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_ORD_name(val_in,&v);
   return ellipname(res,val_in,v,"ORD");
}
const char * get_DSC_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_DSC_name(val_in,&v);
   return ellipname(res,val_in,v,"DSC");
}
const char * get_LNS_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_LNS_name(val_in,&v);
   return ellipname(res,val_in,v,"LNS");
}
const char * get_LNE_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_LNE_name(val_in,&v);
   return ellipname(res,val_in,v,"LNE");
}
const char * get_MACINFO_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_MACINFO_name(val_in,&v);
   return ellipname(res,val_in,v,"MACINFO");
}
const char * get_CFA_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_CFA_name(val_in,&v);
   return ellipname(res,val_in,v,"CFA");
}
const char * get_EH_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_EH_name(val_in,&v);
   return ellipname(res,val_in,v,"EH");
}
const char * get_FRAME_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_FRAME_name(val_in,&v);
   return ellipname(res,val_in,v,"FRAME");
}
const char * get_CHILDREN_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_CHILDREN_name(val_in,&v);
   return ellipname(res,val_in,v,"CHILDREN");
}
const char * get_ADDR_name(unsigned int val_in)
{
   const char *v = 0;
   int res = dwarf_get_ADDR_name(val_in,&v);
   return ellipname(res,val_in,v,"ADDR");
}


