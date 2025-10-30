#ifndef _DOUBLE_CHECK_MECHA_H_
#define _DOUBLE_CHECK_MECHA_H_

#ifndef THHKNS_CH_LENGTH
#define THHKNS_CH_LENGTH (256)
#endif

// �d�����m�i���J���j
typedef struct
{
	// ����
	u16 double_check_threshold;		// �d�����o����臒l
	u16 bill_check_threshold;		// �������o����臒l
	u8  double_area_ratio;			// �ʐϔ䗦臒l(%)
	u8  exclude_length;				// �擪���菜�O�͈�(mm)

	// �o��
	// ���ʃu���b�N
	u8  result;					// ���茋��
	u8  double_check_ratio;		// �d���ʐϔ䗦
	u16 double_check_count;		// �d���ϕ��l
	u16 bill_check_count;								// ��������ϕ��l
	u8  padding[2];

	// ���ԏ��
	u16 bill_thickness_average;							// ��������ӏ��̌��ݕ��ϒl
	u8  double_check_point[THICKNESS_SENSOR_MAX_NUM];	// �d������ӏ����@�i�e�����Z���T���@�ő�16�Z���T���j
	u8  bill_check_point[THICKNESS_SENSOR_MAX_NUM];		// ��������ӏ����@�i�e�����Z���T���@�ő�16�Z���T���j
	u8  bill_top_point[THICKNESS_SENSOR_MAX_NUM];		// ������[����ӏ��@�i�e�����Z���T���@�ő�16�Z���T���j
	u8  exclude_point;									// �擪���菜�O�͈�(point)
	u8  sensor_num;										// �����Z���TCH��
	u8  check_point_top;								// ���m�Ώ۔͈͐�[
	u8  check_point_end;								// ���m�Ώ۔͈͌�[

} ST_DBL_CHK_MECHA;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	s16	double_check_mecha(u8 buf_num, ST_DBL_CHK_MECHA *st);
	u8 get_double_check_thkns_data(ST_BS * pbs, u16 thkns_ch_data[][THHKNS_CH_LENGTH], u16 height, u16 sensor_count);
	
#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif 
