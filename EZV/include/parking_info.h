#ifndef __PARKING_INFO_H__
#define __PARKING_INFO_H__

/******************************************************************************
 * Variable Type Definition
 ******************************************************************************/

enum {
	PARKING_ITM_ID = 0,
	PARKING_ITM_FLOOR,
	PARKING_ITM_ZONE
};

/******************************************************************************
 * Function Export
 ******************************************************************************/
extern void parking_list_init(void);
extern void parking_list_update(MTM_DATA_PARKING* _info);
extern int parking_list_get_item(int _idx, MTM_DATA_PARKING* _dst);
extern int parking_item_to_str(int _id, char* _src, char* _dst);
#endif // __PARKING_INFO_H__

