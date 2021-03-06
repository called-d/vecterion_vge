#include "pv_element.h"

#include <stdlib.h>
#include <string.h>
#include "pv_error.h"
#include "pv_element_info.h"

static bool _pv_element_free_single(PvElement *self);

char *pv_general_str_new(const char * const src)
{
	if(NULL == src){
		pv_error("");
		return NULL;
	}

	char *dst = (char *)malloc(sizeof(char) * (strlen(src) + 1));
	if(NULL == dst){
		pv_critical("");
		exit(-1);
	}

	strcpy(dst, src);

	return dst;
}

static bool _pv_element_recursive_inline_2(PvElement *element,
		PvElementRecursiveFunc func_before,
		PvElementRecursiveFunc func_after,
		gpointer data,
		bool is_asc,
		int *level,
		PvElementRecursiveError *error)
{
	if(NULL != func_before){
		if(!func_before(element, data, *level)){
			// cancel function self childs
			return true;
		}
	}

	(*level)++;

	bool ret = true;

	int num = pv_general_get_parray_num((void **)element->childs);
	for(int i = 0; i < num; i++){
		int ix = 0;
		if(is_asc){
			ix = i;
		}else{
			ix = ((num - 1) - i);
		}
		if(!_pv_element_recursive_inline_2(element->childs[ix],
					func_before,
					func_after,
					data,
					is_asc,
					level,
					error)){
			if(!error->is_error){
				// logging first error.
				error->is_error = true;
				error->level = *level;
				error->element = element->childs[ix];
			}
			ret = false;
			goto end;
		}
	}

end:
	(*level)--;

	if(NULL != func_after){
		if(!func_after(element, data, *level)){
			// self point "return false" is not mean.
			return true;
		}
	}

	return ret;
}

static bool _pv_element_recursive_inline_1(PvElement *element,
		PvElementRecursiveFunc func_before,
		PvElementRecursiveFunc func_after,
		gpointer data,
		bool is_asc,
		PvElementRecursiveError *error)
{
	if(NULL == element){
		pv_error("");
		return false;
	}
	if(NULL == func_before && NULL == func_after){
		pv_error("");
		return false;
	}
	if(NULL == error){
		pv_error("");
		return false;
	}

	// initialize.
	error->is_error = false;
	error->level = -1;
	error->element = NULL;

	int level = 0;
	bool ret = _pv_element_recursive_inline_2(element,
			func_before,
			func_after,
			data,
			is_asc,
			&level,
			error);

	return ret;
}

bool pv_element_recursive_asc(PvElement *element,
		PvElementRecursiveFunc func_before,
		PvElementRecursiveFunc func_after,
		gpointer data,
		PvElementRecursiveError *error)
{
	return _pv_element_recursive_inline_1(element,
			func_before,
			func_after,
			data,
			true,
			error);
}

bool pv_element_recursive_desc(PvElement *element,
		PvElementRecursiveFunc func_before,
		PvElementRecursiveFunc func_after,
		gpointer data,
		PvElementRecursiveError *error)
{
	return _pv_element_recursive_inline_1(element,
			func_before,
			func_after,
			data,
			false,
			error);
}

bool pv_element_recursive_desc_before(PvElement *element,
		PvElementRecursiveFunc func_before,
		gpointer data,
		PvElementRecursiveError *error)
{
	if(NULL == func_before){
		pv_error("");
		return false;
	}

	return pv_element_recursive_desc(element,
			func_before,
			NULL,
			data,
			error);
}

PvElement *pv_element_new(const PvElementKind kind)
{
	PvElement *self = (PvElement *)malloc(sizeof(PvElement));
	pv_assert(self);

	self->parent = NULL;
	self->childs = NULL;

	const PvElementInfo *info = pv_element_get_info_from_kind(kind);
	pv_assertf(info, "%d", kind);
	pv_assertf(info->func_new_data, "%d", kind);

	self->data = info->func_new_data();
	pv_assertf(self->data, "%d", kind);

	self->color_pair = PvColorPair_Default;
	self->stroke = PvStroke_Default;

	self->kind = kind;

	self->etaion_work_appearances = pv_appearance_parray_new_from_num(NUM_WORK_APPEARANCE + 1);
	pv_assert(self->etaion_work_appearances);

	return self;
}

