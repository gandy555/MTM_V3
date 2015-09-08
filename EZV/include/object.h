#ifndef __OBJECT_H__
#define __OBJECT_H__

enum
{
	OBJ_TYPE_BASE = 0x100,
	OBJ_TYPE_TEXT,
	OBJ_TYPE_INPUT,
	OBJ_TYPE_LINE,
	OBJ_TYPE_RECT,
	OBJ_TYPE_PLAT_BUTTON,
	OBJ_TYPE_BUTTON,
	OBJ_TYPE_CHECK,
	OBJ_TYPE_GROUP,
	OBJ_TYPE_ICON,
	OBJ_TYPE_IMAGE,
	OBJ_TYPE_MAX
};

// Object Status Flag
#define	OBJ_STAT_NORMAL				0x001
#define	OBJ_STAT_DISABLE			0x002
#define	OBJ_STAT_UP					0x004
#define	OBJ_STAT_DOWN				0x008
#define	OBJ_STAT_CHECK				0x010
#define	OBJ_STAT_UNCHECK			0x020
#define	OBJ_STAT_SELECT				0x040
#define	OBJ_STAT_UNSELECT			0x080
#define	OBJ_STAT_ON					0x100
#define	OBJ_STAT_OFF				0x200
#define	OBJ_STAT_MASK				0x3FF

#define TOUCH_UP					0		//FALSE
#define TOUCH_DOWN					1		//TRUE

#define IMG_BUTTON_COUNT			2

#define IMG_BUTTON_UP				0
#define IMG_BUTTON_DOWN				1

#define IMG_CHECK_OFF				0		//Uncheck
#define IMG_CHECK_ON				1		//Check

#define IMG_GROUP_UNSELECT			0		//Unselect
#define IMG_GROUP_SELECT			1		//Select

#define IMG_BACKGROUND				0
#define IMG_ICON					0

typedef void (*PFN_OBJ_FUNC)(void *);

class CObject
{
public:
	CObject(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h);
	~CObject();

	// Member Function
	void SetFunc(PFN_OBJ_FUNC pfnFunc);
	void SetRect(int x, int y, int w, int h);
	BOOL IsTouch(int x, int y);

	void SetStatus(UINT status)		{ m_status = status; }
	void AddStatus(UINT status)		{ m_status |= status; }
	void ClrStatus(UINT status)		{ m_status &= ~status; }
	UINT GetStatus()				{ return m_status; }

	virtual void Draw();
	virtual void Touch(BOOL isTouchDown);
	virtual BOOL LoadImage(UINT idx, char* pImageFilePath);
	virtual void UnloadImage();

	// Member Variable
	UINT			m_id;
	UINT			m_type;
	UINT			m_status;

	GR_WINDOW_ID	m_wid;
	GR_GC_ID		m_gc;
	RECT			m_rect;

	PFN_OBJ_FUNC	m_pfnFunc;
};

typedef struct _OBJECT_NODE
{
	CObject*		pObject;
	_OBJECT_NODE*	pPrev;
	_OBJECT_NODE*	pNext;
} __attribute__ ((packed)) OBJECT_NODE;

class CObjectList
{
public:
	CObjectList();
	~CObjectList();

	// Member Function
	UINT AddObject(CObject* pObject);
	void RemoveAll();

	OBJECT_NODE* GetLastNode();

	CObject* FindObjectByID(UINT id);
	CObject* FindObjectByPosition(int x, int y);
	CObject* FindObjectByGroupID(UINT idGroup);
	CObject* NextObjectByGroupID(UINT idGroup, UINT id);
	CObject* FindObjectByType(UINT type);
	CObject* NextObjectByType(UINT type, UINT id);

	void Draw(UINT id);
	void DrawGroup(UINT idGroup, UINT idSelect);

	CObject* SelectInput(UINT id);

	// Member Variable
	OBJECT_NODE*	m_pRootNode;
	UINT			m_nObjectCount;
};

#define	TXT_ALIGN_NONE			0

#define	TXT_HALIGN_LEFT			0x1
#define	TXT_HALIGN_CENTER		0x2
#define	TXT_HALIGN_RIGHT		0x4
#define	TXT_HALIGN_MASK			0x7

#define	TXT_VALIGN_TOP			0x10
#define	TXT_VALIGN_MIDDLE		0x20
#define	TXT_VALIGN_BOTTOM		0x40
#define	TXT_VALIGN_MASK			0x70


class CObjectText : public CObject
{
public:
	CObjectText(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h);
	~CObjectText();

	// Member Function
	virtual void Draw();
//	virtual void Touch(BOOL isTouchDown);
//	virtual BOOL LoadImage(UINT idx, char* pImageFilePath);
//	virtual void UnloadImage();

	void SetAttribue(GR_FONT_ID font, UINT size, UINT color, UINT align);
	void Draw(const char* pszText);

	// Member Variable
	GR_FONT_ID	m_Font;
	UINT		m_Size;
	UINT		m_Color;
	UINT		m_Align;
	char*		m_pText;
};

#define MAX_INPUT		100
class CObjectInput : public CObject
{
public:
	CObjectInput(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h);
	~CObjectInput();

	// Member Function
	virtual void Draw();
//	virtual void Touch(BOOL isTouchDown);
//	virtual BOOL LoadImage(UINT idx, char* pImageFilePath);
//	virtual void UnloadImage();

