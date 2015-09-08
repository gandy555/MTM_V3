/*
*/
#include "common.h"

//-----------------------------------------------------------
// CObject Class Implemetation
//-----------------------------------------------------------

CObject::CObject(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h)
{
	m_wid = wid;
	m_gc = gc;

	SetStatus(OBJ_STAT_NORMAL);
	SetRect(x, y, w, h);

	m_pfnFunc = NULL;
}

CObject::~CObject()
{
}

void CObject::SetFunc(PFN_OBJ_FUNC pfnFunc)
{
	m_pfnFunc = pfnFunc;
}

void CObject::SetRect(int x, int y, int w, int h)
{
	m_rect.x = x;
	m_rect.y = y;
	m_rect.w = w;
	m_rect.h = h;
}

BOOL CObject::IsTouch(int x, int y)
{
	switch(m_type)
	{
	case OBJ_TYPE_INPUT:
	case OBJ_TYPE_PLAT_BUTTON:
	case OBJ_TYPE_BUTTON:
	case OBJ_TYPE_CHECK:
	case OBJ_TYPE_GROUP:
		if(!(GetStatus()&OBJ_STAT_DISABLE))
		{
			return PointInRect(&m_rect, x, y);
		}
		break;
	}

	return FALSE;
}

void CObject::Draw()
{
}

/*
void CObject::Func()
{
}
*/
void CObject::Touch(BOOL isTouchDown)
{
}

BOOL CObject::LoadImage(UINT idx, char* pImageFilePath)
{
}

void CObject::UnloadImage()
{
}

//-----------------------------------------------------------
// CObjectList Class Implemetation
//-----------------------------------------------------------

CObjectList::CObjectList()
{
	m_nObjectCount = 0;
	m_pRootNode = NULL;
}

CObjectList::~CObjectList()
{
}

UINT CObjectList::AddObject(CObject* pObject)
{
	OBJECT_NODE* pNode;
	OBJECT_NODE* pNodeNew;

	if(pObject==NULL) return 0;

	if(m_pRootNode)
	{
		pNode = GetLastNode();
		pNodeNew = new OBJECT_NODE;
		if(pNodeNew == NULL)
		{
			printf("%s: New Node Allocation Failure\r\n");
			return 0;
		}
		pNode->pNext = pNodeNew;
		pNodeNew->pPrev = pNode;
		pNodeNew->pNext = NULL;
		pNodeNew->pObject = pObject;

		m_nObjectCount++;
	}
	else
	{
		m_pRootNode = new OBJECT_NODE;
		if(m_pRootNode == NULL)
		{
			printf("%s: Root Node Allocation Failure\r\n");
			return 0;
		}
		m_pRootNode->pPrev = NULL;	//m_pRootNode;
		m_pRootNode->pNext = NULL;
		m_pRootNode->pObject = pObject;

		m_nObjectCount = 1;
	}

	pObject->m_id = m_nObjectCount;

//	printf("%s: type=%d object added at %d\r\n", __func__, pObject->m_type, pObject->m_id);

	return pObject->m_id;
}

void CObjectList::RemoveAll()
{
	OBJECT_NODE* pNode = m_pRootNode;
	OBJECT_NODE* pNext;
//	CObject* pObject;

	while(pNode)
	{
		pNode->pObject->UnloadImage();
		delete pNode->pObject;

		pNext = pNode->pNext;

		delete pNode;

		if(pNext) pNode = pNext;
		else	  break;
	}

	m_pRootNode = NULL;

//	printf("%s: remove done\r\n", __func__);
}

OBJECT_NODE* CObjectList::GetLastNode()
{
	OBJECT_NODE* pNode = m_pRootNode;

	if(pNode)
	{
		while(pNode->pNext)
		{
			pNode = pNode->pNext;
		}
		return pNode;
	}
	else
	{
		return NULL;
	}
}

CObject* CObjectList::FindObjectByID(UINT id)
{
	OBJECT_NODE* pNode = m_pRootNode;

	if(pNode)
	{
		do
		{
			if(pNode->pObject->m_id == id)
			{
				return pNode->pObject;
			}
		}
		while( (pNode = pNode->pNext) );
	}
	
	return NULL;
}