void pv_element_free(PvElement *element)
{
	pv_assert(element);
	const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
	pv_assert(info);
	pv_assert(element->data);
	info->func_free_data(element->data);

	free(element);
}

static PvElement *_pv_element_copy_single(const PvElement *self)
{
	pv_assert(self);

	PvElement *new_element = (PvElement *)malloc(sizeof(PvElement));
	pv_assert(new_element);

	*new_element = *self;
	new_element->parent = NULL;
	new_element->childs = NULL;

	const PvElementInfo *info = pv_element_get_info_from_kind(self->kind);
	pv_assertf(info, "%d", self->kind);

	new_element->data = info->func_copy_new_data(self->data);
	pv_assertf(new_element->data, "%d", self->kind);

	if(NULL == self->etaion_work_appearances){
		new_element->etaion_work_appearances = NULL;
	}else{
		new_element->etaion_work_appearances
			= pv_appearance_parray_copy_new(self->etaion_work_appearances);
		pv_assert(new_element->etaion_work_appearances);
	}

	return new_element;
}

static PvElement *_pv_element_copy_recursive_inline(const PvElement *self,
		int *level, PvElementRecursiveError *error)
{
	if(NULL == self){
		error->is_error =true;
		error->level = *level;
		error->element = self;
		return NULL;
	}

	// copy self
	PvElement *new_element = _pv_element_copy_single(self);
	if(NULL == new_element){
		error->is_error =true;
		error->level = *level;
		error->element = self;
		return NULL;
	}

	// ** copy childs
	(*level)++;

	int num = pv_general_get_parray_num((void **)self->childs);
	if(0 > num){
		error->is_error = true;
		error->level = *level;
		error->element = self;
		goto error1;
	}

	// pv_debug("copy childs:%3d:%d", *level, num);

	PvElement **childs = NULL;
	if(0 < num){
		childs = malloc((num + 1) * sizeof(PvElement *));
		if(NULL == childs){
			error->is_error = true;
			error->level = *level;
			error->element = self;
			goto error1;
		}

		for(int i = 0; i < num; i++){
			PvElement *child = _pv_element_copy_recursive_inline(
					self->childs[i],
					level,
					error);
			if(NULL == child){
				if(!error->is_error){
					error->is_error =true;
					error->level = *level;
					error->element = self;
					goto error2;
				}
			}
			child->parent = new_element;
			childs[i] = child;
			childs[i + 1] = NULL;
		}

	}
	new_element->childs = childs;

	(*level)--;

	return new_element;

error2:
	pv_element_remove_free_recursive(new_element);

	return NULL;

error1:
	_pv_element_free_single(new_element);

	return NULL;
}

//! @detail element->parent of copied is NULL.
PvElement *pv_element_copy_recursive(const PvElement *self)
{
	if(NULL == self){
		pv_error("");
		return NULL;
	}

	PvElementRecursiveError error = {
		.is_error = false,
		.level = -1,
		.element = NULL,
	};
	int level = 0;
	PvElement *new_element_tree = _pv_element_copy_recursive_inline( self, &level, &error);
	if(NULL == new_element_tree){
		pv_error("");
		return NULL;
	}

	return new_element_tree;
}

/*! @brief 
 *
 * @param parent
 *		NULL: return Error.
 *		Not Layer(Group): return Error.
 * @param prev
 *		NULL: append toplevel element in parent.
 *		Element: append under the prev.
 */
bool pv_element_append_child(PvElement *parent, const PvElement *prev, PvElement *element)
{
	if(NULL == parent){
		pv_bug("");
		return false;
	}
	if(NULL == element){
		pv_bug("");
		return false;
	}

	int num = pv_general_get_parray_num((void **)parent->childs);
	if(0 > num){
		pv_bug("");
		return false;
	}

	PvElement **childs = (PvElement **)realloc(parent->childs,
			sizeof(PvElement*) * (num + 2));
	if(NULL == childs){
		pv_error("");
		return false;
	}
	if(NULL == prev){
		childs[num] = element;
	}else{
		bool isExist = false;
		for(int i = 0; i < num; i++){
			if(prev == childs[i]){
				isExist = true;
				memmove(&childs[i + 1], &childs[i], sizeof(PvElement*) * (num - i));
				childs[i] = element;
				break;
			}
		}
		if(!isExist){
			pv_error("");
			return false;
		}
	}
	childs[num + 1] = NULL;

	parent->childs = childs;
	element->parent = parent;

	return true;
}

