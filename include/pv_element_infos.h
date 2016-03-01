#ifndef __PV_ELEMENT_DATAS_H__
#define __PV_ELEMENT_DATAS_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_element_general.h"

typedef gpointer (*PvElementFuncNewData)(void);
typedef bool (*PvElementFuncDeleteData)(void *data);
typedef gpointer (*PvElementFuncCopyNewData)(void *data);

typedef struct _PvElementInfo{
	PvElementKind kind;
	const char *name;
	PvElementFuncNewData		func_new_data;
	PvElementFuncDeleteData		func_delete_data;
	PvElementFuncCopyNewData	func_copy_new_data;
}PvElementInfo;

extern const PvElementInfo _pv_element_infos[];

const PvElementInfo *pv_element_get_info_from_kind(PvElementKind kind);

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __PV_ELEMENT_DATAS_H__