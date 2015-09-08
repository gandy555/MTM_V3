/*
*/
#include "common.h"
#include "context_weather.h"
#include "main.h"

// g_wid기준좌표: y-center=454, t=438, x-left=297, x-right=790, w=493, 
#define MENT_AREA_X				297
#define MENT_AREA_Y				438
#define MENT_AREA_WIDTH			490
#define MENT_AREA_HEIGHT		32
#define MENT_SCROLL_SPEED		1

#define MENT_PIXEL_MAP_WIDTH	700

#define MAX_MENT_BUFF_SIZE		52

char g_szWeatherMent[IMG_ENUM_WEATHER_COUNT][MAX_MENT_BUFF_SIZE] = 
{
	"오늘은 날씨가 맑을 예정입니다\0",						//맑음,			IMG_ENUM_WEATHER_SERENITY
	"오늘은 날씨가 구름이 조금 낄 예정입니다\0",			//구름 조금,	IMG_ENUM_WEATHER_PARTLY_CLOUDY
	"오늘은 날씨가 흐릴 예정입니다\0",						//흐림,			IMG_ENUM_WEATHER_CLOUDY
	"오늘은 소나기가 올 예정이오니 우산을 준비하세요\0",	//소나기,		IMG_ENUM_WEATHER_SHOWER
	"오늘은 비가 올 예정이오니 우산을 준비하세요\0",		//비,			IMG_ENUM_WEATHER_RAINNY
	"오늘은 뇌우가 칠 예정이오니 우산을 준비하세요\0",		//뇌우,			IMG_ENUM_WEATHER_THUNDERSTORM
	"오늘은 눈이 올 예정이오니 따뜻하게 입고 외출하세요\0"	//눈,			IMG_ENUM_WEATHER_SNOW
};


//
// Construction/Destruction
//
CContextWeather::CContextWeather(GR_WINDOW_ID wid, GR_GC_ID gc)
	: CContext(wid, gc)
{
	m_isGasOff = FALSE;

	m_wid_ment = 0;
	m_pixmap_ment = 0;
}

CContextWeather::~CContextWeather()
{
}