bool pv_element_append_on_focusing(PvElement *focusing_element, PvElement *element)
{
	PvElement *parent = NULL;
	PvElement *sister = NULL;
	switch(focusing_element->kind){
		case PvElementKind_Root:
		case PvElementKind_Layer:
		case PvElementKind_Group:
			{
				parent = focusing_element;
				sister = NULL;
			}
			break;
		default:
			{
				pv_assert(focusing_element->parent);
				parent = focusing_element->parent;
				sister = focusing_element;
			}
			break;
	}

	return pv_element_append_child(parent, sister, element);
}

PvElement *pv_element_get_in_elements_from_member_anchor_point(
		PvElement **elements,
		const PvAnchorPoint *anchor_point)
{
	size_t num = pv_general_get_parray_num((void **)elements);
	for(int i = 0; i < (int)num; i++){
		PvElement *element = elements[i];

		const PvElementInfo *info = pv_element_get_info_from_kind(element->kind);
		pv_assert(info);

		if(info->func_is_exist_anchor_point(element, anchor_point)){
			return element;
		}
	}

	return NULL;
}

static bool _pv_element_free_single(PvElement *self)
{
	if(NULL == self){
		pv_bug("");
		return false;
	}

	const PvElementInfo *info = pv_element_get_info_from_kind(self->kind);
	if(NULL == info){
		pv_bug("");
		return false;
	}

	if(!info->func_free_data(self->data)){
		pv_bug("");
		return false;
	}

	free(self);

	return true;
}

static bool _pv_element_remove_free_recursive_inline(PvElement *self,
		int *level, PvElementRecursiveError *error)
{
	int num = pv_general_get_parray_num((void **)self->childs);
	if(0 > num){
		pv_bug("");
		return false;
	}

	(*level)++;

	// pv_debug("delete childs:%3d:%d", *level, num);

	for(int i = (num - 1); 0 <= i; i--){
		if(!_pv_element_remove_free_recursive_inline(
					self->childs[i],
					level,
					error)){
			if(!error->is_error){
				error->is_error =true;
				error->level = *level;
				error->element = self->childs[i];
				break;
			}
		}
	}

	(*level)--;

	if(!_pv_element_free_single(self)){
		error->is_error =true;
		error->level = *level;
		error->element = self;
		return false;
	}

	return true;
}

static bool _pv_element_detouch_parent(PvElement * const self)
{
	if(NULL == self){
		pv_error("");
		return false;
	}

	if(NULL == self->parent){
		pv_error("");
		return false;
	}

	PvElement **childs = self->parent->childs;
	int num = pv_general_get_parray_num((void **)childs);
	bool is_exist = false;
	for(int i = 0; i < num; i++){
		if(self == childs[i]){
			is_exist = true;
			memmove(&childs[i], &childs[i + 1], sizeof(PvElement*) * (num - i));
			break;
		}
	}
	if(!is_exist){
		pv_error("");
		return false;
	}

	if(0 == num){
		free(childs);
		childs = NULL;
	}else{
		childs = (PvElement **)realloc(childs,
				sizeof(PvElement*) * (num));
		childs[num - 1] = NULL;
	}

	self->parent->childs = childs;
	self->parent = NULL;

	return true;
}

/*! @detail
 * element remove in parent->childs
 * delete element and childs recursive.
 *
 * @todo この関数自体の問題ではないが、この関数を呼び出した側の注意不足で、
 *	rootを除去してしまったり、rootのchildsを空にしてしまったりするが、
 *	どのレベルでセーフティをかける？
 */
bool pv_element_remove_free_recursive(PvElement *self)
{
	if(NULL == self){
		pv_error("");
		return false;
	}

	// 親がいるなら連結を外す
	if(NULL != self->parent){
		if(!_pv_element_detouch_parent(self)){
			pv_error("");
			return false;
		}
	}

	PvElementRecursiveError error = {
		.is_error = false,
		.level = -1,
		.element = NULL,
	};
	int level = 0;
	if(!_pv_element_remove_free_recursive_inline(self, &level, &error)){
		pv_error("%d\n", error.level);
		return false;
	}

	return true;
}