CObject* CObjectList::FindObjectByPosition(int x, int y)
{
	OBJECT_NODE* pNode = m_pRootNode;

	if(pNode)
	{
		do
		{
			if(pNode->pObject->IsTouch(x, y))
			{
		//		printf("%s: found. type=%d, id=%d\r\n", __func__, pNode->pObject->m_type, pNode->pObject->m_id);
				return pNode->pObject;
			}
		}
		while( (pNode = pNode->pNext) );
	}

//	printf("%s: not found.\r\n", __func__);
	
	return NULL;
}

CObject* CObjectList::FindObjectByGroupID(UINT idGroup)
{
	OBJECT_NODE* pNode = m_pRootNode;
	CObjectGroup* pObjectGroup = NULL;

	if(pNode)
	{
		do
		{
			pObjectGroup = (CObjectGroup*)(pNode->pObject);
			if(pObjectGroup->m_idGroup == idGroup)
			{
				return pNode->pObject;
			}
		}
		while( (pNode = pNode->pNext) );
	}
	
	return NULL;
}

CObject* CObjectList::NextObjectByGroupID(UINT idGroup, UINT id)
{
	OBJECT_NODE* pNode = m_pRootNode;
	CObjectGroup* pObjectGroup = NULL;

	if(pNode)
	{
		do
		{
			pObjectGroup = (CObjectGroup*)(pNode->pObject);
			if( (pObjectGroup->m_idGroup == idGroup) && (pObjectGroup->m_id > id) )
			{
				return pNode->pObject;
			}
		}
		while( (pNode = pNode->pNext) );
	}
	
	return NULL;
}

CObject* CObjectList::FindObjectByType(UINT type)
{
	OBJECT_NODE* pNode = m_pRootNode;

	if(pNode)
	{
		do
		{
			if(pNode->pObject->m_type == type)
			{
				return pNode->pObject;
			}
		}
		while( (pNode = pNode->pNext) );
	}
	
	return NULL;
}

CObject* CObjectList::NextObjectByType(UINT type, UINT id)
{
	OBJECT_NODE* pNode = m_pRootNode;

	if(pNode)
	{
		do
		{
			if( (pNode->pObject->m_type == type) && (pNode->pObject->m_id > id) )
			{
				return pNode->pObject;
			}
		}
		while( (pNode = pNode->pNext) );
	}
	
	return NULL;
}

void CObjectList::Draw(UINT id)
{
	CObject* pObject = NULL;

	pObject = FindObjectByID(id);
	if(pObject)
	{
		pObject->Draw();
	}
}

void CObjectList::DrawGroup(UINT idGroup, UINT idSelect)
{
	CObject* pObject = NULL;
	CObjectGroup* pObjectGroup = NULL;

	pObject = FindObjectByGroupID(idGroup);
	while(pObject)
	{
		pObjectGroup = (CObjectGroup*)pObject;
		if(pObjectGroup->m_id == idSelect)
		{
			pObjectGroup->SetStatus(OBJ_STAT_SELECT);
			pObjectGroup->Draw(IMG_GROUP_SELECT);
		}
		else
		{
			pObjectGroup->SetStatus(OBJ_STAT_UNSELECT);
			pObjectGroup->Draw(IMG_GROUP_UNSELECT);
		}

	//	pObject = NextObjectByGroupID(pObject);
		pObject = NextObjectByGroupID(pObjectGroup->m_idGroup, pObjectGroup->m_id);
	}
}

CObject* CObjectList::SelectInput(UINT id)
{
	CObject* pObject = NULL;
	CObjectInput* pObjectInput = NULL;
	CObjectInput* pObjectInputFocus = NULL;

	pObject = FindObjectByType(OBJ_TYPE_INPUT);
	while(pObject)
	{
		pObjectInput = (CObjectInput*)pObject;
		if(pObjectInput->m_id == id)
		{
			pObjectInput->SetFocus(TRUE);
			pObjectInputFocus = pObjectInput;
		}
		else
			pObjectInput->SetFocus(FALSE);

		pObject = NextObjectByType(OBJ_TYPE_INPUT, pObjectInput->m_id);
	}

	return pObjectInputFocus;
}