//
// Member Function
//
void CContextWeather::Init()
{
	CObject* pObject;
	CObjectIcon* pObjectIcon;
	UINT id;

	CContext::Init();

	// 배경 이미지
	pObject = new CObjectImage(m_wid_parent, m_gc, 0, 0, g_scr_info.cols, g_scr_info.rows);
	if(pObject)
	{
		pObject->LoadImage(IMG_BACKGROUND, "/app/img/main_bg_blank.png");

		id = m_ObjectList.AddObject(pObject);
	}

	// 왼쪽 날씨 이미지
	pObject = new CObjectIcon(m_wid_parent, m_gc, 1, 169, 300, 246);
	if(pObject)
	{
		pObjectIcon = (CObjectIcon*)pObject;
		pObjectIcon->AllocImageCount(IMG_ENUM_WEATHER_COUNT);
		pObject->LoadImage(IMG_ENUM_WEATHER_SERENITY,		"/app/img/weather/serenity.png");
		pObject->LoadImage(IMG_ENUM_WEATHER_PARTLY_CLOUDY,	"/app/img/weather/partly_cloudy.png");
		pObject->LoadImage(IMG_ENUM_WEATHER_CLOUDY,			"/app/img/weather/cloudy.png");
		pObject->LoadImage(IMG_ENUM_WEATHER_SHOWER,			"/app/img/weather/shower.png");
		pObject->LoadImage(IMG_ENUM_WEATHER_RAINNY,			"/app/img/weather/rainny.png");
		pObject->LoadImage(IMG_ENUM_WEATHER_THUNDERSTORM,	"/app/img/weather/thunderstorm.png");
		pObject->LoadImage(IMG_ENUM_WEATHER_SNOW,			"/app/img/weather/snow.png");

		id = m_ObjectList.AddObject(pObject);
	}

	// 오른쪽 날씨 이미지
	pObject = new CObjectIcon(m_wid_parent, m_gc, 499, 169, 300, 246);
	if(pObject)
	{
		pObjectIcon = (CObjectIcon*)pObject;
		pObjectIcon->AllocImageCount(IMG_ENUM_WEATHER_COUNT);
		pObject->LoadImage(IMG_ENUM_WEATHER_SERENITY,		"/app/img/weather/serenity.png");
		pObject->LoadImage(IMG_ENUM_WEATHER_PARTLY_CLOUDY,	"/app/img/weather/partly_cloudy.png");
		pObject->LoadImage(IMG_ENUM_WEATHER_CLOUDY,			"/app/img/weather/cloudy.png");
		pObject->LoadImage(IMG_ENUM_WEATHER_SHOWER,			"/app/img/weather/shower.png");
		pObject->LoadImage(IMG_ENUM_WEATHER_RAINNY,			"/app/img/weather/rainny.png");
		pObject->LoadImage(IMG_ENUM_WEATHER_THUNDERSTORM,	"/app/img/weather/thunderstorm.png");
		pObject->LoadImage(IMG_ENUM_WEATHER_SNOW,			"/app/img/weather/snow.png");

		id = m_ObjectList.AddObject(pObject);
	}

	// 가스On/Off 이미지
	pObject = new CObjectIcon(m_wid_parent, m_gc, 325, 245, 137, 137);		//좌측여백 13
	if(pObject)
	{
		pObjectIcon = (CObjectIcon*)pObject;
		pObjectIcon->AllocImageCount(IMG_ENUM_GAS_COUNT);
		pObject->LoadImage(IMG_ENUM_GAS_ON,		"/app/img/main_icon/main_gas_on.png");
		pObject->LoadImage(IMG_ENUM_GAS_OFF,	"/app/img/main_icon/main_gas_off.png");

		id = m_ObjectList.AddObject(pObject);
	}

#if 1
	// 주차아이콘
	pObject = new CObjectImage(m_wid_parent, m_gc, 0, 419, 261, 61);
	if(pObject)
	{
		pObject->LoadImage(IMG_BACKGROUND, "/app/img/main_bar_back.png");

		id = m_ObjectList.AddObject(pObject);
	}
#endif

	//g_setup_data.m_SetupData.gas_stat==1 --> 밸브열림 --> 사용중
	m_isGasOff = (g_setup_data.m_SetupData.gas_stat) ? FALSE : TRUE;	//반대

	//멘트가 표시될 서브 윈도
	m_wid_ment = GrNewWindow(g_wid, MENT_AREA_X, MENT_AREA_Y, MENT_AREA_WIDTH, MENT_AREA_HEIGHT, 0, 0, 0);
	if(m_wid_ment)
	{
	//	GrMapWindow(m_wid_ment);

		//이미지 버퍼링을 처리할 Pixel Map
	//	m_pixmap_ment = GrNewPixmap(MENT_AREA_WIDTH, MENT_AREA_HEIGHT, NULL);
		m_pixmap_ment = GrNewPixmap(MENT_PIXEL_MAP_WIDTH, MENT_AREA_HEIGHT, NULL);

	#if 0
		RedrawImage(m_wid_ment, g_gc, 0, 0, MENT_AREA_WIDTH, MENT_AREA_HEIGHT, 
					MENT_AREA_X, MENT_AREA_Y, MENT_AREA_WIDTH, MENT_AREA_HEIGHT, WEATHER_OBJ_BG);

		RedrawImage(m_pixmap_ment, g_gc, 0, 0, MENT_AREA_WIDTH, MENT_AREA_HEIGHT, 
					MENT_AREA_X, MENT_AREA_Y, MENT_AREA_WIDTH, MENT_AREA_HEIGHT, WEATHER_OBJ_BG);

		DrawText(g_szWeatherMent[g_setup_data.m_SetupData.today_weather],
					 m_pixmap_ment, g_gc, 0, 0, m_width_ment, MENT_AREA_HEIGHT, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

		GrRaiseWindow(m_wid_ment);
	#endif
	}
	else
	{
		printf("%s: GrNewWindow failure\r\n", __func__);
	}
}

void CContextWeather::DeInit()
{
	CContext::DeInit();

	GrDestroyWindow(m_pixmap_ment);

//	GrUnmapWindow(m_wid_ment);
	GrDestroyWindow(m_wid_ment);

	//Ment Scroll Window 영역이 까맣게 보이는 증상이 있어서 배경으로 다시 덮어줌
	RedrawImage(g_wid, g_gc, MENT_AREA_X, MENT_AREA_Y, MENT_AREA_WIDTH, MENT_AREA_HEIGHT, 
				MENT_AREA_X, MENT_AREA_Y, MENT_AREA_WIDTH, MENT_AREA_HEIGHT, WEATHER_OBJ_BG);
	
	m_ObjectList.RemoveAll();
}

void CContextWeather::Draw(UINT nContextNum)
{
	char szText[64] = {0,};
	char szWeatherMent[7][10] = { "맑음\0", "구름조금\0", "흐림\0", "소나기\0", "비\0", "뇌우\0", "눈\0" };

	CObjectIcon* pObjectIcon = NULL;
	int temp_low, temp_high, z_offset = 0;
	PARKING_RES parking_info;
	
	if(m_gc==0) return;

	printf("+CContextWeather::Draw(%d)\r\n", nContextNum);

	//배경
	m_ObjectList.Draw(WEATHER_OBJ_BG);

	DrawText("오  늘", g_wid, g_gc,  83, 20, 60, 18, g_font, 18, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
	DrawText("내  일", g_wid, g_gc, 665, 20, 60, 18, g_font, 18, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

	printf("today_weather=%d, tomorrow_weather=%d\r\n", g_setup_data.m_SetupData.today_weather, g_setup_data.m_SetupData.tomorrow_weather);

	//왼쪽 오늘 날씨
	DrawText(szWeatherMent[g_setup_data.m_SetupData.today_weather], g_wid, g_gc, 65, 51, 96, 32, g_font, 24, RGB(1,19,73), TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

	//왼쪽 오늘 기온 (Text)
#if 0	//소수점1자리 온도표시
	sprintf(szText, "%2.1f/%2.1f\0", 
			g_setup_data.m_SetupData.today_temp_low / 10.0,
			g_setup_data.m_SetupData.today_temp_high / 10.0);
#else	//정수 온도표시
	temp_low  = (g_setup_data.m_SetupData.today_temp_low  >= 0x80) ? (g_setup_data.m_SetupData.today_temp_low  - 256) : g_setup_data.m_SetupData.today_temp_low;
	temp_high = (g_setup_data.m_SetupData.today_temp_high >= 0x80) ? (g_setup_data.m_SetupData.today_temp_high - 256) : g_setup_data.m_SetupData.today_temp_high;
	sprintf(szText, "%2d/%2d\0", temp_low, temp_high);
#endif
	DrawText(szText, g_wid, g_gc, 17, 100, 112, 32, g_font, 24, WHITE, TXT_HALIGN_RIGHT|TXT_VALIGN_MIDDLE);

	//왼쪽 오늘 날씨 이미지 
	pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(WEATHER_OBJ_WEATHER_LEFT);
	if(pObjectIcon)
	{
		pObjectIcon->Draw(g_setup_data.m_SetupData.today_weather);
	}

	//오른쪽 내일 날씨
	DrawText(szWeatherMent[g_setup_data.m_SetupData.tomorrow_weather], g_wid, g_gc, 647, 51, 96, 32, g_font, 24, RGB(162,199,213), TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

	memset(szText, 0, 64);
	//오른쪽 내일 기온 (Text)
#if 0	//소수점1자리 온도표시
	sprintf(szText, "%2.1f/%2.1f\0", 
			g_setup_data.m_SetupData.tomorrow_temp_low / 10.0,
			g_setup_data.m_SetupData.tomorrow_temp_high / 10.0);
#else
	temp_low  = (g_setup_data.m_SetupData.tomorrow_temp_low  >= 0x80) ? (g_setup_data.m_SetupData.tomorrow_temp_low  - 256) : g_setup_data.m_SetupData.tomorrow_temp_low;
	temp_high = (g_setup_data.m_SetupData.tomorrow_temp_high >= 0x80) ? (g_setup_data.m_SetupData.tomorrow_temp_high - 256) : g_setup_data.m_SetupData.tomorrow_temp_high;
	sprintf(szText, "%2d/%2d\0", temp_low, temp_high);
#endif
	DrawText(szText, g_wid, g_gc, 599, 100, 112, 32, g_font, 24, WHITE, TXT_HALIGN_RIGHT|TXT_VALIGN_MIDDLE);

	//오른쪽 내일 날씨 이미지
	pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(WEATHER_OBJ_WEATHER_RIGHT);
	if(pObjectIcon)
	{
		pObjectIcon->Draw(g_setup_data.m_SetupData.tomorrow_weather);
	}

	//가스
	pObjectIcon = (CObjectIcon*)m_ObjectList.FindObjectByID(WEATHER_OBJ_GAS);
	if(pObjectIcon)
	{
//		pObjectIcon->Draw(m_isGasOff ? IMG_ENUM_GAS_ON : IMG_ENUM_GAS_OFF);
		pObjectIcon->Draw(g_setup_data.m_SetupData.gas_stat ? IMG_ENUM_GAS_ON : IMG_ENUM_GAS_OFF);
	}
#if 1
	//주차위치
	m_ObjectList.Draw(WEATHER_OBJ_PARKING_BAR);

	parking_list_get_item(0, &parking_info);		
	if (parking_info.in_out == PARKING_STATUS_IN) {
		memset(szText, 0, 64);
		parking_item_to_str(PARKING_ITM_FLOOR, parking_info.car_floor, szText);
		z_offset = strlen(szText);
		sprintf(&szText[z_offset], "-");

		z_offset = strlen(szText);
		parking_item_to_str(PARKING_ITM_ZONE, parking_info.car_zone, &szText[z_offset]);

		DrawText(szText,  m_wid_parent, m_gc,  73, 436, 180, 32, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);
	}
#endif

	TimerProc(CLOCK_TIMER);

	memset(szText, 0, 64);
//<	버전표기
	if(VER_REVISION != ' ')
	{
		if(strlen(VER_STRING) > 0)
			sprintf(szText, "[ v%d.%02d.%c-%s %s ]\0", VER_MAJOR, VER_MINOR, VER_REVISION, VER_DATE, VER_STRING);
		else
			sprintf(szText, "[ v%d.%02d.%c-%s ]\0", VER_MAJOR, VER_MINOR, VER_REVISION, VER_DATE);
	}
	else
	{
		if(strlen(VER_STRING) > 0)
			sprintf(szText, "[ v%d.%02d-%s %s ]\0", VER_MAJOR, VER_MINOR, VER_DATE, VER_STRING);
		else
			sprintf(szText, "[ v%d.%02d-%s ]\0", VER_MAJOR, VER_MINOR, VER_DATE);
	}
	DrawText(szText, g_wid, g_gc, 0, 400, strlen(szText)*6+4, 14, g_font, 12, BLACK, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
//>

	switch(nContextNum)
	{
	case 0:
		break;
	}

	printf("-CContextWeather::Draw\r\n");
}

void CContextWeather::Proc(UINT nContextNum)
{
	char szWaveFile[7][14] = { "serenity\0", "party_cloudy\0", "cloudy\0", "shower\0", "rainny\0", "thunderstorm\0", "snow\0" };
	char szWaveFilePath[128];

	printf("+CContextWeather::Proc(%d)\r\n", nContextNum);

	switch(nContextNum)
	{
	case 0:		//하단 배너 준비
	//	if(g_isBackLightOn && (g_gpio_mtm.GetGpioInputPin(GPIO_DOOR_DETECT)==HIGH))	//백라이트가 켜져있고 출입문이 닫힘
	//	if(g_gpio_mtm.GetGpioInputPin(GPIO_DOOR_DETECT)==HIGH)	//출입문이 닫힘
		if((!g_setup_data.m_SetupData.alloff_stat) && (g_gpio_mtm.CheckWatchdogReset()==FALSE) && (g_gpio_mtm.GetGpioInputPin(GPIO_DOOR_DETECT)==HIGH) )	//와치독 리셋상태가 아니고 출입문이 닫힌 상태이면
		{
			sprintf(szWaveFilePath, "/app/sound/%s.wav\0", szWaveFile[g_setup_data.m_SetupData.today_weather]);
			PlayWavFile(szWaveFilePath);
		}

		//문자열의 길이
		m_len_ment = strlen(g_szWeatherMent[g_setup_data.m_SetupData.today_weather]);

		//폰트크기를 감안한 문자열이 표시되는 폭
		m_width_ment = m_len_ment*12;

		//멘트 텍스트를 픽셀맵에 그리기
		GrMapWindow(m_wid_ment);

		RedrawImage(m_wid_ment, g_gc, 0, 0, MENT_AREA_WIDTH, MENT_AREA_HEIGHT, 
					MENT_AREA_X, MENT_AREA_Y, MENT_AREA_WIDTH, MENT_AREA_HEIGHT, WEATHER_OBJ_BG);

		RedrawImage(m_pixmap_ment, g_gc, 0, 0, MENT_AREA_WIDTH, MENT_AREA_HEIGHT, 
					MENT_AREA_X, MENT_AREA_Y, MENT_AREA_WIDTH, MENT_AREA_HEIGHT, WEATHER_OBJ_BG);
		RedrawImage(m_pixmap_ment, g_gc, MENT_AREA_WIDTH, 0, MENT_PIXEL_MAP_WIDTH-MENT_AREA_WIDTH, MENT_AREA_HEIGHT, 
					MENT_AREA_X, MENT_AREA_Y, MENT_PIXEL_MAP_WIDTH-MENT_AREA_WIDTH, MENT_AREA_HEIGHT, WEATHER_OBJ_BG);

		DrawText(g_szWeatherMent[g_setup_data.m_SetupData.today_weather],
				 m_pixmap_ment, g_gc, 0, 0, m_width_ment, MENT_AREA_HEIGHT, g_font, 24, WHITE, TXT_HALIGN_LEFT|TXT_VALIGN_MIDDLE);

		GrRaiseWindow(m_wid_ment);

//		g_wallpad_sns.RequestReserve(SNS_CMD_WEATHER_REQ);

		m_nThreadStep = 0;
		break;
	case 1:
		break;
	}

	printf("-CContextWeather::Proc\r\n");
}

void CContextWeather::TimerProc(UINT idTimer)
{
	time_t now;
	struct tm *t;
	char szText[128] = {0,};

	if(idTimer == CLOCK_TIMER)
	{
		now = time(NULL);
		t = localtime(&now);

		sprintf(szText, "%04d년 %02d월 %02d일\0", 
				t->tm_year + 1900,
				t->tm_mon + 1,
				t->tm_mday);

		RedrawImage(g_wid, g_gc, 300, 34, 200, 32, 300, 34, 200, 32, WEATHER_OBJ_BG);
		DrawText(szText, g_wid, g_gc, 300, 34, 200, 32, g_font, 24, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);

		memset(szText, 0, 128);
		sprintf(szText, "%02d:%02d\0", 
				t->tm_hour,
				t->tm_min);

		RedrawImage(g_wid, g_gc, 300, 68, 200, 88, 300, 68, 200, 88, WEATHER_OBJ_BG);
		DrawText(szText, g_wid, g_gc, 300, 68, 200, 88, g_font, 72, WHITE, TXT_HALIGN_CENTER|TXT_VALIGN_MIDDLE);
	}
}

void CContextWeather::RecvProc(UCHAR *pPacket)
{
	if(pPacket == NULL) return;

	printf("+CContextWeather::RecvProc\r\n");

	switch(pPacket[1])
	{
	case SNS_CMD_WEATHER_RES:
	case SNS_CMD_GAS_STAT_RES:
	case SNS_CMD_PARKING_RES:
	case SNS_CMD_WALLPAD_STAT_REQ:
		if (g_isBackLightOn)
			ChangeContext(1);
		break;
	}

	printf("-CContextWeather::RecvProc\r\n");
}

void CContextWeather::ThreadProc()
{
	static int i, x;

	switch(m_nThreadStep)
	{
	case 0:
		x = MENT_AREA_WIDTH;
		i = 0;
		m_nThreadStep++;
		break;
	case 1:		//멘트가 우측에서 진입해서 좌측끝까지 이동
		GrCopyArea(m_wid_ment, g_gc, MENT_AREA_WIDTH-i, 0, i, MENT_AREA_HEIGHT, m_pixmap_ment, 0, 0, MWROP_SRCCOPY);
		x-=MENT_SCROLL_SPEED;
		i+=MENT_SCROLL_SPEED;
		if(i > MENT_AREA_WIDTH) 
		{
			m_nThreadStep++;
			i = 0;
		}
		break;
	case 2:		//2초간 정지. 메인루프가 10ms delay를 갖고 있으므로 최소 200회
		i++;
		if(i >= 2000)
		{
			m_nThreadStep++;
			i = 0;
		}
		break;
	case 3:		//멘트가 사라질때까지 이동
	#if 0
		RedrawImage(m_wid_ment, g_gc, 0, 0, m_width_ment, MENT_AREA_HEIGHT, 
					MENT_AREA_X, MENT_AREA_Y, m_width_ment, MENT_AREA_HEIGHT, WEATHER_OBJ_BG);

		GrCopyArea(m_wid_ment, g_gc, 0, 0, m_width_ment-i, MENT_AREA_HEIGHT, m_pixmap_ment, i, 0, MWROP_SRCCOPY);
	#else
		GrCopyArea(m_wid_ment, g_gc, 0, 0, m_width_ment-i, MENT_AREA_HEIGHT, m_pixmap_ment, i, 0, MWROP_SRCCOPY);
	#endif

		x-=MENT_SCROLL_SPEED;
		i+=MENT_SCROLL_SPEED;

		if(i >= m_width_ment)
		{
			m_nThreadStep++;
			i = 0;
		}
		break;
	case 4:		//2초간 정지. 메인루프가 10ms delay를 갖고 있으므로 최소 200회
		i++;
		if(i >= 2000)
		{
			m_nThreadStep = 0;
		}
		break;
	}
}

void CContextWeather::ButtonDown(UINT usGpioFlag, UINT usEventEnum)
{
	printf("+CContextWeather::ButtonDown\r\n");

	if(usEventEnum == MTM_GPIO_BUTTON_DOWN)
	{
	}
	else if(usEventEnum == MTM_GPIO_BUTTON_LONG)
	{
		if( CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_TOP)|FLAG(GPIO_FRONT_RIGHT_BOTTOM)) ||
			CHK_FLAG(usGpioFlag, FLAG(GPIO_REAR_VOL_UP)|FLAG(GPIO_REAR_VOL_DOWN)) )
		{
			if (!CHK_FLAG(usGpioFlag, FLAG(GPIO_FRONT_RIGHT_MIDDLE))) {
				GrUnmapWindow(m_wid_ment);

				//Ment Scroll Window 영역이 까맣게 보이는 증상이 있어서 배경으로 다시 덮어줌
				RedrawImage(g_wid, g_gc, MENT_AREA_X, MENT_AREA_Y, MENT_AREA_WIDTH, MENT_AREA_HEIGHT, 
						MENT_AREA_X, MENT_AREA_Y, MENT_AREA_WIDTH, MENT_AREA_HEIGHT, WEATHER_OBJ_BG);
				g_state.SetNextState(STATE_SETUP);
			}
		}

	#if(PROTOCOL_SIMULATE == 1)		//for 월패드 통신 테스트(날씨)
		if(usGpioFlag == GPIO_FLAG_FRONT_LEFT_TOP)
		{
			if(g_wallpad_sns.m_nProtocolSimulateNum < 0)
			{
				g_wallpad_sns.m_nProtocolSimulateNum = 0;
			}
			else
			{
				g_wallpad_sns.m_nProtocolSimulateNum++;
				if(g_wallpad_sns.m_nProtocolSimulateNum == PROTO_SIM_COUNT) 
				{
					g_wallpad_sns.m_nProtocolSimulateNum = 0;
				}
			}
			g_wallpad_sns.m_isProtocolSimulate = TRUE;

			printf("m_isProtocolSimulate=%d, m_nProtocolSimulateNum=%d\r\n", g_wallpad_sns.m_isProtocolSimulate, g_wallpad_sns.m_nProtocolSimulateNum);
		}
	#endif
	}
	printf("-CContextWeather::ButtonDown\r\n");
}

void CContextWeather::ButtonUp(UINT usGpioFlag, UINT usEventEnum)
{
	printf("+CContextWeather::ButtonUp\r\n");

	if(usEventEnum == MTM_GPIO_BUTTON_UP)
	{
		switch(usGpioFlag)
		{
		case GPIO_FLAG_FRONT_LEFT_TOP:		//Weather
		//	g_state.SetNextState(STATE_WEATHER);
			g_wallpad_sns.RequestReserve(SNS_CMD_WEATHER_REQ);		//for test, 날씨정보요청
			break;
		case GPIO_FLAG_FRONT_LEFT_MIDDLE:	//Elevator
			g_state.SetNextState(STATE_ELEVATOR);
			break;
		case GPIO_FLAG_FRONT_LEFT_BOTTOM:	//Parking
			g_state.SetNextState(STATE_PARKING);
			break;
		case GPIO_FLAG_FRONT_RIGHT_TOP:		//Gas
			g_state.SetNextState(STATE_GAS);
			break;
		case GPIO_FLAG_FRONT_RIGHT_MIDDLE:	//AllOff - 반전동작
			g_state.SetNextState(STATE_ALLOFF);
			break;
		case GPIO_FLAG_FRONT_RIGHT_BOTTOM:	//Security
			g_state.SetNextState(STATE_SECURITY);
			break;
		}

		if(g_state.GetNextState() != STATE_WEATHER)
		{
		//	GrLowerWindow(m_wid_ment);
			GrUnmapWindow(m_wid_ment);

			//Ment Scroll Window 영역이 까맣게 보이는 증상이 있어서 배경으로 다시 덮어줌
			RedrawImage(g_wid, g_gc, MENT_AREA_X, MENT_AREA_Y, MENT_AREA_WIDTH, MENT_AREA_HEIGHT, 
						MENT_AREA_X, MENT_AREA_Y, MENT_AREA_WIDTH, MENT_AREA_HEIGHT, WEATHER_OBJ_BG);
		}
	}
	printf("-CContextWeather::ButtonUp\r\n");
}

