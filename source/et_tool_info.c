#include "et_tool_info.h"

#include "et_error.h"
#include "pv_element_general.h"
#include "pv_element.h"
#include "pv_focus.h"
#include "et_doc.h"
#include "et_doc_manager.h"
#include "et_etaion.h"

bool _et_etaion_is_bound_point(int radius, PvPoint p1, PvPoint p2)
{
	if(
			(p1.x - radius) < p2.x
			&& p2.x < (p1.x + radius)
			&& (p1.y - radius) < p2.y
			&& p2.y < (p1.y + radius)
	  ){
		return true;
	}else{
		return false;
	}
}

bool _et_tool_nop_mouse_action(EtDocId id_doc, EtMouseAction mouse_action)
{
	switch(mouse_action.action){
		case EtMouseAction_Down:
		case EtMouseAction_Up:
			et_debug("\n");
			break;
		default:
			break;
	}

	return true;
}

static int _et_etaion_radius_path_detect = 6;
static EtMouseActionType _mouse_action_up_down = EtMouseAction_Up;
bool _et_tool_bezier_mouse_action(EtDocId id_doc, EtMouseAction mouse_action)
{
	EtDocId doc_id_current = et_etaion_get_current_doc_id();

	if(id_doc != doc_id_current){
		if(!et_doc_set_focus_to_id(id_doc, pv_focus_get_nofocus())){
			et_error("");
			return false;
		}
		if(!et_etaion_set_current_doc_id(id_doc)){
			et_error("");
			return false;
		}
	}

	EtDoc *doc = et_doc_manager_get_doc_from_id(id_doc);
	if(NULL == doc){
		et_error("");
		return false;
	}

	bool is_error = true;
	PvFocus focus = et_doc_get_focus_from_id(id_doc, &is_error);
	if(is_error){
		et_error("");
		return false;
	}

	switch(mouse_action.action){
		case EtMouseAction_Down:
			{
				et_debug(" x:%d, y:%d,\n",
						(int)mouse_action.point.x,
						(int)mouse_action.point.y);

				_mouse_action_up_down = mouse_action.action;

				PvElement *_element = focus.element;

				bool is_closed = false;
				if(NULL != _element && PvElementKind_Bezier == _element->kind){
					PvElementBezierData *_data =(PvElementBezierData *) _element->data;
					if(NULL == _data){
						et_error("");
						return false;
					}
					if(_data->is_close){
						// if already closed is goto new anchor_point
						_element = NULL;
					}else{
						if(0 < _data->anchor_points_num){
							if(_et_etaion_is_bound_point(
										_et_etaion_radius_path_detect,
										_data->anchor_points[0].points[PvAnchorPointIndex_Point],
										mouse_action.point)
							){
								// ** do close anchor_point
								_data->is_close = true;
								is_closed = true;
							}
						}
					}
				}

				// ** new anchor_point
				if(!is_closed){
					if(!et_doc_add_point(doc, &_element,
								mouse_action.point.x, mouse_action.point.y)){
						et_error("");
						return false;
					}else{
						focus.element = _element;
						if(!et_doc_set_focus_to_id(id_doc, focus)){
							et_error("");
							return false;
						}
					}
				}

			}
			break;
		case EtMouseAction_Up:
			{
				_mouse_action_up_down = mouse_action.action;
			}
			break;
		case EtMouseAction_Move:
			{
				if(EtMouseAction_Down != _mouse_action_up_down){
					break;
				}

				PvElement *_element = focus.element;
				if(NULL == _element || PvElementKind_Bezier != _element->kind){
					et_error("");
					return false;
				}

				PvElementBezierData *_data =(PvElementBezierData *) _element->data;
				if(NULL == _data){
					et_error("");
					return false;
				}
				PvAnchorPoint *ap = NULL;
				if(_data->is_close){
					ap = &_data->anchor_points[0];
				}else{
					ap = &_data->anchor_points[_data->anchor_points_num - 1];
				}
				pv_element_bezier_anchor_point_set_handle(ap, PvAnchorPointIndex_Point,
						mouse_action.point);
			}
			break;
		case EtMouseAction_Unknown:
			et_bug("");
			break;
	}

	return true;
}

EtToolInfo _et_tool_infos[] = {
	{
		.tool_id = 0, 
		.name = "Focus Element",
		.icon = NULL,
		.icon_focus = NULL,
		.cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_element_allow_24x24.svg",
		.func_mouse_action = _et_tool_nop_mouse_action,
	},
	{
		.tool_id = 1, 
		.name = "Anchor Point",
		.icon = NULL,
		.icon_focus = NULL,
		.cursor = NULL,
		.filepath_icon = NULL,
		.filepath_cursor = "resource/tool/tool_anchor_point_put_allow_24x24.svg",
		.func_mouse_action = _et_tool_bezier_mouse_action,
	},
};

int et_tool_get_num()
{
	return sizeof(_et_tool_infos) / sizeof(EtToolInfo);
}

