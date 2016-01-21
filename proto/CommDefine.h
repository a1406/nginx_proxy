#ifndef COMMDEFINE_H
#define COMMDEFINE_H
/**ÿ���ʺ���ÿ�������10����ɫ**/
#define MAX_PLAYER_NUM  10
/**���ֵ���󳤶�**/
#define MAX_PLAYER_NAME_LEN  255
/**���������Ŀ**/
#define MAX_RAID_NUM  600
/**��ȡ���������б�������Ŀ**/
#define MAX_RAID_LIST_NUM  100
/**���������Ŀ**/
#define MAX_BAG_NUM  100
/**������񱳰��Ĵ�С**/
#define MAX_MINGGE_BAG_NUM  24
/**װ���������Ŀ**/
#define MAX_MINGGE_INSTALL_NUM  8
/**�����������Ŀ**/
#define MAX_LIEMINGGE_NUM  18
/**��������ȼ�**/
#define MAX_MINGGE_LEVEL  8
/**������ƬID**/
#define MINGGE_CHIP_ID  8001
/**����۸�**/
#define MINGGE_CHIP_PRICE  2000
/**����������Ŀ**/
#define MAX_QUEST_NUM  400
/**����ͬʱ���е�������Ŀ**/
#define MAX_UNDERWAY_QUEST_NUM  50
/**ÿ��װ�������Ƕ�ı�ʯ��**/
#define MAX_DIAMOND_PER_EQUIP  5
/**ÿ���ʼ����ĸ�����Ŀ**/
#define MAX_ATTACH_PER_MAIL  50
/**ÿ��ְҵ���ļ�����Ŀ**/
#define MAX_SKILL_NUM  50
/**ս����**/
#define RANK_FIGHTINGCAPACITY_TYPE  0
/**������playerid->����**/
#define RANK_COMPETITIVE_PLAYER_TYPE  1
/**����������->playerid**/
#define RANK_COMPETITIVE_RANK_TYPE  2
/**��������ʷս��**/
#define RANK_COMPETITIVE_REOCRD_TYPE  3
/**����������**/
#define RANK_TOURNAMENT_TYPE  4
/**��������**/
#define RANK_ASSOCIATION_TYPE  5
/**��ҵȼ�����**/
#define RANK_PLAYER_LEVEL_TYPE  6
/**����ս��������**/
#define RANK_PLAYER_MACHAI_TYPE  7
/**����������**/
#define RANK_THOUSAND_TOWER_TYPE  8
/**��������**/
#define RANK_MINGGE_TYPE  9
/**�������**/
#define RANK_WING_TYPE  10
/**��������**/
#define RANK_DRAGON_TYPE  11
/**װ������**/
#define RANK_EQUIP_TYPE  12
/**��Ʒ����**/
#define RANK_JEWELRY_TYPE  13
/**��ʯ����**/
#define RANK_GEN_TYPE  14
/**�ƺ�����**/
#define RANK_TITLE_TYPE  15
/**��������**/
#define RANK_PET_TYPE  16
/**����ʱװ��Ŀ**/
#define MAX_SHIZHUANG_NUM  6
/**���ĳ����Ŀ**/
#define MAX_WING_NUM  8
/**ÿ��������Ӹ�����Ŀ**/
#define MAX_TEAM_RAID_DAY_NUM  3
/**ÿ10��������1������**/
#define MAX_ADD_POWER_TIME  360
/**���ʼ�id��ʽ��ȡ���ñ��ȡ�ʼ�����**/
#define MAIL_TYPE_BY_ID  1
/**�ʼ��а������ʼ�����**/
#define MAIL_TYPE_BY_CONTENT  2
/**���������Ŀ**/
#define MAX_MACHAI_NUM  512
/**��󾺼�����ʷս����Ŀ**/
#define MAX_ARENA_RECORD_NUM  10
/**���������Ŀ**/
#define MAX_DRAGON_NUM  200
/**�̵���Ʒ�����Ŀ**/
#define MAX_SHOP_ITEM_NUM  6
/**�������̵���Ʒ�����Ŀ**/
#define MAX_COMPETITIVE_SHOP_ITEM_NUM  12
/**����vip�ȼ�**/
#define MAX_VIP_LEVEL_NUM  15
/**С�����������ȼ�1**/
#define SECRETARY_NOTIFY_LEVEL1  1
/**С�����������ȼ�2**/
#define SECRETARY_NOTIFY_LEVEL2  2
/**С�����������ȼ�3**/
#define SECRETARY_NOTIFY_LEVEL3  3
/**С�����������ȼ�4**/
#define SECRETARY_NOTIFY_LEVEL4  4
/**С�����������ȼ�5**/
#define SECRETARY_NOTIFY_LEVEL5  5
/**С�����������ȼ�6**/
#define SECRETARY_NOTIFY_LEVEL6  6
/**С�����������ȼ�7**/
#define SECRETARY_NOTIFY_LEVEL7  7
/**С�����������ȼ�8**/
#define SECRETARY_NOTIFY_LEVEL8  8
/**С����������������**/
#define SECRETARY_SKILL_UPGRADE  1
/**С�������ͽ�������**/
#define SECRETARY_SKILL_UNLOCK  2
/**���ƺ�ֵ**/
#define MAX_PLAYER_TITLE_VALUE  200
/**���ɾ�ֵ**/
#define MAX_ACHIEVEMENT_VALUE  400
/**��󾹼�����ʷ��¼ֵ**/
#define MAX_COMPETITIVE_HISTORY_VALUE  10
/**��󾹼�����ʷ����ֵ**/
#define MAX_COMPETITIVE_HISTORY_REWARD_VALUE  10
/**�ճ��������ֵ**/
#define MAX_DAILY_QUEST_VALUE  60
/**�����������**/
#define MAX_MACHAI_VALUE  512
/**��������**/
#define QUEST_FLAG_MAIN  1
/**֧������**/
#define QUEST_FLAG_FEEDER  2
/**����������Ŀ**/
#define MAX_SKILL_CONFIG_NUM  4
/**���������**/
#define MAX_FRIEND_NUM  50
/**����û��ȼ�**/
#define MAX_PLAYER_LEVEL_NUM  100
/**��󸱱��½�**/
#define MAX_RAID_SECTION_NUM  40
/**ÿ���������VIP����ֵ**/
#define MAX_VIP_EXP_DAILY_NUM  12
/**vip����ÿ10��������1��**/
#define MAX_VIP_EXP_CHECK_TIME  600
/**����ÿ������������������**/
#define MAX_FRIEND_SEND_POWER_NUM  15
/**����ÿ�ս���������������**/
#define MAX_FRIEND_RECV_POWER_NUM  10
/**��������ֵ**/
#define MAX_SEND_POWER_VAL  3
/**ƴս�ȴ�ʱ��**/
#define FIGHT_WAIT_TIME  1800
/**ƴս����ʱ��**/
#define FIGHT_RUN_TIME  600
/**ƴսÿ��������**/
#define FIGHT_MAX_COUNT  4
/**Ԯ��cdʱ��**/
#define HELPER_CD_TIME  72000
/**ʮ���Ψһ���ֵ���Ʒ�������**/
#define MAX_TEN_ACT_ONLY_NUM  10
/**�ۼƳ�ֵ����¼��Ŀ**/
#define MAX_TOTAL_RECHARGE_NUM  30
/**���������ʱ��Ʒ��Ŀ**/
#define MAX_MACHAI_TIME_LIMIT_PROP_VAL  20
/**������������**/
#define MINGGE_EXP_TYPE  99
#endif