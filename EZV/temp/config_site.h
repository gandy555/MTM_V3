/*
	사이트 설정항목 정의

	: 사이트별로 기능이나 동작방식이 달라지는 요소들을 정의함
*/

#ifndef __CONFIG_SITE_H__
#define __CONFIG_SITE_H__

//아파트 건설사/브랜드 (알베벳순)
enum
{
	SITE_ID_AMCO,				//엠코,			엠코타운
	SITE_ID_BOOYOUNG,			//부영,			사랑으로
	SITE_ID_BYUKSAN,			//벽산건설,		블루밍
	SITE_ID_CHUNGWON,			//청원건설,		네이처빌
	SITE_ID_DAEA,				//대아,			아이투빌
	SITE_ID_DAEDONG,			//대동,			다숲/이미지
	SITE_ID_DAEGYO,				//대교디앤에스,	미켈란쉐르빌
	SITE_ID_DAEJU,				//대주건설,		피오레
	SITE_ID_DAERIM,				//대림산업,		e-편한세상
	SITE_ID_DAESUNG,			//대성산업,		유니드
	SITE_ID_DAEWON,				//대원,			칸타빌
	SITE_ID_DAEWOO_GUN,			//대우건설,		푸르지오
	SITE_ID_DAEWOO_CHA,			//대우차판매,	이안
	SITE_ID_DONGBU,				//동부건설,		센트레빌
	SITE_ID_DONGIL,				//동일토건,		하이빌
	SITE_ID_DONGMOON,			//동문건설,		굿모닝힐
	SITE_ID_DONGWON,			//동원개발,		동원로얄듀크
	SITE_ID_DONGYANG,			//동양고속건설,	파라곤
	SITE_ID_DOOSAN,				//두산건설,		위브
	SITE_ID_GEUMHO,				//금호건설,		리첸시아/어울림
	SITE_ID_GEUMKANG,			//금강종합건설,	스위첸
	SITE_ID_GS,					//GS건설,		자이
	SITE_ID_GYERYONG,			//계룡건설,		리슈빌
	SITE_ID_GYUNGNAM,			//경남,			아너스빌
	SITE_ID_HANIL,				//한일건설,		유앤아이/베라체
	SITE_ID_HANJIN,				//한진중공업,	로즈힐/해모르
	SITE_ID_HANLA,				//한라건설,		비발디
	SITE_ID_HANSIN,				//한신공영,		휴
	SITE_ID_HANSOL,				//한솔건설,		솔파크
	SITE_ID_HANTO,				//한국토지신탁,	코아루
	SITE_ID_HANWHA,				//한화건설,		꿈에그린
	SITE_ID_HEUNGHWA,			//흥화,			흥화브라운빌
	SITE_ID_HWASUNG,			//화설산업,		화성파크드림
	SITE_ID_HYOSUNG,			//효성,			백년가약
	SITE_ID_HYUNDAI_GUN,		//현대건설,		하이페리온/홈타운/힐스테이트
	SITE_ID_HYUNDAI_SAN,		//현대산업개발,	아이파크
	SITE_ID_HYUNJIN,			//현진에버빌,	에버빌
	SITE_ID_ISU,				//이수건설,		브라운스톤
	SITE_ID_IS_DONGSEO,			//IS동서,		에일린의뜰
	SITE_ID_JINHEUNG,			//진흥기업,		더루벤스/마제스타워
	SITE_ID_JOONGANG,			//중앙건설,		하이츠
	SITE_ID_JOONGHEUNG,			//중흥건설,		S-클래스
	SITE_ID_KOLON,				//코오롱건설,	하늘채
	SITE_ID_LIGA,				//LIGA건영,		리가
	SITE_ID_LOTTE,				//롯데건설,		캐슬
	SITE_ID_MYUNGJI,			//명지건설,		엘펜하임
	SITE_ID_NAMGWANG,			//남광토건,		하우스토리
	SITE_ID_NAMYANG,			//남양건설,		남양휴튼
	SITE_ID_NEULPUREUN,			//늘푸른주택,	오스카
	SITE_ID_POONGRIM,			//풍림산업,		아이원
	SITE_ID_POSCO,				//포스코건설,	더샾
	SITE_ID_SAMBOO,				//삼부토건,		르네상스
	SITE_ID_SAMHWAN,			//삼환기업,		나우빌
	SITE_ID_SAMSUNG_JOONG,		//삼성중공업,	삼성쉐르빌
	SITE_ID_SAMSUNG_MOOL,		//삼성물산,		래미안
	SITE_ID_SEOHAE,				//서해종합건설,	서해그랑블
	SITE_ID_SEYANG,				//세양건설,		청마루
	SITE_ID_SINCHANG,			//신창건설,		비바패필리
	SITE_ID_SINDO,				//신도종합건설,	브래뉴
	SITE_ID_SINDONGA,			//신동아건설,	파란채/파밀리에
	SITE_ID_SINHAN,				//신한,			미지엔
	SITE_ID_SINIL,				//신일,			해피트리
	SITE_ID_SINIL_GUN,			//신일건업,		유토빌
	SITE_ID_SINSUNG,			//신성,			미소지움
	SITE_ID_SINWON,				//신원종합개발,	아침도시
	SITE_ID_SINYOUNG,			//신영,			로얄팰리스/프로방스
	SITE_ID_SK,					//SK건설,		뷰
	SITE_ID_SSANGYONG,			//쌍용건걸,		예가
	SITE_ID_SUNGJI,				//성지건설,		리벨루스
	SITE_ID_SUNGWON,			//성원건설,		상떼빌
	SITE_ID_TAEYOUNG,			//태영,			데시앙
	SITE_ID_ULTRA,				//울트라건설,	울트라/참누리
	SITE_ID_WOOMI,				//우미건설,		우미린
	SITE_ID_WOONAM,				//우남건설,		우남퍼스트빌
	SITE_ID_WOORIM,				//우림건설,		우림필유/카이저팰리스
	SITE_ID_WORLD,				//월드건설,		메르디앙
	SITE_ID_YOOJIN,				//유진,			마젤란21

	SITE_ID_COUNT
};

//아파트 지역
enum
{
	SITE_REGION_SEOUL_GANGGYO,		//프로지오		서울-광교
	SITE_REGION_BUSAN_MYUNGJI,		//에일린의뜰	부산-명지신도시		IS-동서
	SITE_REGION_HANAM_WIRYE,		//엠코타운		하남-위례신도시

	SITE_REGION_COUNT
};

// 사이트별 기능옵션 식별자

#endif //__CONFIG_SITE_H__