	void SetAttribue(GR_FONT_ID font, UINT size, UINT color_fg, UINT color_bg, UINT align);
	void SetInput(char* pszInput);

	void SetFocus(BOOL isFocus)	{ m_isFocus = isFocus; }
	BOOL GetFocus()				{ return m_isFocus; }

	void PutChar(char ch);
	void DelChar();

	void PutWChar(USHORT wch);
	void DelWChar();

	// Member Variable
	GR_FONT_ID	m_Font;
	UINT		m_Size;
	UINT		m_Color_fg;
	UINT		m_Color_bg;
	UINT		m_Align;
	BOOL		m_isFocus;
	char		m_szInput[MAX_INPUT+1];
	int			m_idxInput;
};

class CObjectLine : public CObject
{
public:
	CObjectLine(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h);
	~CObjectLine();

	// Member Function
	virtual void Draw();
//	virtual void Func();
	virtual BOOL LoadImage(UINT idx, char* pImageFilePath);
	virtual void UnloadImage();

	void Draw(UINT color, BOOL isDash=FALSE);

	// Member Variable
	UINT		m_Color;
	BOOL		m_isDash;
};

class CObjectRect : public CObject
{
public:
	CObjectRect(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h);
	~CObjectRect();

	// Member Function
	virtual void Draw();
//	virtual void Func();
	virtual BOOL LoadImage(UINT idx, char* pImageFilePath);
	virtual void UnloadImage();

	void Draw(UINT color, BOOL isDash=FALSE, BOOL isFill=FALSE);

	// Member Variable
	UINT		m_Color;
	BOOL		m_isDash;
	BOOL		m_isFill;
};

class CObjectPlatButton : public CObject
{
public:
	CObjectPlatButton(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h);
	~CObjectPlatButton();

	// Member Function
	virtual void Draw();
//	virtual void Func();
	virtual void Touch(BOOL isTouchDown);
	virtual BOOL LoadImage(UINT idx, char* pImageFilePath);
	virtual void UnloadImage();

	void SetText(char* pszText, GR_FONT_ID font, UINT size, UINT color_normal, UINT color_touch);

	// Member Variable
	GR_DRAW_ID	m_Image;

	char*		m_pszText;
	GR_FONT_ID	m_font;
	UINT		m_size;
	UINT		m_color;
	UINT		m_color_normal;
	UINT		m_color_touch;
};

class CObjectButton : public CObject
{
public:
	CObjectButton(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h);
	~CObjectButton();

	// Member Function
	virtual void Draw();
//	virtual void Func();
	virtual void Touch(BOOL isTouchDown);
	virtual BOOL LoadImage(UINT idx, char* pImageFilePath);
	virtual void UnloadImage();

	void Draw(UINT idxImage);

	// Member Variable
	GR_DRAW_ID	m_Image[IMG_BUTTON_COUNT];
	UINT		m_idxImage;
};

class CObjectCheck : public CObject
{
public:
	CObjectCheck(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h);
	~CObjectCheck();

	// Member Function
	virtual void Draw();
//	virtual void Func();
	virtual void Touch(BOOL isTouchDown);
	virtual BOOL LoadImage(UINT idx, char* pImageFilePath);
	virtual void UnloadImage();

	void Draw(UINT idxImage);

	// Member Variable
	GR_DRAW_ID	m_Image[IMG_BUTTON_COUNT];
	UINT		m_idxImage;
};

class CObjectGroup : public CObject
{
public:
	CObjectGroup(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h);
	~CObjectGroup();

	// Member Function
	virtual void Draw();
//	virtual void Func();
	virtual void Touch(BOOL isTouchDown);
	virtual BOOL LoadImage(UINT idx, char* pImageFilePath);
	virtual void UnloadImage();

	void Draw(UINT idxImage);
	void SetGroup(UINT idGroup)		{ m_idGroup = idGroup; }
	UINT GetGroup()					{ return m_idGroup; }
	void SetOption(BOOL isOption)	{ m_isOption = isOption; }
	void SetText(char* pszText, GR_FONT_ID font, UINT size, UINT color, UINT align);

	// Member Variable
	GR_DRAW_ID	m_Image[IMG_BUTTON_COUNT];
	UINT		m_idxImage;
	UINT		m_idGroup;
	BOOL		m_isOption;

	char*		m_pszText;
	GR_FONT_ID	m_text_font;
	UINT		m_text_size;
	UINT		m_text_color;
	UINT		m_text_align;
};

class CObjectIcon : public CObject
{
public:
	CObjectIcon(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h);
	~CObjectIcon();

	// Member Function
	virtual void Draw();
//	virtual void Func();
	virtual BOOL LoadImage(UINT idx, char* pImageFilePath);
	virtual void UnloadImage();

	void Draw(UINT idxImage);
	BOOL AllocImageCount(UINT imgCount);

	// Member Variable
	GR_DRAW_ID*	m_pImage;
	UINT		m_nImgCount;
	UINT		m_idxImage;
};

class CObjectImage : public CObject
{
public:
	CObjectImage(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h);
	~CObjectImage();

	// Member Function
	virtual void Draw();
//	virtual void Func();
	virtual BOOL LoadImage(UINT idx, char* pImageFilePath);
	virtual void UnloadImage();

	// Member Variable
	GR_DRAW_ID	m_Image;
};

#endif //__OBJECT_H__