//-----------------------------------------------------------
// CObjectText Class Implemetation
//-----------------------------------------------------------

CObjectText::CObjectText(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h)
	: CObject(wid, gc, x, y, w, h)
{
	m_type = OBJ_TYPE_TEXT;

	m_Font = 0;
	m_Size = 0;
	m_Color = 0;
	m_Align = 0;

	m_pText = NULL;
}

CObjectText::~CObjectText()
{
	if(m_pText)
	{
		delete[] m_pText;
	}
}

void CObjectText::Draw()
{
	DrawTextRect(m_pText, m_wid, m_gc, &m_rect, m_Font, m_Size, m_Color, m_Align);
}

void CObjectText::Draw(const char* pszText)
{
	int len;

	if(pszText==NULL)
	{
		printf("%s: Invalid Text String\r\n", __func__);
		return;
	}

	if(m_pText)
	{
		delete[] m_pText;
	}
	len = strlen(pszText)+1;
	m_pText = new char[len];
	strcpy(m_pText, pszText);

	Draw();
}

/*
void CObjectText::Func()
{
}
*/
/*
BOOL CObjectText::LoadImage(UINT idx, char* pImageFilePath)
{
}

void CObjectText::UnloadImage()
{
}
*/
void CObjectText::SetAttribue(GR_FONT_ID font, UINT size, UINT color, UINT align)
{
	m_Font = font;
	m_Size = size;
	m_Color = color;
	m_Align = align;
}

//-----------------------------------------------------------
// CObjectInput Class Implemetation
//-----------------------------------------------------------

CObjectInput::CObjectInput(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h)
	: CObject(wid, gc, x, y, w, h)
{
	m_type = OBJ_TYPE_INPUT;

	m_Font = 0;
	m_Size = 0;
	m_Color_fg = 0;
	m_Color_bg = 0;
	m_Align = 0;

	memset(m_szInput, 0, MAX_INPUT+1);
	m_isFocus = FALSE;
	m_idxInput = 0;
}

CObjectInput::~CObjectInput()
{

}

void CObjectInput::Draw()
{
	DrawRect(m_rect.x, m_rect.y, m_rect.w, m_rect.h, m_wid, m_gc, m_Color_bg, TRUE);
	if(m_isFocus)
	{
		DrawRect(m_rect.x, m_rect.y, m_rect.w, m_rect.h, m_wid, m_gc, RGB(255,255,255), FALSE);
	//	DrawTextRect2(m_szInput, m_wid, m_gc, &m_rect, m_Font, m_Size, RGB(48,49,48), m_Color_fg, m_Align);
		DrawTextRect(m_szInput, m_wid, m_gc, &m_rect, m_Font, m_Size, m_Color_fg, m_Align);
	}
	else
	{
		DrawTextRect(m_szInput, m_wid, m_gc, &m_rect, m_Font, m_Size, m_Color_fg, m_Align);
	}
}

/*
void CObjectInput::Touch(BOOL isTouchDown)
{
	if(isTouchDown)
	{
		SetFocus(TRUE);
	}
}
*/

void CObjectInput::SetAttribue(GR_FONT_ID font, UINT size, UINT color_fg, UINT color_bg, UINT align)
{
	m_Font = font;
	m_Size = size;
	m_Color_fg = color_fg;
	m_Color_bg = color_bg;
	m_Align = align;
}

void CObjectInput::SetInput(char* pszInput)
{
	if(pszInput)
	{
		memset(m_szInput, 0, MAX_INPUT+1);
		memcpy(m_szInput, pszInput, (strlen(pszInput)>MAX_INPUT) ? MAX_INPUT : strlen(pszInput));
		m_idxInput = strlen(m_szInput);
	}
	else
	{
		m_idxInput = 0;
	}
}

void CObjectInput::PutChar(char ch)
{
//	printf("%s\r\n", __func__);

	m_szInput[m_idxInput] = ch;

	if(m_idxInput < MAX_INPUT) m_idxInput++;

	Draw();
}

