#ifndef __NANO_X_CUSTOM_H__
#define __NANO_X_CUSTOM_H__

#ifdef __cplusplus
extern "C" {
#endif

#define RGB						MW0RGB

#define GR_EVENT_TYPE_USER		23

enum
{
	EVT_NONE,
	EVT_STATE_CHANGE,
	EVT_CONTEXT_CHANGE,
	EVT_MICROWAVE_DETECT,
	EVT_WALLPAD_DATA_RECEIVE
};

typedef	struct gr_event_list GR_EVENT_LIST;
struct gr_event_list {
	GR_EVENT_LIST	*next;				/* next element in list */
	GR_EVENT		event;				/* event */
};

typedef struct gr_client GR_CLIENT;
struct gr_client {
	int				id;					/* client id and socket descriptor */
	GR_EVENT_LIST	*eventhead;			/* head of event chain (or NULL) */
	GR_EVENT_LIST	*eventtail;			/* tail of event chain (or NULL) */
	GR_CLIENT		*next;				/* the next client in the list */
	GR_CLIENT		*prev;				/* the previous client in the list */
	int				waiting_for_event;	/* used to implement GrGetNextEvent*/
	char			*shm_cmds;
	int				shm_cmds_size;
	int				shm_cmds_shmid;
	unsigned long	processid;			/* client process id*/
};

typedef	struct gr_event_client	GR_EVENT_CLIENT;
struct gr_event_client	{
	GR_EVENT_CLIENT	*next;				/* next interested client or NULL */
	GR_EVENT_MASK	eventmask;			/* events client wants to see */
	GR_CLIENT		*client;			/* client who is interested */
};

typedef struct gr_pixmap GR_PIXMAP;
struct gr_pixmap {
	GR_COORD		x;					/* x position (0)*/
	GR_COORD		y;					/* y position (0)*/
	GR_SIZE			width;				/* width */
	GR_SIZE			height;				/* height */
    struct _mwscreendevice *psd;		/* associated screen device */
	GR_WINDOW_ID	id;					/* pixmap id */
	/* end of GR_DRAWABLE common members*/

	GR_PIXMAP		*next;				/* next pixmap in list */
	GR_CLIENT		*owner;				/* client that created it */
};

typedef struct gr_window GR_WINDOW;
struct gr_window {
	GR_COORD		x;					/* absolute x position */
	GR_COORD		y;					/* absolute y position */
	GR_SIZE			width;				/* width */
	GR_SIZE			height;				/* height */
    struct _mwscreendevice *psd;		/* associated screen device */
	GR_WINDOW_ID	id;					/* window id */
	/* end of GR_DRAWABLE common members*/

	GR_WINDOW		*next;				/* next window in complete list */
	GR_CLIENT		*owner;				/* client that created it */
	GR_WINDOW		*parent;			/* parent window */
	GR_WINDOW		*children;			/* first child window */
	GR_WINDOW		*siblings;			/* next sibling window */
	GR_SIZE			bordersize;			/* size of border */
	GR_COLOR		bordercolor;		/* color of border */
	GR_COLOR		background;			/* background color */
	GR_PIXMAP		*bgpixmap;			/* background pixmap */
	int				bgpixmapflags;		/* center, tile etc. */
	GR_EVENT_MASK	nopropmask;			/* events not to be propagated */
	GR_EVENT_CLIENT	*eventclients;		/* clients interested in events */
	GR_CURSOR_ID	cursorid;			/* cursor for this window */
	GR_BOOL			mapped;				/* TRUE means requested to be mapped */
	GR_BOOL			realized;			/* TRUE means window is visible */
	GR_BOOL			output;				/* TRUE if window can do output */
	GR_WM_PROPS		props;				/* window properties*/
	GR_CHAR			*title;				/* window title*/
	MWCLIPREGION	*clipregion;		/* window clipping region */
};

GR_WINDOW	*GsPrepareWindow(GR_WINDOW_ID wid);
GR_EVENT	*GsAllocEvent(GR_CLIENT *client);

int GdCaptureScreen(char *path);

BOOL PointInRect(RECT* pRect, int x, int y);
void DrawText(char* pszText, GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h, GR_FONT_ID font, UINT size, UINT color, UINT align);
void DrawText2(char* pszText, GR_WINDOW_ID wid, GR_GC_ID gc, int x, int y, int w, int h, GR_FONT_ID font, UINT size, UINT color_fg, UINT color_bg, UINT align);
void DrawTextRect(char* pszText, GR_WINDOW_ID wid, GR_GC_ID gc, RECT* pRect, GR_FONT_ID font, UINT size, UINT color, UINT align);
void DrawTextRect2(char* pszText, GR_WINDOW_ID wid, GR_GC_ID gc, RECT* pRect, GR_FONT_ID font, UINT size, UINT color_fg, UINT color_bg, UINT align);
void DrawLine(int x1, int y1, int x2, int y2, GR_WINDOW_ID wid, GR_GC_ID gc, UINT color, BOOL isDash);
void DrawRect(int x, int y, int w, int h, GR_WINDOW_ID wid, GR_GC_ID gc, UINT color, BOOL isFill);
void DrawRect2(RECT* pRect, GR_WINDOW_ID wid, GR_GC_ID gc, UINT color, BOOL isFill);
void RedrawImage(GR_WINDOW_ID wid, GR_GC_ID gc, int dx, int dy, int dw, int dh, int sx, int sy, int sw, int sh, UINT id);

void GetLastEvent(GR_EVENT *ep);
void post_event(GR_EVENT_TYPE type, unsigned int event, unsigned int p1, unsigned int p2);

extern char g_dash_patern[2];

#ifdef __cplusplus
}
#endif

#endif //__NANO_X_CUSTOM_H__
