#include "et_pointing_manager.h"

#include <stdlib.h>
#include "et_error.h"

static EtPointingManager *pointing_manager = NULL;

EtPointingManager *et_pointing_manager_init()
{
	EtPointingManager *this;
	this = (EtPointingManager *)malloc(sizeof(EtPointingManager));
	if(NULL == this){
		et_critical("");
		return NULL;
	}

	this->slot_mouse_action = NULL;

	pointing_manager = this;

	return this;
}

bool et_pointing_manager_set_slot_mouse_action(EtPointingManagerSlotMouseAction func)
{
	EtPointingManager *this = pointing_manager;
	if(NULL == this){
		et_bug("");
		return false;
	}

	if(NULL != this->slot_mouse_action){
		et_bug("");
		return false;
	}

	this->slot_mouse_action = func;

	return true;
}

bool et_pointing_manager_signal_mouse_action(EtDocId id_doc, EtMouseAction mouse_action)
{
	EtPointingManager *this = pointing_manager;
	if(NULL == this){
		et_bug("");
		return false;
	}
	if(NULL == this->slot_mouse_action){
		et_bug("");
		return false;
	}

	if(!this->slot_mouse_action(id_doc, mouse_action)){
		et_error("");
		return false;
	}

	return true;
}