void CObjectInput::DelChar()
{
//	printf("%s\r\n", __func__);

	if(m_idxInput > 0)
	{
		if( (m_idxInput > 1) && (m_szInput[m_idxInput-1] >= 0xA1) && (m_szInput[m_idxInput-1] <= 0xFE) )
		{
			DelWChar();
		}
		else
		{
			m_idxInput--;
			m_szInput[m_idxInput] = ' ';
			Draw();
		}
	}
}

void CObjectInput::PutWChar(USHORT wch)
{
	unsigned short* pwch;

	if(m_idxInput <= (MAX_INPUT-2))
	{
		pwch = (USHORT*)&m_szInput[m_idxInput];
		*pwch = wch;
		if(m_idxInput <= MAX_INPUT) m_idxInput+=2;
		Draw();
	}
}

void CObjectInput::DelWChar()
{
//	USHORT* pwch;

	if(m_idxInput > 1)
	{
		if( (m_szInput[m_idxInput-1] >= 0xA1) && (m_szInput[m_idxInput-1] <= 0xFE) )
		{
			m_szInput[m_idxInput-2] = ' ';
			m_szInput[m_idxInput-1] = ' ';
			m_idxInput-=2;
			Draw();
		}
		else
		{
			DelChar();
		}
	}
}

//-----------------------------------------------------------
// CObjectLine Class Implemetation
//-----------------------------------------------------------

CObjectLine::CObjectLine(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h)
	: CObject(wid, gc, x, y, w, h)
{
	m_type = OBJ_TYPE_LINE;
}

CObjectLine::~CObjectLine()
{
}

// width=x2, height=y2
void CObjectLine::Draw()
{
	if(m_wid && m_gc)
	{
		GrSetGCForeground(m_gc, m_Color);
		if(m_isDash)
		{
			GrSetGCLineAttributes(m_gc, GR_LINE_ONOFF_DASH);
			GrSetGCDash(m_gc, g_dash_patern, 2);
			GrLine(m_wid, m_gc, m_rect.x, m_rect.y, m_rect.w, m_rect.h);
			GrSetGCLineAttributes(m_gc, GR_LINE_SOLID);
		}
		else
		{
			GrLine(m_wid, m_gc, m_rect.x, m_rect.y, m_rect.w, m_rect.h);
		}
	}
}

void CObjectLine::Draw(UINT color, BOOL isDash)
{
	m_Color = color;
	m_isDash = isDash;

	Draw();
}

/*
void CObjectLine::Func()
{
}
*/

BOOL CObjectLine::LoadImage(UINT idx, char* pImageFilePath)
{
}

void CObjectLine::UnloadImage()
{
}

//-----------------------------------------------------------
// CObjectRect Class Implemetation
//-----------------------------------------------------------

CObjectRect::CObjectRect(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h)
	: CObject(wid, gc, x, y, w, h)
{
	m_type = OBJ_TYPE_RECT;
}

CObjectRect::~CObjectRect()
{
}

void CObjectRect::Draw()
{
	if(m_wid && m_gc)
	{
		GrSetGCForeground(m_gc, m_Color);
		if(m_isFill)
		{
			GrFillRect(m_wid, m_gc, m_rect.x, m_rect.y, m_rect.w, m_rect.h);
		}
		else if(m_isDash)
		{
			GrSetGCLineAttributes(m_gc, GR_LINE_ONOFF_DASH);
			GrSetGCDash(m_gc, g_dash_patern, 2);
			GrRect(m_wid, m_gc, m_rect.x, m_rect.y, m_rect.w, m_rect.h);
			GrSetGCLineAttributes(m_gc, GR_LINE_SOLID);
		}
		else
		{
			GrRect(m_wid, m_gc, m_rect.x, m_rect.y, m_rect.w, m_rect.h);
		}
	}
}

void CObjectRect::Draw(UINT color, BOOL isDash, BOOL isFill)
{
	m_Color = color;
	m_isDash = isDash;
	m_isFill = isFill;

	Draw();
}

