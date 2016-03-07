#include "pv_svg_info.h"

#include <strings.h>
#include "pv_error.h"

PvElement *_pv_svg_svg_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		const ConfReadSvg *conf
		)
{
	return element_parent;
}

PvElement *_pv_svg_g_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		const ConfReadSvg *conf
		)
{
	// ** is exist groupmode=layer
	PvElementKind kind = PvElementKind_Group;
	xmlChar *value = NULL;
	/*
	   value = xmlGetNsProp(xmlnode,"groupmode",
	   "http://www.inkscape.org/namespaces/inkscape");
	   if(NULL == value){
	   value = xmlGetProp(xmlnode,"inkscape:groupmode");
	   }
	 */
	if(NULL == value){
		value = xmlGetProp(xmlnode, BAD_CAST "groupmode");
	}
	if(NULL != value){
		if(0 == strcasecmp("layer", (char*)value)){
			kind = PvElementKind_Layer;
		}
	}
	xmlFree(value);

	PvElement *element_new = pv_element_new(kind);
	if(NULL == element_new){
		pv_error("");
		return NULL;
	}
	if(!pv_element_append_child(element_parent, NULL, element_new)){
		pv_error("");
		return NULL;
	}

	return element_new;
}

void pv_element_anchor_point_init(PvAnchorPoint *ap)
{
	*ap = PvAnchorPoint_default;
}

const char *_pv_svg_read_args_from_str(double *args, int num_args, const char *str)
{
	const char *p = str;
	char *next;
	int i = 0;
	const char *str_error = NULL;
	while('\0' != *p){
		if(',' == *p){
			p++;
			continue;
		}
		if(!pv_general_strtod(&args[i], p, &next, &str_error)){
			break;
		}
		p = next;
		i++;
		if(!(i < num_args)){
			break;
		}
	}
	for(; i < num_args; i++){
		args[i] = 0;
	}

	return p;
}


void _pv_svg_fill_double_array(double *dst, double value, int size)
{
	for(int i = 0; i < size; i++){
		dst[i] = value;
	}
}

void _pv_svg_copy_double_array(double *dst, double *src, int size)
{
	for(int i = 0; i < size; i++){
		dst[i] = src[i];
	}
}

bool _pv_svg_path_set_anchor_points_from_str(PvElement *element, const char *str)
{
	const int num_args = 10;
	double args[num_args];
	double prev_args[num_args];
	_pv_svg_fill_double_array(args, 0, num_args);
	_pv_svg_fill_double_array(prev_args, 0, num_args);

	PvAnchorPoint ap;
	if(NULL == element || NULL == element->data){
		pv_error("");
		return false;
	}
	PvElementBezierData *data = element->data;

	double x_next_0 = 0, y_next_0 = 0;

	const char *p = str;
	while('\0' != *p){
		bool is_append = false;
		switch(*p){
			case 'M':
			case 'L':
				p = _pv_svg_read_args_from_str(args, 2, ++p);
				pv_element_anchor_point_init(&ap);
				ap.points[PvAnchorPointIndex_Point].x = args[0];
				ap.points[PvAnchorPointIndex_Point].y = args[1];
				is_append = true;
				break;
			case 'C':
				p = _pv_svg_read_args_from_str(args, 6, ++p);
				pv_element_anchor_point_init(&ap);
				ap.points[PvAnchorPointIndex_HandlePrev].x = args[2] - args[4];
				ap.points[PvAnchorPointIndex_HandlePrev].y = args[3] - args[5];
				ap.points[PvAnchorPointIndex_Point].x = args[4];
				ap.points[PvAnchorPointIndex_Point].y = args[5];
				ap.points[PvAnchorPointIndex_HandleNext].x = 0;
				ap.points[PvAnchorPointIndex_HandleNext].y = 0;
				if(0 < data->anchor_points_num){
					PvAnchorPoint *ap_prev = &data->anchor_points[data->anchor_points_num - 1];
					ap_prev->points[PvAnchorPointIndex_HandleNext].x = args[0];
					ap_prev->points[PvAnchorPointIndex_HandleNext].y = args[1];
				}else{
					x_next_0 = args[0];
					y_next_0 = args[1];
				}
				is_append = true;
				break;
			case 'S':
				p = _pv_svg_read_args_from_str(args, 4, ++p);
				pv_element_anchor_point_init(&ap);
				ap.points[PvAnchorPointIndex_HandlePrev].x = args[0] - args[2];
				ap.points[PvAnchorPointIndex_HandlePrev].y = args[1] - args[3];
				ap.points[PvAnchorPointIndex_Point].x = args[2];
				ap.points[PvAnchorPointIndex_Point].y = args[3];
				ap.points[PvAnchorPointIndex_HandleNext].x = 0;
				ap.points[PvAnchorPointIndex_HandleNext].y = 0;
				is_append = true;
				break;
			case 'Z':
			case 'z':
				p++;
				data->is_close = true;
				if(data->anchor_points_num < 1){
					break;
				}
				ap = data->anchor_points[0];
				ap.points[PvAnchorPointIndex_HandlePrev].x = x_next_0;
				ap.points[PvAnchorPointIndex_HandlePrev].y = y_next_0;
				is_append = true;
				break;
			case ' ':
			case ',':
				p++;
				break;
			default:
				p++;
		}

		if(is_append){
			if(!pv_element_bezier_add_anchor_point(element, ap)){
				pv_error("");
				return false;
			}
		}
	}

	return true;
}

PvElement *_pv_svg_path_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		const ConfReadSvg *conf
		)
{
	PvElement *element_new = pv_element_new(PvElementKind_Bezier);
	if(NULL == element_new){
		pv_error("");
		return NULL;
	}

	xmlChar *value = xmlGetProp(xmlnode, BAD_CAST "d");
	if(NULL != value){
		pv_debug("d='%s'\n", (char *)value);
		if(!_pv_svg_path_set_anchor_points_from_str(element_new, (char*)value)){
			pv_error("");
			return NULL;
		}

		pv_element_debug_print(element_new);
	}
	xmlFree(value);

	if(!pv_element_append_child(element_parent, NULL, element_new)){
		pv_error("");
		return NULL;
	}

	return element_new;
}

PvElement *_pv_svg_unknown_new_element_from_svg(
		PvElement *element_parent,
		xmlNodePtr xmlnode,
		bool *isDoChild,
		gpointer data,
		const ConfReadSvg *conf
		)
{
	pv_warning("Not implement:'%s'", xmlnode->name);

	return element_parent;
}

const PvSvgInfo _pv_svg_infos[] = {
	{
		.tagname = "svg",
		.func_new_element_from_svg = _pv_svg_svg_new_element_from_svg,
	},
	{
		.tagname = "g",
		.func_new_element_from_svg = _pv_svg_g_new_element_from_svg,
	},
	{
		.tagname = "path",
		.func_new_element_from_svg = _pv_svg_path_new_element_from_svg,
	},
	/* Unknown (or not implement) tag */
	{
		.tagname = NULL,
		.func_new_element_from_svg = _pv_svg_unknown_new_element_from_svg,
	},
};

const PvSvgInfo *pv_svg_get_svg_info_from_tagname(const char *tagname)
{
	const PvSvgInfo *info = &_pv_svg_infos[0];
	for(int i = 0; NULL != info->tagname; i++){
		if(0 == strcasecmp(tagname, info->tagname)){
			return info;
		}
		info = &_pv_svg_infos[i];
	}

	return info;
}

