#include "common.h"
#include "main.h"

// A series of numbers are passed indicating the on / off state 
// for example { 3, 1 } indicates 3 on and 1 off 
char g_dash_patern[2] = { 10, 5 };

//-----------------------------------------------------------
// Global Function Implemetation
//-----------------------------------------------------------
BOOL PointInRect(RECT* pRect, int x, int y)
{
	if(pRect)
	{
		if( (x>pRect->x) && (x<(pRect->x+pRect->w)) && (y>pRect->y) && (y<(pRect->y+pRect->h)) )
		{
			return TRUE;
		}
	}

	return FALSE;
}

void DrawText(char* pszText, GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h, GR_FONT_ID font, UINT size, UINT color, UINT align)
{
	int align_x, align_y, len, width, height, base;
	GR_GC_INFO gc_info;

	if(pszText)
	{
		GrGetGCInfo(gc, &gc_info);

		GrSetGCUseBackground(gc, FALSE);
		GrSetGCForeground(gc, color);
		GrSetFontSizeEx(font, size, size);

		len = strlen(pszText);

		align_x = x;
		align_y = y;

		if(align != TXT_ALIGN_NONE)
		{
			GrGetGCTextSize(gc, pszText, len, MWTF_DBCS_EUCKR, &width, &height, &base);

			// horizontal align
			if(align & TXT_HALIGN_CENTER) 
			{
				if(width < w) 
				{
					align_x = x + ((w - width) / 2);
				}
			}
			else if(align & TXT_HALIGN_RIGHT) 
			{
				if(width < w) 
				{
					align_x = x + (w - width);
				}
			}

			// vertical align
			if(align & TXT_VALIGN_MIDDLE) 
			{
				if(height < h) 
				{
					align_y = y + ((h - height) / 2);
				}
			}
			else if(align & TXT_VALIGN_BOTTOM) 
			{
				if(height < h) 
				{
					align_y = y + (h - height);
				}
			}
		}

		GrText(wid, gc, align_x, align_y, pszText, len, MWTF_DBCS_EUCKR|GR_TFTOP);

		//Recover GC Info
		GrSetGCUseBackground(gc, gc_info.usebackground);
		GrSetGCForeground(gc, gc_info.foreground);
	}
}

void DrawText2(char* pszText, GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h, GR_FONT_ID font, UINT size, UINT color_fg, UINT color_bg, UINT align)
{
	int align_x, align_y, len, width, height, base;
	GR_GC_INFO gc_info;

	if(pszText)
	{
		GrGetGCInfo(gc, &gc_info);

		GrSetGCUseBackground(gc, TRUE);
		GrSetGCForeground(gc, color_fg);
		GrSetGCBackground(gc, color_bg);
		GrSetFontSizeEx(font, size, size);

		len = strlen(pszText);

		align_x = x;
		align_y = y;

		if(align != TXT_ALIGN_NONE)
		{
			GrGetGCTextSize(gc, pszText, len, MWTF_DBCS_EUCKR, &width, &height, &base);

			// horizontal align
			if(align & TXT_HALIGN_CENTER) 
			{
				if(width < w) 
				{
					align_x = x + ((w - width) / 2);
				}
			}
			else if(align & TXT_HALIGN_RIGHT) 
			{
				if(width < w) 
				{
					align_x = x + (w - width);
				}
			}

			// vertical align
			if(align & TXT_VALIGN_MIDDLE) 
			{
				if(height < h) 
				{
					align_y = y + ((h - height) / 2);
				}
			}
			else if(align & TXT_VALIGN_BOTTOM) 
			{
				if(height < h) 
				{
					align_y = y + (h - height);
				}
			}
		}

		GrText(wid, gc, align_x, align_y, pszText, len, MWTF_DBCS_EUCKR|GR_TFTOP);

		//Recover GC Info
		GrSetGCUseBackground(gc, gc_info.usebackground);
		GrSetGCForeground(gc, gc_info.foreground);
		GrSetGCBackground(gc, gc_info.background);
	}
}

void DrawTextRect(char* pszText, GR_WINDOW_ID wid, GR_GC_ID gc, RECT* pRect, GR_FONT_ID font, UINT size, UINT color, UINT align)
{
	DrawText(pszText, wid, gc, pRect->x, pRect->y, pRect->w, pRect->h, font, size, color, align);
}