/*
void CObjectRect::Func()
{
}
*/

BOOL CObjectRect::LoadImage(UINT idx, char* pImageFilePath)
{
}

void CObjectRect::UnloadImage()
{
}

//-----------------------------------------------------------
// CObjectPlatButton Class Implemetation
//-----------------------------------------------------------

CObjectPlatButton::CObjectPlatButton(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h)
	: CObject(wid, gc, x, y, w, h)
{
	m_type = OBJ_TYPE_PLAT_BUTTON;
}

CObjectPlatButton::~CObjectPlatButton()
{
	UnloadImage();
}

void CObjectPlatButton::Draw()
{
	if(m_wid && m_gc)
	{
		if(m_Image)
		{
			GrDrawImageToFit(m_wid, m_gc, m_rect.x, m_rect.y, m_rect.w, m_rect.h, m_Image);
		}

		if(m_pszText && m_font)
		{
			DrawTextRect(m_pszText, m_wid, m_gc, &m_rect, m_font, m_size, m_color, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
		}
	}
}

void CObjectPlatButton::Touch(BOOL isTouchDown)
{
	if(isTouchDown)
	{
		m_color = m_color_touch;
		Draw();
	}
	else
	{
		m_color = m_color_normal;
		Draw();
	}
}

BOOL CObjectPlatButton::LoadImage(UINT idx, char* pImageFilePath)
{
	if(pImageFilePath == NULL)
	{
		printf("%s: Invalid File Path\r\n", __func__);
		return FALSE;
	}

	m_Image = GrLoadImageFromFile(pImageFilePath, 0);
	if(m_Image == 0)
	{
		printf("%s: GrLoadImageFromFile, %s\r\n", __func__, pImageFilePath);
		return FALSE;
	}

//	printf("%s: '%s' loaded at %d\r\n", __func__, pImageFilePath, idx);

	return FALSE;
}

void CObjectPlatButton::UnloadImage()
{
	if(m_Image)
	{
		GrFreeImage(m_Image);
		m_Image = 0;
	}

//	printf("%s: unload complete\r\n", __func__);
}

void CObjectPlatButton::SetText(char* pszText, GR_FONT_ID font, UINT size, UINT color_normal, UINT color_touch)
{
	if(pszText==NULL) return;

	if(m_pszText)
	{
		delete[] m_pszText;
	}

	m_pszText = new char[strlen(pszText)+1];
	strcpy(m_pszText, pszText);

	m_font = font;
	m_size = size;

	m_color = color_normal;
	m_color_normal = color_normal;
	m_color_touch = color_touch;
}

//-----------------------------------------------------------
// CObjectButton Class Implemetation
//-----------------------------------------------------------

CObjectButton::CObjectButton(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h)
	: CObject(wid, gc, x, y, w, h)
{
	m_type = OBJ_TYPE_BUTTON;
	m_idxImage = 0;
}

CObjectButton::~CObjectButton()
{
	UnloadImage();
}

void CObjectButton::Draw()
{
//	printf("+%s\r\n", __func__);
	if(m_wid && m_gc && m_Image)
	{
		GrSetGCUseBackground(m_gc, FALSE);
		GrDrawImageToFit(m_wid, m_gc, m_rect.x, m_rect.y, m_rect.w, m_rect.h, m_Image[m_idxImage]);
	}
//	printf("-%s\r\n", __func__);
}

void CObjectButton::Draw(UINT idxImage)
{
//	printf("+%s\r\n", __func__);

	if(idxImage >= IMG_BUTTON_COUNT)
	{
		printf("%s: Invalid Image Index(%d), Max=%d\r\n", __func__, idxImage, IMG_BUTTON_COUNT);
		return;
	}

	m_idxImage = idxImage;

	Draw();
//	printf("-%s\r\n", __func__);
}

/*
void CObjectButton::Func()
{
}
*/

void CObjectButton::Touch(BOOL isTouchDown)
{
//	printf("+%s\r\n", __func__);
	if(isTouchDown)
	{
		Draw(IMG_BUTTON_DOWN);
	}
	else
	{
		Draw(IMG_BUTTON_UP);
	}
//	printf("-%s\r\n", __func__);
}

BOOL CObjectButton::LoadImage(UINT idx, char* pImageFilePath)
{
	if(idx >= IMG_BUTTON_COUNT)
	{
		printf("%s: Invalid Image Index(%d), Max=%d\r\n", __func__, idx, IMG_BUTTON_COUNT);
		return FALSE;
	}
	if(pImageFilePath == NULL)
	{
		printf("%s: Invalid File Path\r\n", __func__);
		return FALSE;
	}

	m_Image[idx] = GrLoadImageFromFile((char *)pImageFilePath, 0);
	if(m_Image[idx] == 0)
	{
		printf("%s: GrLoadImageFromFile failure, %s\r\n", __func__, pImageFilePath);
		return FALSE;
	}

//	printf("%s: '%s' loaded at %d\r\n", __func__, pImageFilePath, idx);

	return TRUE;
}

void CObjectButton::UnloadImage()
{
	int i;

	for(i=0; i<IMG_BUTTON_COUNT; i++)
	{
		if(m_Image[i])
		{
			GrFreeImage(m_Image[i]);
			m_Image[i] = 0;
		}
	}

//	printf("%s: unload complete\r\n", __func__);
}

//-----------------------------------------------------------
// CObjectCheck Class Implemetation
//-----------------------------------------------------------

CObjectCheck::CObjectCheck(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h)
	: CObject(wid, gc, x, y, w, h)
{
	m_type = OBJ_TYPE_CHECK;
	m_idxImage = 0;
}

CObjectCheck::~CObjectCheck()
{
	UnloadImage();
}

void CObjectCheck::Draw()
{
	if(m_wid && m_gc && m_Image)
	{
		GrDrawImageToFit(m_wid, m_gc, m_rect.x, m_rect.y, m_rect.w, m_rect.h, m_Image[m_idxImage]);
	}
}

void CObjectCheck::Draw(UINT idxImage)
{
	if(idxImage >= IMG_BUTTON_COUNT)
	{
		printf("%s: Invalid Image Index(%d), Max=%d\r\n", __func__, idxImage, IMG_BUTTON_COUNT);
		return;
	}

	m_idxImage = idxImage;

	Draw();
}

/*
void CObjectCheck::Func()
{
}
*/

void CObjectCheck::Touch(BOOL isTouchDown)
{
	if(isTouchDown)
	{
		Draw(IMG_BUTTON_DOWN);
	}
	else
	{
		Draw(IMG_BUTTON_UP);
	}
}

BOOL CObjectCheck::LoadImage(UINT idx, char* pImageFilePath)
{
	if(idx >= IMG_BUTTON_COUNT)
	{
		printf("%s: Invalid Image Index(%d), Max=%d\r\n", __func__, idx, IMG_BUTTON_COUNT);
		return FALSE;
	}
	if(pImageFilePath == NULL)
	{
		printf("%s: Invalid File Path\r\n", __func__);
		return FALSE;
	}

	m_Image[idx] = GrLoadImageFromFile(pImageFilePath, 0);
	if(m_Image[idx] == 0)
	{
		printf("%s: GrLoadImageFromFile, %s\r\n", __func__, pImageFilePath);
		return FALSE;
	}

//	printf("%s: '%s' loaded at %d\r\n", __func__, pImageFilePath, idx);

	return TRUE;
}

void CObjectCheck::UnloadImage()
{
	int i;

	for(i=0; i<IMG_BUTTON_COUNT; i++)
	{
		if(m_Image[i])
		{
			GrFreeImage(m_Image[i]);
			m_Image[i] = 0;
		}
	}

//	printf("%s: unload complete\r\n", __func__);
}

//-----------------------------------------------------------
// CObjectGroup Class Implemetation
//-----------------------------------------------------------

CObjectGroup::CObjectGroup(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h)
	: CObject(wid, gc, x, y, w, h)
{
	m_type = OBJ_TYPE_GROUP;
	m_idxImage = 0;
	m_pszText = NULL;
	m_isOption = FALSE;
}

CObjectGroup::~CObjectGroup()
{
	if(m_pszText)
	{
		delete[] m_pszText;
		m_pszText = NULL;
	}

	UnloadImage();
}

void CObjectGroup::Draw()
{
	GR_IMAGE_INFO img_info;
	RECT rcOptionText;

	if(m_wid && m_gc && m_Image)
	{
		if(m_isOption)
		{
			GrGetImageInfo(m_Image[m_idxImage], &img_info);
			GrDrawImageToFit(m_wid, m_gc, m_rect.x, m_rect.y, img_info.width, img_info.height, m_Image[m_idxImage]);
			if(m_pszText)
			{
				memcpy(&rcOptionText, &m_rect, sizeof(RECT));
				rcOptionText.x += (img_info.width + 10);
				DrawTextRect(m_pszText, m_wid, m_gc, &rcOptionText, m_text_font, m_text_size, m_text_color, m_text_align);
			}
		}
		else
		{
			GrDrawImageToFit(m_wid, m_gc, m_rect.x, m_rect.y, m_rect.w, m_rect.h, m_Image[m_idxImage]);

			if(m_pszText)
			{
				DrawTextRect(m_pszText, m_wid, m_gc, &m_rect, m_text_font, m_text_size, m_text_color, m_text_align);
			}
		}
	}
}

void CObjectGroup::Draw(UINT idxImage)
{
	if(idxImage >= IMG_BUTTON_COUNT)
	{
		printf("%s: Invalid Image Index(%d), Max=%d\r\n", __func__, idxImage, IMG_BUTTON_COUNT);
		return;
	}

	m_idxImage = idxImage;

	Draw();
}

/*
void CObjectGroup::Func()
{
}
*/

void CObjectGroup::Touch(BOOL isTouchDown)
{
	if(isTouchDown)
	{
	//	Draw(IMG_BUTTON_DOWN);
	}
	else
	{
		Draw(IMG_BUTTON_UP);
	}
}


BOOL CObjectGroup::LoadImage(UINT idx, char* pImageFilePath)
{
	if(idx >= IMG_BUTTON_COUNT)
	{
		printf("%s: Invalid Image Index(%d), Max=%d\r\n", __func__, idx, IMG_BUTTON_COUNT);
		return FALSE;
	}
	if(pImageFilePath == NULL)
	{
		printf("%s: Invalid File Path\r\n", __func__);
		return FALSE;
	}

	m_Image[idx] = GrLoadImageFromFile(pImageFilePath, 0);
	if(m_Image[idx] == 0)
	{
		printf("%s: GrLoadImageFromFile, %s\r\n", __func__, pImageFilePath);
		return FALSE;
	}

//	printf("%s: '%s' loaded at %d\r\n", __func__, pImageFilePath, idx);

	return TRUE;
}

void CObjectGroup::UnloadImage()
{
	int i;

	for(i=0; i<IMG_BUTTON_COUNT; i++)
	{
		if(m_Image[i])
		{
			GrFreeImage(m_Image[i]);
			m_Image[i] = 0;
		}
	}

//	printf("%s: unload complete\r\n", __func__);
}

void CObjectGroup::SetText(char* pszText, GR_FONT_ID font, UINT size, UINT color, UINT align)
{
	if(pszText==NULL) return;

	if(m_pszText)
	{
		delete[] m_pszText;
	}

	m_pszText = new char[strlen(pszText)+1];
	strcpy(m_pszText, pszText);

	m_text_font = font;
	m_text_size = size;
	m_text_color = color;
	m_text_align = align;
}

//-----------------------------------------------------------
// CObjectIcon Class Implemetation
//-----------------------------------------------------------

CObjectIcon::CObjectIcon(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h)
	: CObject(wid, gc, x, y, w, h)
{
	m_type = OBJ_TYPE_ICON;
	m_pImage = NULL;
	m_nImgCount = 0;
	m_idxImage = 0;
}

CObjectIcon::~CObjectIcon()
{
	UnloadImage();
}

void CObjectIcon::Draw()
{
	if(m_wid && m_gc && m_pImage)
	{
		GrDrawImageToFit(m_wid, m_gc, m_rect.x, m_rect.y, m_rect.w, m_rect.h, m_pImage[m_idxImage]);
	}
}

void CObjectIcon::Draw(UINT idxImage)
{
	if( (m_pImage==NULL) || (m_nImgCount==0) )
	{
		printf("%s: Image Count Not Allocated\r\n", __func__);
		return;
	}

	if(idxImage >= m_nImgCount)
	{
		printf("%s: Invalid Image Index(%d), Max=%d\r\n", __func__, idxImage, m_nImgCount);
		return;
	}

	m_idxImage = idxImage;

	Draw();
}

/*
void CObjectIcon::Func()
{
}
*/

BOOL CObjectIcon::LoadImage(UINT idx, char* pImageFilePath)
{
	if( (m_pImage==NULL) || (m_nImgCount==0) )
	{
		printf("%s: Image Count Not Allocated\r\n", __func__);
		return FALSE;
	}

	if(idx >= m_nImgCount)
	{
		printf("%s: Invalid Image Index(%d), Max=%d\r\n", __func__, idx, m_nImgCount);
		return FALSE;
	}

	if(pImageFilePath == NULL)
	{
		printf("%s: Invalid File Path\r\n", __func__);
		return FALSE;
	}

	m_pImage[idx] = GrLoadImageFromFile(pImageFilePath, 0);
	if(m_pImage[idx] == 0)
	{
		printf("%s: GrLoadImageFromFile, %s\r\n", __func__, pImageFilePath);
		return FALSE;
	}

//	printf("%s: '%s' loaded at %d\r\n", __func__, pImageFilePath, idx);

	return TRUE;
}

void CObjectIcon::UnloadImage()
{
	int i;

	if(m_pImage)
	{
		for(i=0; i<m_nImgCount; i++)
		{
			if(m_pImage[i])
			{
				GrFreeImage(m_pImage[i]);
				m_pImage[i] = 0;
			}
		}
		delete[] m_pImage;
		m_pImage = NULL;

		m_nImgCount = 0;
	}

//	printf("%s: unload complete\r\n", __func__);
}

BOOL CObjectIcon::AllocImageCount(UINT imgCount)
{
	if(m_pImage)
	{
		printf("%s: Image Count Already Allocated\r\n", __func__);
		return FALSE;
	}

	m_pImage = new GR_DRAW_ID[imgCount];
	if(m_pImage==NULL)
	{
		printf("%s: Image Count Allocation Failure\r\n", __func__);
		return FALSE;
	}

	m_nImgCount = imgCount;

	return TRUE;
}

//-----------------------------------------------------------
// CObjectImage Class Implemetation
//-----------------------------------------------------------

CObjectImage::CObjectImage(GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h)
	: CObject(wid, gc, x, y, w, h)
{
	m_type = OBJ_TYPE_IMAGE;
}

CObjectImage::~CObjectImage()
{
	UnloadImage();
}

void CObjectImage::Draw()
{
	if(m_wid && m_gc && m_Image)
	GrDrawImageToFit(m_wid, m_gc, m_rect.x, m_rect.y, m_rect.w, m_rect.h, m_Image);
}

/*
void CObjectImage::Func()
{
}
*/

BOOL CObjectImage::LoadImage(UINT idx, char* pImageFilePath)
{
	if(pImageFilePath == NULL)
	{
		printf("%s: Invalid File Path\r\n", __func__);
		return FALSE;
	}

	m_Image = GrLoadImageFromFile(pImageFilePath, 0);
	if(m_Image == 0)
	{
		printf("%s: GrLoadImageFromFile, %s\r\n", __func__, pImageFilePath);
		return FALSE;
	}

//	printf("%s: '%s' loaded at %d\r\n", __func__, pImageFilePath, idx);

	return FALSE;
}

void CObjectImage::UnloadImage()
{
	if(m_Image)
	{
		GrFreeImage(m_Image);
		m_Image = 0;
	}

//	printf("%s: unload complete\r\n", __func__);
}
