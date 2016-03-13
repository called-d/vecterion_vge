#ifndef include_PV_ELEMENT_H
#define include_PV_ELEMENT_H
/** ******************************
 * @brief PhotonVector Vector Graphics Format.
 *
 ****************************** */

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdbool.h>
#include "pv_element_general.h"


struct PvElement;
typedef struct PvElement PvElement;
struct PvElement{
	PvElement *parent;
	PvElement **childs; // I know "children".

	PvElementKind kind;
	// kind固有の情報を格納した型のオブジェクト
	gpointer data;
};

/**
 * use pv_element_recursive_desc_before();
 * @return false: cancel recursive(search childs).
 * false is not error.
 * (this is no tracking "*error" is true.
 *  please use your own "data" struct.)
 */
typedef bool (*PvElementRecursiveFunc)(PvElement *element, gpointer data, int level);
typedef struct PvElementRecursiveError{
	bool is_error;
	int level;
	const PvElement *element;
}PvElementRecursiveError;


PvElement *pv_element_new(const PvElementKind kind);
/** @brief 
 * copy element_tree parent is NULL.
 */
PvElement *pv_element_copy_recursive(const PvElement *this);
/** @brief
 * @return false: error from this function.
 */
bool pv_element_recursive_desc_before(PvElement *element,
		PvElementRecursiveFunc func, gpointer data,
		PvElementRecursiveError *error);
bool pv_element_recursive_desc(PvElement *element,
		PvElementRecursiveFunc func_before,
		PvElementRecursiveFunc func_after,
		gpointer data,
		PvElementRecursiveError *error);
/** @brief 
 *
 * @param parent
 *		NULL: return Error.
 *		Not Layer(Group): return Error.
 * @param prev
 *		NULL: append toplevel element in parent.
 *		Element: append under the prev.
 */
bool pv_element_append_child(PvElement * const parent,
		PvElement * const prev, PvElement * const element);
/** @brief
 * element remove in parent->childs
 * delete element and childs recursive.
 */
bool pv_element_remove_delete_recursive(PvElement * const this);

const char *pv_element_get_name_from_kind(PvElementKind kind);


bool pv_element_raster_read_file(PvElement * const this,
		const char * const path);

/** @brief 
 *
 * @param this
 *		Not Bezier: return Error.
 */
bool pv_element_bezier_add_anchor_point(PvElement * const this,
		const PvAnchorPoint anchor_point);

/** @brief set handle from graphic point.
 * @param ap_index PvAnchorPointIndex_Point is next and mirror reverse handle prev.
 */
void pv_element_bezier_anchor_point_set_handle(PvAnchorPoint *ap,
		PvAnchorPointIndex ap_index, PvPoint gpoint);

/** @brief
 *
 * @return PvPoint to graphic point.
 *		if rise error return value is not specitication.(ex. {0,0})
 */
PvPoint pv_anchor_point_get_handle(const PvAnchorPoint ap, PvAnchorPointIndex ap_index);

void pv_element_debug_print(const PvElement *element);

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_PV_ELEMENT_H