void pv_element_copy_property(PvElement *dst, PvElement *src)
{
	dst->color_pair = src->color_pair;
	dst->stroke = src->stroke;
}

typedef struct PvElementIsDiffRecursiveData{
	bool is_diff;
}PvElementIsDiffRecursiveData;

static bool _pv_element_is_diff_one(
		PvElement *element0,
		PvElement *element1,
		PvElementIsDiffRecursiveData *data)
{
	if(NULL == element0 || NULL == element1){
		pv_error("%p,%p", element0, element1);
		goto error;
	}

	if(element0->kind != element1->kind){
		data->is_diff = true;
		return true;
	}

	if(!pv_color_pair_is_equal(element0->color_pair, element1->color_pair)){
		data->is_diff = true;
		return true;
	}

	if(!pv_stroke_is_equal(element0->stroke, element1->stroke)){
		data->is_diff = true;
		return true;
	}

	int num0 = pv_general_get_parray_num((void **)element0->childs);
	int num1 = pv_general_get_parray_num((void **)element1->childs);
	if(num0 != num1){
		data->is_diff = true;
		return true;
	}

	const PvElementInfo *info = pv_element_get_info_from_kind(element0->kind);
	if(NULL == info || NULL == info->func_is_diff_one){
		pv_bug("");
		goto error;
	}

	if(!info->func_is_diff_one(&(data->is_diff), element0, element1)){
		pv_bug("%d", element0->kind);
		goto error;
	}

	return true;
error:
	return false;
}

static bool _pv_element_is_diff_recursive_inline(
		PvElement *element0,
		PvElement *element1,
		gpointer data,
		int *level,
		PvElementRecursiveError *error)
{
	bool ret = true;
	PvElementIsDiffRecursiveData *_data = data;

	if(!_pv_element_is_diff_one(element0, element1, _data)){
		pv_error("");
		error->is_error = true;
		error->level = *level;
		error->element = element0;
		return false;
	}
	if(_data->is_diff){
		return false;
	}

	int num0 = pv_general_get_parray_num((void **)element0->childs);
	int num1 = pv_general_get_parray_num((void **)element1->childs);
	if(num0 != num1){
		// pv_debug("level:%d num:%d,%d", *level, num0, num1);
		_data->is_diff = true;
		return false;
	}else{
		(*level)++;
		for(int i = 0; i < num0; i++){
			ret = _pv_element_is_diff_recursive_inline(
					element0->childs[i],
					element1->childs[i],
					data,
					level,
					error);
			if(!ret){
				break;
			}
			if(error->is_error){
				ret = false;
				break;
			}
		}
		(*level)--;
	}

	return ret;
}

//! @todo argument to const
bool pv_element_is_diff_recursive(PvElement *element0, PvElement *element1)
{
	if(element0 == element1){
		pv_debug("");
		return true;
	}

	PvElementIsDiffRecursiveData _data = {
		.is_diff = false,
	};
	PvElementRecursiveError error = PvElementRecursiveError_Default;
	int level = 0;
	_pv_element_is_diff_recursive_inline(
			element0,
			element1,
			&_data,
			&level,
			&error);
	if(error.is_error){
		pv_error("%d\n", error.level);
		return false;
	}

	return _data.is_diff;
}

PvElement *pv_element_curve_new_from_rect(PvRect rect)
{
	PvElement *self = pv_element_new(PvElementKind_Curve);
	pv_assert(self);

	for(int i = 0; i < 4; i++){
		PvPoint point = pv_rect_get_edge_point(rect, i);
		PvAnchorPoint ap = pv_anchor_point_from_point(point);
		bool ret = pv_element_curve_add_anchor_point(self, ap);
		pv_assert(ret);
	}
	pv_element_curve_set_close_anchor_point(self, true);

	return self;
}

