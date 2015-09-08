/*
	����Ʈ �����׸� ����

	: ����Ʈ���� ����̳� ���۹���� �޶����� ��ҵ��� ������
*/

#ifndef __CONFIG_SITE_H__
#define __CONFIG_SITE_H__

//����Ʈ �Ǽ���/�귣�� (�˺�����)
enum
{
	SITE_ID_AMCO,				//����,			����Ÿ��
	SITE_ID_BOOYOUNG,			//�ο�,			�������
	SITE_ID_BYUKSAN,			//����Ǽ�,		����
	SITE_ID_CHUNGWON,			//û���Ǽ�,		����ó��
	SITE_ID_DAEA,				//���,			��������
	SITE_ID_DAEDONG,			//�뵿,			�ٽ�/�̹���
	SITE_ID_DAEGYO,				//�뱳��ؿ���,	���̶�������
	SITE_ID_DAEJU,				//���ְǼ�,		�ǿ���
	SITE_ID_DAERIM,				//�븲���,		e-���Ѽ���
	SITE_ID_DAESUNG,			//�뼺���,		���ϵ�
	SITE_ID_DAEWON,				//���,			ĭŸ��
	SITE_ID_DAEWOO_GUN,			//���Ǽ�,		Ǫ������
	SITE_ID_DAEWOO_CHA,			//������Ǹ�,	�̾�
	SITE_ID_DONGBU,				//���ΰǼ�,		��Ʈ����
	SITE_ID_DONGIL,				//�������,		���̺�
	SITE_ID_DONGMOON,			//�����Ǽ�,		�¸����
	SITE_ID_DONGWON,			//��������,		�����ξ��ũ
	SITE_ID_DONGYANG,			//�����ӰǼ�,	�Ķ��
	SITE_ID_DOOSAN,				//�λ�Ǽ�,		����
	SITE_ID_GEUMHO,				//��ȣ�Ǽ�,		��þ�þ�/��︲
	SITE_ID_GEUMKANG,			//�ݰ����հǼ�,	����þ
	SITE_ID_GS,					//GS�Ǽ�,		����
	SITE_ID_GYERYONG,			//���Ǽ�,		������
	SITE_ID_GYUNGNAM,			//�泲,			�Ƴʽ���
	SITE_ID_HANIL,				//���ϰǼ�,		���ؾ���/����ü
	SITE_ID_HANJIN,				//�����߰���,	������/�ظ�
	SITE_ID_HANLA,				//�Ѷ�Ǽ�,		��ߵ�
	SITE_ID_HANSIN,				//�ѽŰ���,		��
	SITE_ID_HANSOL,				//�ѼְǼ�,		����ũ
	SITE_ID_HANTO,				//�ѱ�������Ź,	�ھƷ�
	SITE_ID_HANWHA,				//��ȭ�Ǽ�,		�޿��׸�
	SITE_ID_HEUNGHWA,			//��ȭ,			��ȭ�����
	SITE_ID_HWASUNG,			//ȭ�����,		ȭ����ũ�帲
	SITE_ID_HYOSUNG,			//ȿ��,			��Ⱑ��
	SITE_ID_HYUNDAI_GUN,		//����Ǽ�,		�����丮��/ȨŸ��/��������Ʈ
	SITE_ID_HYUNDAI_SAN,		//����������,	������ũ
	SITE_ID_HYUNJIN,			//����������,	������
	SITE_ID_ISU,				//�̼��Ǽ�,		�����
	SITE_ID_IS_DONGSEO,			//IS����,		���ϸ��Ƕ�
	SITE_ID_JINHEUNG,			//������,		���纥��/������Ÿ��
	SITE_ID_JOONGANG,			//�߾ӰǼ�,		������
	SITE_ID_JOONGHEUNG,			//����Ǽ�,		S-Ŭ����
	SITE_ID_KOLON,				//�ڿ��հǼ�,	�ϴ�ä
	SITE_ID_LIGA,				//LIGA�ǿ�,		����
	SITE_ID_LOTTE,				//�Ե��Ǽ�,		ĳ��
	SITE_ID_MYUNGJI,			//�����Ǽ�,		��������
	SITE_ID_NAMGWANG,			//�������,		�Ͽ콺�丮
	SITE_ID_NAMYANG,			//����Ǽ�,		������ư
	SITE_ID_NEULPUREUN,			//��Ǫ������,	����ī
	SITE_ID_POONGRIM,			//ǳ�����,		���̿�
	SITE_ID_POSCO,				//�����ڰǼ�,	����
	SITE_ID_SAMBOO,				//������,		���׻�
	SITE_ID_SAMHWAN,			//��ȯ���,		�����
	SITE_ID_SAMSUNG_JOONG,		//�Ｚ�߰���,	�Ｚ������
	SITE_ID_SAMSUNG_MOOL,		//�Ｚ����,		���̾�
	SITE_ID_SEOHAE,				//�������հǼ�,	���ر׶���
	SITE_ID_SEYANG,				//����Ǽ�,		û����
	SITE_ID_SINCHANG,			//��â�Ǽ�,		������ʸ�
	SITE_ID_SINDO,				//�ŵ����հǼ�,	�귡��
	SITE_ID_SINDONGA,			//�ŵ��ưǼ�,	�Ķ�ä/�Ĺи���
	SITE_ID_SINHAN,				//����,			������
	SITE_ID_SINIL,				//����,			����Ʈ��
	SITE_ID_SINIL_GUN,			//���ϰǾ�,		�����
	SITE_ID_SINSUNG,			//�ż�,			�̼�����
	SITE_ID_SINWON,				//�ſ����հ���,	��ħ����
	SITE_ID_SINYOUNG,			//�ſ�,			�ξ��Ӹ���/���ι潺
	SITE_ID_SK,					//SK�Ǽ�,		��
	SITE_ID_SSANGYONG,			//�ֿ�ǰ�,		����
	SITE_ID_SUNGJI,				//�����Ǽ�,		�����罺
	SITE_ID_SUNGWON,			//�����Ǽ�,		�󶼺�
	SITE_ID_TAEYOUNG,			//�¿�,			���þ�
	SITE_ID_ULTRA,				//��Ʈ��Ǽ�,	��Ʈ��/������
	SITE_ID_WOOMI,				//��̰Ǽ�,		��̸�
	SITE_ID_WOONAM,				//�쳲�Ǽ�,		�쳲�۽�Ʈ��
	SITE_ID_WOORIM,				//�츲�Ǽ�,		�츲����/ī�����Ӹ���
	SITE_ID_WORLD,				//����Ǽ�,		�޸����
	SITE_ID_YOOJIN,				//����,			������21

	SITE_ID_COUNT
};

//����Ʈ ����
enum
{
	SITE_REGION_SEOUL_GANGGYO,		//��������		����-����
	SITE_REGION_BUSAN_MYUNGJI,		//���ϸ��Ƕ�	�λ�-�����ŵ���		IS-����
	SITE_REGION_HANAM_WIRYE,		//����Ÿ��		�ϳ�-���ʽŵ���

	SITE_REGION_COUNT
};

// ����Ʈ�� ��ɿɼ� �ĺ���

#endif //__CONFIG_SITE_H__