void DrawTextRect2(char* pszText, GR_WINDOW_ID wid, GR_GC_ID gc, RECT* pRect, GR_FONT_ID font, UINT size, UINT color_fg, UINT color_bg, UINT align)
{
	DrawText2(pszText, wid, gc, pRect->x, pRect->y, pRect->w, pRect->h, font, size, color_fg, color_bg, align);
}

void DrawLine(int x1, int y1, int x2, int y2, GR_WINDOW_ID wid, GR_GC_ID gc, UINT color, BOOL isDash)
{
	GrSetGCForeground(gc, color);

	if(isDash)
	{
		GrSetGCLineAttributes(gc, GR_LINE_ONOFF_DASH);
		GrSetGCDash(gc, g_dash_patern, 2);
		GrLine(wid, gc, x1, y1, x2, y2);
		GrSetGCLineAttributes(gc, GR_LINE_SOLID);
	}
	else
	{
		GrLine(wid, gc, x1, y1, x2, y2);
	}
}

void DrawRect(int x, int y, int w, int h, GR_WINDOW_ID wid, GR_GC_ID gc, UINT color, BOOL isFill)
{
	GrSetGCForeground(gc, color);

	if(isFill)
	{
		GrFillRect(wid, gc, x, y, w, h);
	}
	else
	{
		GrRect(wid, gc, x, y, w, h);
	}
}

void DrawRect2(RECT* pRect, GR_WINDOW_ID wid, GR_GC_ID gc, UINT color, BOOL isFill)
{
	if(pRect==NULL) return;

	GrSetGCForeground(gc, color);

	if(isFill)
	{
		GrFillRect(wid, gc, pRect->x, pRect->y, pRect->w, pRect->h);
	}
	else
	{
		GrRect(wid, gc, pRect->x, pRect->y, pRect->w, pRect->h);
	}
}

void RedrawImage(GR_WINDOW_ID wid, GR_GC_ID gc, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh, UINT id)
{
	CObject* pObject = NULL;
	CObjectImage* pObjectImage = NULL;

	if(g_state.m_pCurrContext)
	{
		pObject = g_state.m_pCurrContext->m_ObjectList.FindObjectByID(id);
		if(pObject)
		{
			if(pObject->m_type == OBJ_TYPE_IMAGE)
			{
				pObjectImage = (CObjectImage*)pObject;
				GrDrawImagePartToFit(wid, gc, dx, dy, dw, dh, sx, sy, sw, sh, pObjectImage->m_Image);
			}
		}
	}
}

void GetLastEvent(GR_EVENT *ep)
{
	GR_EVENT event_curr, event_prev;
	int nCount = 0;

	while(1)
	{
	//	memset(&event_curr, 0, sizeof(GR_EVENT));
		GrCheckNextEvent(&event_curr);
	//	GrGetNextEvent(&event_curr);
		if(event_curr.type == GR_EVENT_TYPE_NONE)
		{
			if(nCount>1)
			{
				printf("%s: %d Message Ignored\r\n", __func__, nCount-1);
			}
			memcpy(ep, &event_prev, sizeof(GR_EVENT));
			break;
		}
		else
		{
			nCount++;
			memcpy(&event_prev, &event_curr, sizeof(GR_EVENT));
			printf("%s: Event.Type=%d\r\n", __func__, event_curr.type);
		}
		usleep(1000);
	}
}

void post_event(GR_EVENT_TYPE type, unsigned int event, unsigned int p1, unsigned int p2)
{
	GR_WINDOW *wp;
	GR_EVENT_USER *ep;		/* event type ptr */
	GR_EVENT_CLIENT *ecp;	/* current event client */

	wp = GsPrepareWindow(g_wid);

	if(!wp || !wp->realized || !wp->output) return;

	for(ecp = wp->eventclients; ecp; ecp = ecp->next)
	{
		ep = (GR_EVENT_USER*)GsAllocEvent(ecp->client);
		if(!ep) 
		{
			continue;
		}
		ep->type	= type;
		ep->wid		= wp->id;
		ep->event	= event;
		ep->p1		= p1;
		ep->p2		= p2;
	}

	//GrPostEventDone(ep);
}