PvElement *pv_element_curve_new_set_anchor_path(PvAnchorPath *anchor_path)
{
	PvElement *element = pv_element_new(PvElementKind_Curve);
	pv_assert(element);

	PvElementCurveData *data = element->data;
	pv_anchor_path_free(data->anchor_path);
	data->anchor_path = anchor_path;

	return element;
}

PvElement *pv_element_curve_new_set_anchor_point(PvAnchorPoint *anchor_point)
{
	PvElement *element = pv_element_new(PvElementKind_Curve);
	pv_assert(element);

	pv_element_curve_append_anchor_point(element, anchor_point, -1);

	return element;
}

void pv_element_curve_append_anchor_point(PvElement *element, PvAnchorPoint *anchor_point, int index)
{
	PvElementCurveData *data = element->data;
	pv_anchor_path_append_anchor_point(data->anchor_path, anchor_point, index);
}

bool pv_element_curve_add_anchor_point(PvElement *self, const PvAnchorPoint anchor_point)
{
	pv_assert(self);
	pv_assert(self->data);

	if(PvElementKind_Curve != self->kind){
		pv_bug("");
		return false;
	}

	PvElementCurveData *data = self->data;
	pv_anchor_path_add_anchor_point(data->anchor_path, &anchor_point);

	return true;
}

int pv_element_curve_get_num_anchor_point(const PvElement *self)
{
	assert(self);
	assert(self->data);
	assert(PvElementKind_Curve == self->kind);

	PvElementCurveData *data = self->data;
	return pv_anchor_path_get_anchor_point_num(data->anchor_path);
}

void pv_element_curve_set_close_anchor_point(PvElement *self, bool is_close)
{
	PvElementCurveData *data = self->data;
	pv_anchor_path_set_is_close(data->anchor_path, is_close);
}

static bool pv_element_raster_read_file(PvElement *self, const char *path)
{
	if(PvElementKind_Raster != self->kind){
		pv_bug("");
		return false;
	}

	if(NULL == self->data){
		pv_bug("");
		return false;
	}

	PvElementRasterData *data = self->data;
	if(NULL != data->path){
		pv_fixme("");
		return false;
	}
	data->path = pv_general_str_new(path);
	if(NULL == data->path){
		pv_error("");
		return false;
	}
	data->pixbuf = gdk_pixbuf_new_from_file(data->path, NULL);
	if(NULL == data->pixbuf){
		pv_warning("%s\n", data->path);
		return false;
	}

	return true;
}

PvElement *pv_element_raster_new_from_filepath(const char *filepath)
{
	PvElement *self = pv_element_new(PvElementKind_Raster);
	if(NULL == self){
		pv_error("");
		return NULL;
	}

	if(! pv_element_raster_read_file(self, filepath)){
		_pv_element_free_single(self);
		return NULL;
	}

	return self;
}

const char *pv_element_get_name_from_kind(PvElementKind kind)
{
	const PvElementInfo *info = pv_element_get_info_from_kind(kind);
	if(NULL == info){
		pv_error("");
		return NULL;
	}

	return info->name;
}

bool pv_element_kind_is_viewable_object(PvElementKind kind)
{
	switch(kind){
		case PvElementKind_NotDefined:
		case PvElementKind_Root:
		case PvElementKind_Layer:
		case PvElementKind_Group:
		case PvElementKind_EndOfKind:
			return false;
		case PvElementKind_Curve:
		case PvElementKind_Raster:
			return true;
		default:
			pv_bug("");
			return false;
	}
}

bool pv_element_kind_is_object(PvElementKind kind)
{
	switch(kind){
		case PvElementKind_NotDefined:
		case PvElementKind_Root:
		case PvElementKind_Layer:
		case PvElementKind_EndOfKind:
			return false;
		case PvElementKind_Group:
		case PvElementKind_Curve:
		case PvElementKind_Raster:
			return true;
		default:
			pv_bug("");
			return false;
	}
}

void pv_element_debug_print(const PvElement *element)
{
	if(NULL == element){
		pv_debug("");
		return;
	}
	if(PvElementKind_Curve != element->kind){
		pv_debug("%d", element->kind);
		return;
	}


	PvElementCurveData *data = element->data;
	if(NULL == data){
		pv_debug("");
		return;
	}

	pv_anchor_path_debug_print(data->anchor_path);
}

